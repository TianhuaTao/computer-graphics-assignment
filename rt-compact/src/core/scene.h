//
// Created by Sam on 2020/6/15.
//

#ifndef RT_COMPACT_SCENE_H
#define RT_COMPACT_SCENE_H

#include "rt_compact.h"
#include "math/geometry.h"
#include "primitive.h"

class Scene {
public:
    Scene(std::shared_ptr<Primitive> group,
          const std::vector<std::shared_ptr<Light>> &lights)
            : lights(lights), group(group) {
        worldBound = group->WorldBound();
        for (const auto &light : lights) {
            light->Preprocess(*this);
            if (light->flags & (int)LightFlags::Infinite)
                infiniteLights.push_back(light);
        }
    }
    const Bounds3f &WorldBound() const { return worldBound; }
    /**
     * 完整的求交函数，转发给其下的 group
     * @param ray
     * @param isect
     * @return 是否有交点
     */
    bool Intersect(const Ray &ray, SurfaceHit *isect) const;

    /**
     * 预测求交函数，只返回结果
     * @param ray
     * @return
     */
    bool IntersectP(const Ray &ray) const;


    bool IntersectTr(Ray ray, Sampler &sampler, SurfaceHit *isect,
                     Color *transmittance) const;


    std::vector<std::shared_ptr<Light>> lights;

    std::vector<std::shared_ptr<Light>> infiniteLights;

private:
    std::shared_ptr<Primitive> group;   // 包围盒
    Bounds3f worldBound;
};


#endif //RT_COMPACT_SCENE_H
