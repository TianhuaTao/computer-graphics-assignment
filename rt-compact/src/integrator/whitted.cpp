//
// Created by Sam on 2020/6/17.
//

#include "whitted.h"
#include "scene.h"
#include "sampler/sampler.h"
#include "reflection/reflection.h"
Color WhittedIntegrator::Li(const RayDifferential &ray, const Scene &scene,
                            Sampler &sampler, MemoryBank &arena,
                            int depth) const {
    Color L(0.);
    // Find closest ray intersection or return background radiance
    SurfaceHit isect;
    if (!scene.Intersect(ray, &isect)) {
        for (const auto &light : scene.lights) L += light->Le(ray);
        return L;
    }else{
        // debug
        // return  {0.5};
    }


    const Normal3f &n = isect.shading.n;
    Vector3f wo = isect.wo;

    isect.ComputeScatteringFunctions(ray, arena);
    if (!isect.bsdf)
        return Li(isect.SpawnRay(ray.d), scene, sampler, arena, depth);

    L += isect.Le(wo);

    for (const auto &light : scene.lights) {
        Vector3f wi;
        Float pdf;
        VisibilityTester visibility;
        Color Li =
                light->Sample_Li(isect, sampler.Get2D(), &wi, &pdf, &visibility);
        if (Li.IsBlack() || pdf == 0) continue;
        Color f = isect.bsdf->f(wo, wi);
        if (!f.IsBlack() && visibility.Unoccluded(scene))
            L += f * Li * AbsDot(wi, n) / pdf;
    }
    if (depth + 1 < maxDepth) {
        L += SpecularReflect(ray, isect, scene, sampler, arena, depth);
        L += SpecularTransmit(ray, isect, scene, sampler, arena, depth);
    }
    return L;
}

WhittedIntegrator *CreateWhittedIntegrator(
        const ParamSet &params, std::shared_ptr<Sampler> sampler,
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
                                    Bounds2i{{pb[0], pb[2]}, {pb[1], pb[3]}});
            if (pixelBounds.Area() == 0)
                Error("Degenerate \"pixelbounds\" specified.");
        }
    }
    return new WhittedIntegrator(maxDepth, camera, sampler, pixelBounds);
}

Color WhittedIntegrator::SpecularReflect(
        const RayDifferential &ray, const SurfaceHit &isect,
        const Scene &scene, Sampler &sampler, MemoryBank &arena, int depth) const {
    Vector3f wo = isect.wo, wi;
    Float pdf;
    BxDFType type = BxDFType(BSDF_REFLECTION | BSDF_SPECULAR);
    Color f = isect.bsdf->Sample_f(wo, &wi, sampler.Get2D(), &pdf, type);

    const Normal3f &ns = isect.shading.n;
    if (pdf > 0.f && !f.IsBlack() && AbsDot(wi, ns) != 0.f) {
        RayDifferential rd = isect.SpawnRay(wi);
        if (ray.hasDifferentials) {
            rd.hasDifferentials = true;
            rd.rxOrigin = isect.p + isect.dpdx;
            rd.ryOrigin = isect.p + isect.dpdy;
            Normal3f dndx = isect.shading.dndu * isect.dudx +
                            isect.shading.dndv * isect.dvdx;
            Normal3f dndy = isect.shading.dndu * isect.dudy +
                            isect.shading.dndv * isect.dvdy;
            Vector3f dwodx = -ray.rxDirection - wo,
                    dwody = -ray.ryDirection - wo;
            Float dDNdx = Dot(dwodx, ns) + Dot(wo, dndx);
            Float dDNdy = Dot(dwody, ns) + Dot(wo, dndy);
            rd.rxDirection =
                    wi - dwodx + 2.f * Vector3f(Dot(wo, ns) * dndx + dDNdx * ns);
            rd.ryDirection =
                    wi - dwody + 2.f * Vector3f(Dot(wo, ns) * dndy + dDNdy * ns);
        }
        return f * Li(rd, scene, sampler, arena, depth + 1) * AbsDot(wi, ns) /
               pdf;
    } else
        return Color(0.f);
}

Color WhittedIntegrator::SpecularTransmit(
        const RayDifferential &ray, const SurfaceHit &isect,
        const Scene &scene, Sampler &sampler, MemoryBank &arena, int depth) const {
    Vector3f wo = isect.wo, wi;
    Float pdf;
    const Point3f &p = isect.p;
    const BSDF &bsdf = *isect.bsdf;
    Color f = bsdf.Sample_f(wo, &wi, sampler.Get2D(), &pdf,
                            BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR));
    Color L = Color(0.f);
    Normal3f ns = isect.shading.n;
    if (pdf > 0.f && !f.IsBlack() && AbsDot(wi, ns) != 0.f) {
        RayDifferential rd = isect.SpawnRay(wi);
        if (ray.hasDifferentials) {
            rd.hasDifferentials = true;
            rd.rxOrigin = p + isect.dpdx;
            rd.ryOrigin = p + isect.dpdy;

            Normal3f dndx = isect.shading.dndu * isect.dudx +
                            isect.shading.dndv * isect.dvdx;
            Normal3f dndy = isect.shading.dndu * isect.dudy +
                            isect.shading.dndv * isect.dvdy;

            Float eta = 1 / bsdf.eta;
            if (Dot(wo, ns) < 0) {
                eta = 1 / eta;
                ns = -ns;
                dndx = -dndx;
                dndy = -dndy;
            }


            Vector3f dwodx = -ray.rxDirection - wo,
                    dwody = -ray.ryDirection - wo;
            Float dDNdx = Dot(dwodx, ns) + Dot(wo, dndx);
            Float dDNdy = Dot(dwody, ns) + Dot(wo, dndy);

            Float mu = eta * Dot(wo, ns) - AbsDot(wi, ns);
            Float dmudx =
                    (eta - (eta * eta * Dot(wo, ns)) / AbsDot(wi, ns)) * dDNdx;
            Float dmudy =
                    (eta - (eta * eta * Dot(wo, ns)) / AbsDot(wi, ns)) * dDNdy;

            rd.rxDirection =
                    wi - eta * dwodx + Vector3f(mu * dndx + dmudx * ns);
            rd.ryDirection =
                    wi - eta * dwody + Vector3f(mu * dndy + dmudy * ns);
        }
        L = f * Li(rd, scene, sampler, arena, depth + 1) * AbsDot(wi, ns) / pdf;
    }
    return L;
}
