//
// Created by Sam on 2020/6/16.
//

#include <core/interaction.h>
#include <reflection/reflection.h>
#include "path.h"
#include "camera/camera.h"
#include "camera/film.h"
#include "scene.h"
#include "sampler/sampler.h"


PathIntegrator::PathIntegrator(int maxDepth,
                               std::shared_ptr<const Camera> camera,
                               std::shared_ptr<Sampler> sampler,
                               const Bounds2i &pixelBounds, Float rrThreshold)
        : SamplerIntegrator(camera, sampler, pixelBounds),
          maxDepth(maxDepth),
          rrThreshold(rrThreshold)
{}

void PathIntegrator::Preprocess(const Scene &scene, Sampler &sampler) {

}

Color PathIntegrator::Li(const RayDifferential &r, const Scene &scene,
                         Sampler &sampler, MemoryBank &bank,
                         int) const {
    Color L(0.f), beta(1.f);
    RayDifferential ray(r);
    bool specularBounce = false;
    int bounces;

    Float etaScale = 1;

    for (bounces = 0;; ++bounces) {

        SurfaceHit isect;
        bool foundIntersection = scene.Intersect(ray, &isect);

        // 介质采样
        MediumInteraction mi;
        if (ray.medium) beta *= ray.medium->Sample(ray, sampler, bank, &mi);
        if (beta.IsBlack()) break;

        // 处理 medium 或者表面
        if (mi.IsValid()) {
            if (bounces >= maxDepth) break;

            L += beta * RandomSampleOneLight(mi, scene, bank, sampler, true
            );

            Vector3f wo = -ray.d, wi;
            mi.phase->Sample_p(wo, &wi, sampler.Get2D());
            ray = mi.SpawnRay(wi);
            specularBounce = false;
        } else {
            // 判断相交物体是否发光
            if (bounces == 0 || specularBounce) {
                if (foundIntersection) {
                    L += beta * isect.Le(-ray.d);
                } else {
                    for (const auto &light : scene.infiniteLights)  // 环境背景灯光
                        L += beta * light->Le(ray);
                }
            }

            // 终止条件
            if (!foundIntersection || bounces >= maxDepth) break;

            // 计算散射方程，跳过 medium 边界
            isect.ComputeScatteringFunctions(ray, bank, true);
            if (!isect.bsdf) {
                ray = isect.SpawnRay(ray.d);
                bounces--;
                continue;
            }


            // 对灯光采样，找到路径贡献
            L += beta * RandomSampleOneLight(isect, scene, bank, sampler);


            // 对 BSDF 采样获得新的方向
            Vector3f wo = -ray.d, wi;
            Float pdf;
            BxDFType flags;
            Color f = isect.bsdf->Sample_f(wo, &wi, sampler.Get2D(), &pdf,
                                           BSDF_ALL, &flags);
            if (f.IsBlack() || pdf == 0.f) break;
            beta *= f * AbsDot(wi, isect.shading.n) / pdf;
            specularBounce = (flags & BSDF_SPECULAR) != 0;
            if ((flags & BSDF_SPECULAR) && (flags & BSDF_TRANSMISSION)) {
                Float eta = isect.bsdf->eta;
                // 根据是进入还是离开，调整系数
                etaScale *= (Dot(wo, isect.n) > 0) ? (eta * eta) : 1 / (eta * eta);
            }
            ray = isect.SpawnRay(wi);   // 下一个光线方向


        }

        // 按一定概率终止光线，概率随着折射次数增加而增加
        Color rrBeta = beta * etaScale;
        if (rrBeta.MaxComponentValue() < rrThreshold && bounces > 3) {
            Float q = std::max((Float) .05, 1 - rrBeta.MaxComponentValue());
            if (sampler.Get1D() <= q) break;
            beta /= 1 - q;
        }
    }

    return L;
}

PathIntegrator *CreatePathIntegrator(const ParamSet &params,
                                     std::shared_ptr<Sampler> sampler,
                                     std::shared_ptr<const Camera> camera) {
    int maxDepth = params.FindOneInt("maxdepth", 5);
    int np;
    const int *pb = params.FindInt("pixelbounds", &np);
    Bounds2i pixelBounds = camera->film->GetSampleBounds();
    if (pb) {
        if (np != 4)
            Error("Expected four values for \"pixelbounds\" parameter. Got %d.",
                  np);
        else {
            pixelBounds = Intersect(pixelBounds,
                                    Bounds2i{{pb[0], pb[2]},
                                             {pb[1], pb[3]}});
            if (pixelBounds.Area() == 0)
                Error("Degenerate \"pixelbounds\" specified.");
        }
    }
    Float rrThreshold = params.FindOneFloat("rrthreshold", 1.);
    std::string lightStrategy =
            params.FindOneString("lightsamplestrategy", "spatial");
    return new PathIntegrator(maxDepth, camera, sampler, pixelBounds,
                              rrThreshold);
}


