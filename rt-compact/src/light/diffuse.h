//
// Created by Sam on 2020/6/17.
//

#ifndef RT_COMPACT_DIFFUSE_H
#define RT_COMPACT_DIFFUSE_H

#include "light.h"
class DiffuseAreaLight : public AreaLight {
public:

    DiffuseAreaLight(const Transform &LightToWorld,
                     const MediumInterface &mediumInterface, const Color &Le,
                     int nSamples, const std::shared_ptr<Shape> &shape,
                     bool twoSided = false);
    Color L(const Interaction &intr, const Vector3f &w) const {
        return (twoSided || Dot(intr.n, w) > 0) ? Lemit : Color(0.f);
    }
    Color Power() const;
    Color Sample_Li(const Interaction &ref, const Point2f &u, Vector3f *wo,
                    Float *pdf, VisibilityTester *vis) const;
    Float Pdf_Li(const Interaction &, const Vector3f &) const;


protected:

    const Color Lemit;
    std::shared_ptr<Shape> shape;

    const bool twoSided;
    const Float area;
};

std::shared_ptr<AreaLight> CreateDiffuseAreaLight(
        const Transform &light2world, const Medium *medium,
        const ParamSet &paramSet, const std::shared_ptr<Shape> &shape);



#endif //RT_COMPACT_DIFFUSE_H
