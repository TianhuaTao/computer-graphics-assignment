//
// Created by Sam on 2020/6/15.
//

#include "scene.h"


bool Scene::Intersect(const Ray &ray, SurfaceHit *isect) const {

    return group->Intersect(ray, isect);
}

bool Scene::IntersectP(const Ray &ray) const {

    return group->IntersectP(ray);
}

bool Scene::IntersectTr(Ray ray, Sampler &sampler, SurfaceHit *isect,
                        Color *Tr) const {
    *Tr = Color(1.f);
    while (true) {
        bool hitSurface = Intersect(ray, isect);
        if (ray.medium) *Tr *= ray.medium->Tr(ray, sampler);

        if (!hitSurface) return false;
        if (isect->primitive->GetMaterial() != nullptr) return true;
        ray = isect->SpawnRay(ray.d);
    }
}
