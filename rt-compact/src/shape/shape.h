//
// Created by Sam on 2020/6/15.
//

#ifndef RT_COMPACT_SHAPE_H
#define RT_COMPACT_SHAPE_H

#include "rt_compact.h"
#include "math/geometry.h"

/**
 * 形状基类
 */
class Shape {
public:

    Shape(const Transform *ObjectToWorld, const Transform *WorldToObject,
          bool reverseOrientation);
    virtual ~Shape();
    virtual Bounds3f ObjectBound() const = 0;
    virtual Bounds3f WorldBound() const;
    virtual bool Intersect(const Ray &ray, Float *tHit,
                           SurfaceHit *isect,
                           bool testAlphaTexture = true) const = 0;
    virtual bool IntersectP(const Ray &ray,
                            bool testAlphaTexture = true) const {
        return Intersect(ray, nullptr, nullptr, testAlphaTexture);
    }
    virtual Float Area() const = 0;

    virtual Interaction Sample(const Point2f &u, Float *pdf) const = 0;
    virtual Float Pdf(const Interaction &) const { return 1 / Area(); }


    virtual Interaction Sample(const Interaction &ref, const Point2f &u,
                               Float *pdf) const;
    virtual Float Pdf(const Interaction &ref, const Vector3f &wi) const;


    const Transform *ObjectToWorld, *WorldToObject;
    const bool reverseOrientation;
    const bool transformSwapsHandedness;
};

#endif //RT_COMPACT_SHAPE_H
