//
// Created by Sam on 2020/6/15.
//

#ifndef RT_COMPACT_INTEGRATOR_H
#define RT_COMPACT_INTEGRATOR_H

#include "rt_compact.h"
#include "math/geometry.h"
#include "camera/camera.h"

#include "prob.h"
/**
 * 积分器
 */
class Integrator {
public:
    virtual ~Integrator();

    /**
     * 渲染的主方法
     * @param scene
     */
    virtual void Render(const Scene &scene) = 0;
};


Color RandomSampleOneLight(const Interaction &it, const Scene &scene,
                           MemoryBank &arena, Sampler &sampler,
                           bool handleMedia = false,
                           const Distribution1D *lightDistrib = nullptr);
Color DirectLightingEst(const Interaction &it, const Point2f &uScattering,
                        const Light &light, const Point2f &uLight,
                        const Scene &scene, Sampler &sampler,
                        MemoryBank &arena, bool handleMedia = false,
                        bool specular = false);

/**
 * 基于采样的积分器
 */
class SamplerIntegrator : public Integrator {
public:

    SamplerIntegrator(std::shared_ptr<const Camera> camera,
                      std::shared_ptr<Sampler> sampler,
                      const Bounds2i &pixelBounds)
            : camera(camera), sampler(sampler), pixelBounds(pixelBounds) {}
    virtual void Preprocess(const Scene &scene, Sampler &sampler) {}
    void Render(const Scene &scene);
    virtual Color Li(const RayDifferential &ray, const Scene &scene,
                     Sampler &sampler, MemoryBank &arena,
                     int depth = 0) const = 0;

protected:
    std::shared_ptr<const Camera> camera;
private:
    std::shared_ptr<Sampler> sampler;
    const Bounds2i pixelBounds;
};


#endif //RT_COMPACT_INTEGRATOR_H
