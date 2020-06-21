//
// Created by Sam on 2020/6/15.
//

#ifndef RT_COMPACT_PRIMITIVE_H
#define RT_COMPACT_PRIMITIVE_H

#include "light/light.h"
#include "material/material.h"
#include "rt_compact.h"
#include "math/geometry.h"
#include "math/transform.h"
#include "medium/medium.h"

/**
 * 求交图元
 */
class Primitive {
public:

    virtual ~Primitive();
    virtual Bounds3f WorldBound() const = 0;
    virtual bool Intersect(const Ray &r, SurfaceHit *) const = 0;
    virtual bool IntersectP(const Ray &r) const = 0;
    virtual const AreaLight *GetAreaLight() const = 0;
    virtual const Material *GetMaterial() const = 0;
    virtual void ComputeScatteringFunctions(SurfaceHit *isect,
                                            MemoryBank &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const = 0;
};


class GeometricPrimitive : public Primitive {
public:

    virtual Bounds3f WorldBound() const;
    virtual bool Intersect(const Ray &r, SurfaceHit *isect) const;
    virtual bool IntersectP(const Ray &r) const;
    GeometricPrimitive(const std::shared_ptr<Shape> &shape,
                       const std::shared_ptr<Material> &material,
                       const std::shared_ptr<AreaLight> &areaLight,
                       const MediumInterface &mediumInterface);
    const AreaLight *GetAreaLight() const;
    const Material *GetMaterial() const;
    void ComputeScatteringFunctions(SurfaceHit *isect,
                                    MemoryBank &arena, TransportMode mode,
                                    bool allowMultipleLobes) const;

private:

    std::shared_ptr<Shape> shape;
    std::shared_ptr<Material> material;
    std::shared_ptr<AreaLight> areaLight;
    MediumInterface mediumInterface;
};


class TransformedPrimitive : public Primitive {
public:

    TransformedPrimitive(std::shared_ptr<Primitive> &primitive,
                         const AnimatedTransform &PrimitiveToWorld);
    bool Intersect(const Ray &r, SurfaceHit *in) const;
    bool IntersectP(const Ray &r) const;
    const AreaLight *GetAreaLight() const { return nullptr; }
    const Material *GetMaterial() const { return nullptr; }
    void ComputeScatteringFunctions(SurfaceHit *isect,
                                    MemoryBank &arena, TransportMode mode,
                                    bool allowMultipleLobes) const {

    }
    Bounds3f WorldBound() const {
        return PrimitiveToWorld.MotionBounds(primitive->WorldBound());
    }

private:
    std::shared_ptr<Primitive> primitive;
    const AnimatedTransform PrimitiveToWorld;
};


class Group : public Primitive {
public:

    const AreaLight *GetAreaLight() const;
    const Material *GetMaterial() const;
    void ComputeScatteringFunctions(SurfaceHit *isect,
                                    MemoryBank &arena, TransportMode mode,
                                    bool allowMultipleLobes) const;
};

#endif //RT_COMPACT_PRIMITIVE_H
