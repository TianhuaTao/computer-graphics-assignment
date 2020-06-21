//
// Created by Sam on 2020/6/15.
//

#ifndef RT_COMPACT_TRIANGLE_H
#define RT_COMPACT_TRIANGLE_H

#include "rt_compact.h"
#include "shape.h"
#include "math/geometry.h"
#include "texture/texture.h"
struct TriangleMesh {
    // TriangleMesh Public Methods
    TriangleMesh(const Transform &ObjectToWorld, int nTriangles,
                 const int *vertexIndices, int nVertices, const Point3f *P,
                 const Vector3f *S, const Normal3f *N, const Point2f *uv,
                 const std::shared_ptr<Texture<Float>> &alphaMask,
                 const std::shared_ptr<Texture<Float>> &shadowAlphaMask,
                 const int *faceIndices);

    // TriangleMesh Data
    const int nTriangles, nVertices;
    std::vector<int> vertexIndices;
    std::unique_ptr<Point3f[]> p;
    std::unique_ptr<Normal3f[]> n;
    std::unique_ptr<Vector3f[]> s;
    std::unique_ptr<Point2f[]> uv;
    std::shared_ptr<Texture<Float>> alphaMask, shadowAlphaMask;
    std::vector<int> faceIndices;
};

class Triangle : public Shape {
public:
    Triangle(const Transform *ObjectToWorld, const Transform *WorldToObject,
             bool reverseOrientation, const std::shared_ptr<TriangleMesh> &mesh,
             int triNumber)
            : Shape(ObjectToWorld, WorldToObject, reverseOrientation), mesh(mesh) {
        v = &mesh->vertexIndices[3 * triNumber];

        faceIndex = mesh->faceIndices.size() ? mesh->faceIndices[triNumber] : 0;
    }
    Bounds3f ObjectBound() const;
    Bounds3f WorldBound() const;
    bool Intersect(const Ray &ray, Float *tHit, SurfaceHit *isect,
                   bool testAlphaTexture = true) const;
    bool IntersectP(const Ray &ray, bool testAlphaTexture = true) const;
    Float Area() const;

    using Shape::Sample;  // Bring in the other Sample() overload.
    Interaction Sample(const Point2f &u, Float *pdf) const;



private:
    void GetUVs(Point2f uv[3]) const {
        if (mesh->uv) {
            uv[0] = mesh->uv[v[0]];
            uv[1] = mesh->uv[v[1]];
            uv[2] = mesh->uv[v[2]];
        } else {
            uv[0] = Point2f(0, 0);
            uv[1] = Point2f(1, 0);
            uv[2] = Point2f(1, 1);
        }
    }

    std::shared_ptr<TriangleMesh> mesh;
    const int *v;
    int faceIndex;
};

std::vector<std::shared_ptr<Shape>> CreateTriangleMesh(
        const Transform *o2w, const Transform *w2o, bool reverseOrientation,
        int nTriangles, const int *vertexIndices, int nVertices, const Point3f *p,
        const Vector3f *s, const Normal3f *n, const Point2f *uv,
        const std::shared_ptr<Texture<Float>> &alphaTexture,
        const std::shared_ptr<Texture<Float>> &shadowAlphaTexture,
        const int *faceIndices = nullptr);
std::vector<std::shared_ptr<Shape>> CreateTriangleMeshShape(
        const Transform *o2w, const Transform *w2o, bool reverseOrientation,
        const ParamSet &params,
        std::map<std::string, std::shared_ptr<Texture<Float>>> *floatTextures =
        nullptr);



#endif //RT_COMPACT_TRIANGLE_H
