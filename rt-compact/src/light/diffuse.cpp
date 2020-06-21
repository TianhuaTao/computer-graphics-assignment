//
// Created by Sam on 2020/6/17.
//

#include "diffuse.h"
#include "shape/triangle.h"
#include "prob.h"
DiffuseAreaLight::DiffuseAreaLight(const Transform &LightToWorld,
                                   const MediumInterface &mediumInterface,
                                   const Color &Lemit, int nSamples,
                                   const std::shared_ptr<Shape> &shape,
                                   bool twoSided)
        : AreaLight(LightToWorld, mediumInterface, nSamples),
          Lemit(Lemit),
          shape(shape),
          twoSided(twoSided),
          area(shape->Area()) {

    if (WorldToLight.HasScale() &&
        dynamic_cast<const Triangle *>(shape.get()) == nullptr)
        Warning(
                "Scaling detected in world to light transformation!");
}

Color DiffuseAreaLight::Power() const {
    return (twoSided ? 2 : 1) * Lemit * area * Pi;
}

Color DiffuseAreaLight::Sample_Li(const Interaction &ref, const Point2f &u,
                                  Vector3f *wi, Float *pdf,
                                  VisibilityTester *vis) const {

    Interaction pShape = shape->Sample(ref, u, pdf);
    pShape.mediumInterface = mediumInterface;
    if (*pdf == 0 || (pShape.p - ref.p).LengthSquared() == 0) {
        *pdf = 0;
        return 0.f;
    }
    *wi = Normalize(pShape.p - ref.p);
    *vis = VisibilityTester(ref, pShape);
    return L(pShape, -*wi);
}

Float DiffuseAreaLight::Pdf_Li(const Interaction &ref,
                               const Vector3f &wi) const {

    return shape->Pdf(ref, wi);
}

std::shared_ptr<AreaLight> CreateDiffuseAreaLight(
        const Transform &light2world, const Medium *medium,
        const ParamSet &paramSet, const std::shared_ptr<Shape> &shape) {
    Color L = paramSet.FindOneSpectrum("L", Color(1.0));
    Color sc = paramSet.FindOneSpectrum("scale", Color(1.0));
    int nSamples = paramSet.FindOneInt("samples",
                                       paramSet.FindOneInt("nsamples", 1));
    bool twoSided = paramSet.FindOneBool("twosided", false);

    return std::make_shared<DiffuseAreaLight>(light2world, medium, L * sc,
                                              nSamples, shape, twoSided);
}
