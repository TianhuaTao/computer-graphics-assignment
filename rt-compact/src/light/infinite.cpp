//
// Created by Sam on 2020/6/17.
//

#include "infinite.h"
#include "parallel.h"
#include "util/imageio.h"
InfiniteAreaLight::InfiniteAreaLight(const Transform &LightToWorld,
                                     const Color &L, int nSamples,
                                     const std::string &texmap)
        : Light((int)LightFlags::Infinite, LightToWorld, MediumInterface(),
                nSamples) ,color(L){
    Point2i resolution;
    std::unique_ptr<RGBColor[]> texels(nullptr);
    if (texmap != "") {
        texels = std::unique_ptr<RGBColor[]>(ReadImagePFM(texmap, &resolution.x,&resolution.y));
        if (texels)
            for (int i = 0; i < resolution.x * resolution.y; ++i)
                texels[i] *= L.ToRGBSpectrum();
    }
    if (!texels) {
        resolution.x = resolution.y = 1;
        texels = std::unique_ptr<RGBColor[]>(new RGBColor[1]);
        texels[0] = L.ToRGBSpectrum();
    }
    Lmap.reset(new MIPMap<RGBColor>(resolution, texels.get()));


    int width = 2 * Lmap->Width(), height = 2 * Lmap->Height();
    std::unique_ptr<Float[]> img(new Float[width * height]);
    float fwidth = 0.5f / std::min(width, height);
    ParallelFor(
            [&](int64_t v) {
                Float vp = (v + .5f) / (Float)height;
                Float sinTheta = std::sin(Pi * (v + .5f) / height);
                for (int u = 0; u < width; ++u) {
                    Float up = (u + .5f) / (Float)width;
                    img[u + v * width] = Lmap->Lookup(Point2f(up, vp), fwidth).y();
                    img[u + v * width] *= sinTheta;
                }
            },
            height, 32);

    distribution.reset(new Distribution2D(img.get(), width, height));
}

Color InfiniteAreaLight::Power() const {
    return Pi * worldRadius * worldRadius *
           Color(Lmap->Lookup(Point2f(.5f, .5f), .5f),
                 SpectrumType::Illuminant);
}

Color InfiniteAreaLight::Le(const RayDifferential &ray) const {

    Vector3f w = Normalize(WorldToLight(ray.d));
    Point2f st(SphericalPhi(w) * Inv2Pi, SphericalTheta(w) * InvPi);
    return Color(Lmap->Lookup(st), SpectrumType::Illuminant);
}

Color InfiniteAreaLight::Sample_Li(const Interaction &ref, const Point2f &u,
                                   Vector3f *wi, Float *pdf,
                                   VisibilityTester *vis) const {

    Float mapPdf;
    Point2f uv = distribution->SampleContinuous(u, &mapPdf);
    if (mapPdf == 0) return Color(0.f);

    Float theta = uv[1] * Pi, phi = uv[0] * 2 * Pi;
    Float cosTheta = std::cos(theta), sinTheta = std::sin(theta);
    Float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
    *wi =
            LightToWorld(Vector3f(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta));

    *pdf = mapPdf / (2 * Pi * Pi * sinTheta);
    if (sinTheta == 0) *pdf = 0;

    *vis = VisibilityTester(ref, Interaction(ref.p + *wi * (2 * worldRadius),
                                             ref.time, mediumInterface));
    return Color(Lmap->Lookup(uv), SpectrumType::Illuminant);
}

Float InfiniteAreaLight::Pdf_Li(const Interaction &, const Vector3f &w) const {


    Vector3f wi = WorldToLight(w);
    Float theta = SphericalTheta(wi), phi = SphericalPhi(wi);
    Float sinTheta = std::sin(theta);
    if (sinTheta == 0) return 0;
    return distribution->Pdf(Point2f(phi * Inv2Pi, theta * InvPi)) /
           (2 * Pi * Pi * sinTheta);
}

std::shared_ptr<InfiniteAreaLight> CreateInfiniteLight(
        const Transform &light2world, const ParamSet &paramSet) {
    Color L = paramSet.FindOneSpectrum("L", Color(1.0));
    Color sc = paramSet.FindOneSpectrum("scale", Color(1.0));
    std::string texmap = paramSet.FindOneFilename("mapname", "");
    int nSamples = paramSet.FindOneInt("samples",
                                       paramSet.FindOneInt("nsamples", 1));

    return std::make_shared<InfiniteAreaLight>(light2world, L * sc, nSamples,
                                               texmap);
}