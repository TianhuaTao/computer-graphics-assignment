//
// Created by Sam on 2020/6/16.
//

#include "light.h"
#include "color.h"
#include "scene.h"
Light::Light(int flags, const Transform &LightToWorld,
             const MediumInterface &mediumInterface, int nSamples)
        : flags(flags),
          mediumInterface(mediumInterface),
          LightToWorld(LightToWorld),
          WorldToLight(Inverse(LightToWorld)) {

}

Light::~Light() {}

bool VisibilityTester::Unoccluded(const Scene &scene) const {
    return !scene.IntersectP(p0.SpawnRayTo(p1));
}

Color VisibilityTester::Tr(const Scene &scene, Sampler &sampler) const {
    Ray ray(p0.SpawnRayTo(p1));
    Color Tr(1.f);
    while (true) {
        SurfaceHit isect;
        bool hitSurface = scene.Intersect(ray, &isect);

        if (hitSurface && isect.primitive->GetMaterial() != nullptr)
            return Color(0.0f);


        if (ray.medium) Tr *= ray.medium->Tr(ray, sampler);


        if (!hitSurface) break;
        ray = isect.SpawnRayTo(p1);
    }
    return Tr;
}

Color Light::Le(const RayDifferential &ray) const { return Color(0.f); }

AreaLight::AreaLight(const Transform &LightToWorld, const MediumInterface &medium,
                     int nSamples)
        : Light((int)LightFlags::Area, LightToWorld, medium, nSamples) {

}