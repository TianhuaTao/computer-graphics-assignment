//
// Created by Sam on 2020/6/15.
//

#include "integrator.h"
#include "scene.h"
#include "sampler/sampler.h"
#include "camera/camera.h"
#include "reflection/reflection.h"

Integrator::~Integrator() {}

Color RandomSampleOneLight(const Interaction &it, const Scene &scene,
                           MemoryBank &arena, Sampler &sampler,
                           bool handleMedia, const Distribution1D *lightDistrib) {
    // Randomly choose a single light to sample
    int nLights = int(scene.lights.size());
    if (nLights == 0) return Color(0.f);
    int lightNum;
    Float lightPdf;
    if (lightDistrib) {
        lightNum = lightDistrib->SampleDiscrete(sampler.Get1D(), &lightPdf);
        if (lightPdf == 0) return Color(0.f);
    } else {
        lightNum = std::min((int) (sampler.Get1D() * nLights), nLights - 1);
        lightPdf = Float(1) / nLights;
    }
    const std::shared_ptr<Light> &light = scene.lights[lightNum];
    Point2f uLight = sampler.Get2D();
    Point2f uScattering = sampler.Get2D();
    return DirectLightingEst(it, uScattering, *light, uLight,
                             scene, sampler, arena, handleMedia) / lightPdf;
}

/**
 * 计算单个光源样本的直接照明估算
 */
Color DirectLightingEst(const Interaction &it, const Point2f &uScattering,
                        const Light &light, const Point2f &uLight,
                        const Scene &scene, Sampler &sampler,
                        MemoryBank &arena, bool handleMedia, bool specular) {
    BxDFType bsdfFlags =
            specular ? BSDF_ALL : BxDFType(BSDF_ALL & ~BSDF_SPECULAR);
    Color Ld(0.f);

    Vector3f wi;
    Float lightPdf = 0, scatteringPdf = 0;
    VisibilityTester visibility;
    Color Li = light.Sample_Li(it, uLight, &wi, &lightPdf, &visibility);

    if (lightPdf > 0 && !Li.IsBlack()) {
        Color f;
        if (it.IsSurfaceInteraction()) {
            const SurfaceHit &isect = (const SurfaceHit &) it;
            f = isect.bsdf->f(isect.wo, wi, bsdfFlags) *
                AbsDot(wi, isect.shading.n);
            scatteringPdf = isect.bsdf->Pdf(isect.wo, wi, bsdfFlags);

        } else {
            const MediumInteraction &mi = (const MediumInteraction &) it;
            Float p = mi.phase->p(mi.wo, wi);
            f = Color(p);
            scatteringPdf = p;

        }
        if (!f.IsBlack()) {
            if (handleMedia) {
                Li *= visibility.Tr(scene, sampler);
            } else {
                if (!visibility.Unoccluded(scene)) {
                    Li = Color(0.f);
                }
            }
            if (!Li.IsBlack()) {
                if (IsDeltaLight(light.flags))
                    Ld += f * Li / lightPdf;
                else {
                    Float weight =
                            PowerHeuristic(1, lightPdf, 1, scatteringPdf);
                    Ld += f * Li * weight / lightPdf;
                }
            }
        }
    }

    // Sample BSDF with multiple importance sampling
    if (!IsDeltaLight(light.flags)) {
        Color f;
        bool sampledSpecular = false;
        if (it.IsSurfaceInteraction()) {
            BxDFType sampledType;
            const SurfaceHit &isect = (const SurfaceHit &) it;
            f = isect.bsdf->Sample_f(isect.wo, &wi, uScattering, &scatteringPdf,
                                     bsdfFlags, &sampledType);
            f *= AbsDot(wi, isect.shading.n);
            sampledSpecular = (sampledType & BSDF_SPECULAR) != 0;
        } else {
            const MediumInteraction &mi = (const MediumInteraction &) it;
            Float p = mi.phase->Sample_p(mi.wo, &wi, uScattering);
            f = Color(p);
            scatteringPdf = p;
        }

        if (!f.IsBlack() && scatteringPdf > 0) {
            Float weight = 1;
            if (!sampledSpecular) {
                lightPdf = light.Pdf_Li(it, wi);
                if (lightPdf == 0) return Ld;
                weight = PowerHeuristic(1, scatteringPdf, 1, lightPdf);
            }
            SurfaceHit lightIsect;
            Ray ray = it.SpawnRay(wi);
            Color Tr(1.f);
            bool foundSurfaceInteraction =
                    handleMedia ? scene.IntersectTr(ray, sampler, &lightIsect, &Tr)
                                : scene.Intersect(ray, &lightIsect);

            // Add light contribution from material sampling
            Color Li(0.f);
            if (foundSurfaceInteraction) {
                if (lightIsect.primitive->GetAreaLight() == &light)
                    Li = lightIsect.Le(-wi);
            } else
                Li = light.Le(ray);
            if (!Li.IsBlack()) Ld += f * Li * Tr * weight / scatteringPdf;
        }
    }
    return Ld;
}


void SamplerIntegrator::Render(const Scene &scene) {
    Preprocess(scene, *sampler);

    Bounds2i sampleBounds = camera->film->GetSampleBounds();
    Vector2i sampleExtent = sampleBounds.Diagonal();
    const int tileSize = 16;
    Point2i nTiles((sampleExtent.x + tileSize - 1) / tileSize,
                   (sampleExtent.y + tileSize - 1) / tileSize);
    std::cout << "Start Rendering" << endl;
    {
        /// 主要循环
        ParallelFor2D([&](Point2i tile) {

            MemoryBank bank;


            int seed = tile.y * nTiles.x + tile.x;
            std::unique_ptr<Sampler> tileSampler = sampler->Clone(seed);

            int x0 = sampleBounds.pMin.x + tile.x * tileSize;
            int x1 = std::min(x0 + tileSize, sampleBounds.pMax.x);
            int y0 = sampleBounds.pMin.y + tile.y * tileSize;
            int y1 = std::min(y0 + tileSize, sampleBounds.pMax.y);
            Bounds2i tileBounds(Point2i(x0, y0), Point2i(x1, y1));

            std::unique_ptr<FilmBlock> filmTile =
                    camera->film->GetFilmTile(tileBounds);

            for (Point2i pixel : tileBounds) {
                {
                    tileSampler->StartPixel(pixel);
                }

                if (!InsideExclusive(pixel, pixelBounds))
                    continue;
                do {

                    CameraSample cameraSample =
                            tileSampler->GetCameraSample(pixel);

                    // Generate camera ray for current sample
                    RayDifferential ray;
                    Float rayWeight =
                            camera->GenerateRayDifferential(cameraSample, &ray);
                    ray.ScaleDifferentials(
                            1 / std::sqrt((Float) tileSampler->samplesPerPixel));
                    // (rayWeight == 1)

                    // Evaluate radiance along camera ray
                    Color L(0.f);


                    if (rayWeight > 0) {    // always
                        L = Li(ray, scene, *tileSampler, bank);

                    }
                    // Issue warning if unexpected radiance value returned
                    if (L.HasNaNs()) {
                        cout << "nan\n";
                        L = Color(0.f);
                    } else if (L.y() < -1e-5) {
                        cout << "neg\n";
                        L = Color(0.f);
                    } else if (std::isinf(L.y())) {
                        cout << "inf\n";
                        L = Color(0.f);
                    }

                    // Add camera ray's contribution to image
                    filmTile->AddSample(cameraSample.pFilm, L, rayWeight);


                } while (tileSampler->StartNextSample());
            }


            camera->film->MergeFilmTile(std::move(filmTile));

        }, nTiles);
        std::cout << "Finish Rendering" << endl;

    }

    camera->film->WriteImage();
}

