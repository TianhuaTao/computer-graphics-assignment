//
// Created by Sam on 2020/6/16.
//

#include <core/prob.h>
#include "distant.h"

DistantLight::DistantLight(const Transform &LightToWorld, const Color &L,
                           const Vector3f &wLight)
        : Light((int)LightFlags::DeltaDirection, LightToWorld, MediumInterface()),
          L(L),
          wLight(Normalize(LightToWorld(wLight))) {}

Color DistantLight::Sample_Li(const Interaction &ref, const Point2f &u,
                              Vector3f *wi, Float *pdf,
                              VisibilityTester *vis) const {

    *wi = wLight;
    *pdf = 1;
    Point3f pOutside = ref.p + wLight * (2 * worldRadius);
    *vis =
            VisibilityTester(ref, Interaction(pOutside, ref.time, mediumInterface));
    return L;
}

Color DistantLight::Power() const {
    return L * Pi * worldRadius * worldRadius;
}

Float DistantLight::Pdf_Li(const Interaction &, const Vector3f &) const {
    return 0.f;
}

std::shared_ptr<DistantLight> CreateDistantLight(const Transform &light2world,
                                                 const ParamSet &paramSet) {
    Color L = paramSet.FindOneSpectrum("L", Color(1.0));
    Color sc = paramSet.FindOneSpectrum("scale", Color(1.0));
    Point3f from = paramSet.FindOnePoint3f("from", Point3f(0, 0, 0));
    Point3f to = paramSet.FindOnePoint3f("to", Point3f(0, 0, 1));
    Vector3f dir = from - to;
    return std::make_shared<DistantLight>(light2world, L * sc, dir);
}
