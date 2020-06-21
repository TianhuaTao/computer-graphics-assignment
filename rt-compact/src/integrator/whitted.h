//
// Created by Sam on 2020/6/17.
//

#ifndef RT_COMPACT_WHITTED_H
#define RT_COMPACT_WHITTED_H

#include "integrator.h"

/**
 * 简单的递归光线追踪积分
 */
class WhittedIntegrator : public SamplerIntegrator {
public:

    WhittedIntegrator(int maxDepth, std::shared_ptr<const Camera> camera,
                      std::shared_ptr<Sampler> sampler,
                      const Bounds2i &pixelBounds)
            : SamplerIntegrator(camera, sampler, pixelBounds), maxDepth(maxDepth) {}
    Color Li(const RayDifferential &ray, const Scene &scene,
             Sampler &sampler, MemoryBank &arena, int depth) const;
    Color SpecularReflect(const RayDifferential &ray,
                          const SurfaceHit &isect,
                          const Scene &scene, Sampler &sampler,
                          MemoryBank &arena, int depth) const;
    Color SpecularTransmit(const RayDifferential &ray,
                           const SurfaceHit &isect,
                           const Scene &scene, Sampler &sampler,
                           MemoryBank &arena, int depth) const;
private:

    const int maxDepth;
};

WhittedIntegrator *CreateWhittedIntegrator(
        const ParamSet &params, std::shared_ptr<Sampler> sampler,
        std::shared_ptr<const Camera> camera);



#endif //RT_COMPACT_WHITTED_H
