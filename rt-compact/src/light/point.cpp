//
// Created by Sam on 2020/6/16.
//

#include "point.h"
#include "prob.h"
Color PointLight::Sample_Li(const Interaction &ref, const Point2f &u,
                            Vector3f *wi, Float *pdf,
                            VisibilityTester *vis) const {

    *wi = Normalize(pLight - ref.p);
    *pdf = 1.f;
    *vis =
            VisibilityTester(ref, Interaction(pLight, ref.time, mediumInterface));
    return I / DistanceSquared(pLight, ref.p);
}

Color PointLight::Power() const { return 4 * Pi * I; }

Float PointLight::Pdf_Li(const Interaction &, const Vector3f &) const {
    return 0;
}


std::shared_ptr<PointLight> CreatePointLight(const Transform &light2world,
                                             const Medium *medium,
                                             const ParamSet &paramSet) {
    Color I = paramSet.FindOneSpectrum("I", Color(1.0));
    Color sc = paramSet.FindOneSpectrum("scale", Color(1.0));
    Point3f P = paramSet.FindOnePoint3f("from", Point3f(0, 0, 0));
    Transform l2w = Translate(Vector3f(P.x, P.y, P.z)) * light2world;
    return std::make_shared<PointLight>(l2w, medium, I * sc);
}
