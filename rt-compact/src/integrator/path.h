//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_PATH_H
#define RT_COMPACT_PATH_H

#include "integrator.h"


class PathIntegrator : public SamplerIntegrator {
public:
    PathIntegrator(int maxDepth, std::shared_ptr<const Camera> camera,
                   std::shared_ptr<Sampler> sampler,
                   const Bounds2i &pixelBounds, Float rrThreshold = 1);

    void Preprocess(const Scene &scene, Sampler &sampler) override ;
    Color Li(const RayDifferential &ray, const Scene &scene,
             Sampler &sampler, MemoryBank &bank, int depth) const override;

private:
    const int maxDepth;
    const Float rrThreshold;
};

PathIntegrator *CreatePathIntegrator(const ParamSet &params,
                                     std::shared_ptr<Sampler> sampler,
                                     std::shared_ptr<const Camera> camera);



#endif //RT_COMPACT_PATH_H
