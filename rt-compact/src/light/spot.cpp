//
// Created by Sam on 2020/6/19.
//

#include "spot.h"
#include "memory.h"
#include "prob.h"
#include "reflection/reflection.h"
SpotLight::SpotLight(const Transform &LightToWorld,
                     const MediumInterface &mediumInterface, const Color &I,
                     Float totalWidth, Float falloffStart)
        : Light((int)LightFlags::DeltaPosition, LightToWorld, mediumInterface),
          pLight(LightToWorld(Point3f(0, 0, 0))),
          I(I),
          cosTotalWidth(std::cos(Radians(totalWidth))),
          cosFalloffStart(std::cos(Radians(falloffStart))) {}

Color SpotLight::Sample_Li(const Interaction &ref, const Point2f &u,
                              Vector3f *wi, Float *pdf,
                              VisibilityTester *vis) const {
    *wi = Normalize(pLight - ref.p);
    *pdf = 1.f;
    *vis =
            VisibilityTester(ref, Interaction(pLight, ref.time, mediumInterface));
    return I * Falloff(-*wi) / DistanceSquared(pLight, ref.p);
}

Float SpotLight::Falloff(const Vector3f &w) const {
    Vector3f wl = Normalize(WorldToLight(w));
    Float cosTheta = wl.z;
    if (cosTheta < cosTotalWidth) return 0;
    if (cosTheta >= cosFalloffStart) return 1;

    Float delta =
            (cosTheta - cosTotalWidth) / (cosFalloffStart - cosTotalWidth);
    return (delta * delta) * (delta * delta);
}

Color SpotLight::Power() const {
    return I * 2 * Pi * (1 - .5f * (cosFalloffStart + cosTotalWidth));
}

Float SpotLight::Pdf_Li(const Interaction &, const Vector3f &) const {
    return 0.f;
}

std::shared_ptr<SpotLight> CreateSpotLight(const Transform &l2w,
                                           const Medium *medium,
                                           const ParamSet &paramSet) {
    Color I = paramSet.FindOneSpectrum("I", Color(1.0));
    Color sc = paramSet.FindOneSpectrum("scale", Color(1.0));
    Float coneangle = paramSet.FindOneFloat("coneangle", 30.);
    Float conedelta = paramSet.FindOneFloat("conedeltaangle", 5.);

    Point3f from = paramSet.FindOnePoint3f("from", Point3f(0, 0, 0));
    Point3f to = paramSet.FindOnePoint3f("to", Point3f(0, 0, 1));
    Vector3f dir = Normalize(to - from);
    Vector3f du, dv;
    CoordinateSystem(dir, &du, &dv);
    Transform dirToZ =
            Transform(Matrix4x4(du.x, du.y, du.z, 0., dv.x, dv.y, dv.z, 0., dir.x,
                                dir.y, dir.z, 0., 0, 0, 0, 1.));
    Transform light2world =
            l2w * Translate(Vector3f(from.x, from.y, from.z)) * Inverse(dirToZ);
    return std::make_shared<SpotLight>(light2world, medium, I * sc, coneangle,
                                       coneangle - conedelta);
}
