//
// Created by Sam on 2020/6/16.
//

#include <core/memory.h>
#include "bvh.h"
#include "parallel.h"


struct BVHPrimitiveInfo {
    BVHPrimitiveInfo() {}

    BVHPrimitiveInfo(size_t primitiveNumber, const Bounds3f &bounds)
            : primitiveNumber(primitiveNumber),
              bounds(bounds),
              centroid(.5f * bounds.pMin + .5f * bounds.pMax) {}

    size_t primitiveNumber;
    Bounds3f bounds;
    Point3f centroid;
};

struct BVHNode {

    void InitLeaf(int first, int n, const Bounds3f &b) {
        firstPrimOffset = first;
        nPrimitives = n;
        bounds = b;
        children[0] = children[1] = nullptr;
    }

    void InitInterior(int axis, BVHNode *c0, BVHNode *c1) {
        children[0] = c0;
        children[1] = c1;
        bounds = Union(c0->bounds, c1->bounds);
        splitAxis = axis;
        nPrimitives = 0;
    }

    Bounds3f bounds;
    BVHNode *children[2];
    int splitAxis, firstPrimOffset, nPrimitives;
};

struct LinearBVHNode {
    Bounds3f bounds;
    union {
        int primitivesOffset;   // leaf
        int secondChildOffset;  // interior
    };
    uint16_t nPrimitives;  // 0 -> interior node
    uint8_t axis;          // interior node: xyz
    uint8_t pad[1];        // ensure 32 byte total size
};



BVH::BVH(std::vector<std::shared_ptr<Primitive>> p,
         int maxPrimsInNode)
        : maxPrimsInNode(std::min(255, maxPrimsInNode)),
          primitives(std::move(p)) {

    if (primitives.empty()) return;


    // 初始化 primitiveInfo 数组
    std::vector<BVHPrimitiveInfo> primitiveInfo(primitives.size());
    for (size_t i = 0; i < primitives.size(); ++i)
        primitiveInfo[i] = {i, primitives[i]->WorldBound()};

    // 建树
    MemoryBank bank;
    int totalNodes = 0;
    std::vector<std::shared_ptr<Primitive>> orderedPrims;
    orderedPrims.reserve(primitives.size());
    BVHNode *root;

    root = recursiveBuild(bank, primitiveInfo, 0, primitives.size(),
                          &totalNodes, orderedPrims);
    primitives.swap(orderedPrims);
    primitiveInfo.resize(0);


    nodes = AllocMem<LinearBVHNode>(totalNodes);
    int offset = 0;
    flattenBVHTree(root, &offset);

}

Bounds3f BVH::WorldBound() const {
    return nodes ? nodes[0].bounds : Bounds3f();
}


BVHNode *BVH::recursiveBuild(
        MemoryBank &arena, std::vector<BVHPrimitiveInfo> &primitiveInfo, int start,
        int end, int *totalNodes,
        std::vector<std::shared_ptr<Primitive>> &orderedPrims) {

    BVHNode *node = arena.Alloc<BVHNode>();
    (*totalNodes)++;

    Bounds3f bounds;
    for (int i = start; i < end; ++i)
        bounds = Union(bounds, primitiveInfo[i].bounds);
    int nPrimitives = end - start;
    if (nPrimitives == 1) {

        int firstPrimOffset = orderedPrims.size();
        for (int i = start; i < end; ++i) {
            int primNum = primitiveInfo[i].primitiveNumber;
            orderedPrims.push_back(primitives[primNum]);
        }
        node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
        return node;
    } else {

        Bounds3f centroidBounds;
        for (int i = start; i < end; ++i)
            centroidBounds = Union(centroidBounds, primitiveInfo[i].centroid);
        int dim = centroidBounds.MaximumExtent();


        int mid = (start + end) / 2;
        if (centroidBounds.pMax[dim] == centroidBounds.pMin[dim]) {
            // Create leaf _BVHBuildNode_
            int firstPrimOffset = orderedPrims.size();
            for (int i = start; i < end; ++i) {
                int primNum = primitiveInfo[i].primitiveNumber;
                orderedPrims.push_back(primitives[primNum]);
            }
            node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
            return node;
        } else {
            // 二分
            mid = (start + end) / 2;
            std::nth_element(&primitiveInfo[start], &primitiveInfo[mid],
                             &primitiveInfo[end - 1] + 1,
                             [dim](const BVHPrimitiveInfo &a,
                                   const BVHPrimitiveInfo &b) {
                                 return a.centroid[dim] < b.centroid[dim];
                             });


            node->InitInterior(dim,
                               recursiveBuild(arena, primitiveInfo, start, mid,
                                              totalNodes, orderedPrims),
                               recursiveBuild(arena, primitiveInfo, mid, end,
                                              totalNodes, orderedPrims));
        }
    }
    return node;
}



int BVH::flattenBVHTree(BVHNode *node, int *offset) {
    LinearBVHNode *linearNode = &nodes[*offset];
    linearNode->bounds = node->bounds;
    int myOffset = (*offset)++;
    if (node->nPrimitives > 0) {

        linearNode->primitivesOffset = node->firstPrimOffset;
        linearNode->nPrimitives = node->nPrimitives;
    } else {

        linearNode->axis = node->splitAxis;
        linearNode->nPrimitives = 0;
        flattenBVHTree(node->children[0], offset);
        linearNode->secondChildOffset =
                flattenBVHTree(node->children[1], offset);
    }
    return myOffset;
}

BVH::~BVH() { FreeMem(nodes); }

bool BVH::Intersect(const Ray &ray, SurfaceHit *isect) const {
    if (!nodes) return false;

    bool hit = false;
    Vector3f invDir(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
    int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};

    int toVisitOffset = 0, currentNodeIndex = 0;
    int nodesToVisit[64];
    while (true) {
        const LinearBVHNode *node = &nodes[currentNodeIndex];

        if (node->bounds.IntersectP(ray, invDir, dirIsNeg)) {
            if (node->nPrimitives > 0) {

                for (int i = 0; i < node->nPrimitives; ++i)
                    if (primitives[node->primitivesOffset + i]->Intersect(
                            ray, isect))
                        hit = true;
                if (toVisitOffset == 0) break;
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            } else {

                if (dirIsNeg[node->axis]) {
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->secondChildOffset;
                } else {
                    nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
            }
        } else {
            if (toVisitOffset == 0) break;
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
    }
    return hit;
}

bool BVH::IntersectP(const Ray &ray) const {
    if (!nodes) return false;

    Vector3f invDir(1.f / ray.d.x, 1.f / ray.d.y, 1.f / ray.d.z);
    int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};
    int nodesToVisit[64];
    int toVisitOffset = 0, currentNodeIndex = 0;
    while (true) {
        const LinearBVHNode *node = &nodes[currentNodeIndex];
        if (node->bounds.IntersectP(ray, invDir, dirIsNeg)) {

            if (node->nPrimitives > 0) {
                for (int i = 0; i < node->nPrimitives; ++i) {
                    if (primitives[node->primitivesOffset + i]->IntersectP(
                            ray)) {
                        return true;
                    }
                }
                if (toVisitOffset == 0) break;
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            } else {
                if (dirIsNeg[node->axis]) {
                    /// second child first
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->secondChildOffset;
                } else {
                    nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
            }
        } else {
            if (toVisitOffset == 0) break;
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
    }
    return false;
}

std::shared_ptr<BVH> CreateBVHAccelerator(
        std::vector<std::shared_ptr<Primitive>> prims, const ParamSet &ps) {
    int maxPrimsInNode = 4;
    return std::make_shared<BVH>(std::move(prims), maxPrimsInNode);
}
