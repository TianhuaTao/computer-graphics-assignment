//
// Created by Sam on 2020/6/15.
//

#include "primitive.h"
#include "shape/shape.h"
#include "interaction.h"
Primitive::~Primitive() {}
const AreaLight *Group::GetAreaLight() const {
    return nullptr;
}

const Material *Group::GetMaterial() const {
    return nullptr;
}

void Group::ComputeScatteringFunctions(SurfaceHit *isect,
                                       MemoryBank &arena,
                                       TransportMode mode,
                                       bool allowMultipleLobes) const {
}

TransformedPrimitive::TransformedPrimitive(std::shared_ptr<Primitive> &primitive,
                                           const AnimatedTransform &PrimitiveToWorld)
        : primitive(primitive), PrimitiveToWorld(PrimitiveToWorld) {
}

bool TransformedPrimitive::Intersect(const Ray &r,
                                     SurfaceHit *isect) const {
    Transform InterpolatedPrimToWorld;
    PrimitiveToWorld.Interpolate(r.time, &InterpolatedPrimToWorld);
    Ray ray = Inverse(InterpolatedPrimToWorld)(r);
    if (!primitive->Intersect(ray, isect)) return false;
    r.tMax = ray.tMax;
    if (!InterpolatedPrimToWorld.IsIdentity())
        *isect = InterpolatedPrimToWorld(*isect);

    return true;
}

bool TransformedPrimitive::IntersectP(const Ray &r) const {
    Transform InterpolatedPrimToWorld;
    PrimitiveToWorld.Interpolate(r.time, &InterpolatedPrimToWorld);
    Transform InterpolatedWorldToPrim = Inverse(InterpolatedPrimToWorld);
    return primitive->IntersectP(InterpolatedWorldToPrim(r));
}

GeometricPrimitive::GeometricPrimitive(const std::shared_ptr<Shape> &shape,
                                       const std::shared_ptr<Material> &material,
                                       const std::shared_ptr<AreaLight> &areaLight,
                                       const MediumInterface &mediumInterface)
        : shape(shape),
          material(material),
          areaLight(areaLight),
          mediumInterface(mediumInterface) {

}

Bounds3f GeometricPrimitive::WorldBound() const { return shape->WorldBound(); }

bool GeometricPrimitive::IntersectP(const Ray &r) const {
    return shape->IntersectP(r);
}

bool GeometricPrimitive::Intersect(const Ray &r,
                                   SurfaceHit *isect) const {
    Float tHit;
    if (!shape->Intersect(r, &tHit, isect)) return false;
    r.tMax = tHit;
    isect->primitive = this;

    if (mediumInterface.IsMediumTransition())
        isect->mediumInterface = mediumInterface;
    else
        isect->mediumInterface = MediumInterface(r.medium);
    return true;
}

const AreaLight *GeometricPrimitive::GetAreaLight() const {
    return areaLight.get();
}

const Material *GeometricPrimitive::GetMaterial() const {
    return material.get();
}

void GeometricPrimitive::ComputeScatteringFunctions(
        SurfaceHit *isect, MemoryBank &arena, TransportMode mode,
        bool allowMultipleLobes) const {

    if (material)
        material->ComputeScatteringFunctions(isect, arena, mode,
                                             allowMultipleLobes);
}
