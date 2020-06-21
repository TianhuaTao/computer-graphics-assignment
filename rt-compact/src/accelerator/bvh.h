//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_BVH_H
#define RT_COMPACT_BVH_H

#include "primitive.h"
struct BVHNode;


struct BVHPrimitiveInfo;

struct LinearBVHNode;

/**
 * 包围盒加速算法
 * 通过按照 primitive 中心的某一坐标排序，使用二分法，每次把所有的元素分为相等数目的两组
 * 递归建树，再转化为利于访问的线性结构
 */
class BVH : public Group {
public:

    BVH(std::vector<std::shared_ptr<Primitive>> p,
        int maxPrimsInNode = 1);
    Bounds3f WorldBound() const;
    ~BVH();
    bool Intersect(const Ray &ray, SurfaceHit *isect) const;
    bool IntersectP(const Ray &ray) const;

private:

    BVHNode *recursiveBuild(
            MemoryBank &arena, std::vector<BVHPrimitiveInfo> &primitiveInfo,
            int start, int end, int *totalNodes,
            std::vector<std::shared_ptr<Primitive>> &orderedPrims);


    int flattenBVHTree(BVHNode *node, int *offset);

    const int maxPrimsInNode;
    std::vector<std::shared_ptr<Primitive>> primitives;
    LinearBVHNode *nodes = nullptr;
};

std::shared_ptr<BVH> CreateBVHAccelerator(
        std::vector<std::shared_ptr<Primitive>> prims, const ParamSet &ps);



#endif //RT_COMPACT_BVH_H
