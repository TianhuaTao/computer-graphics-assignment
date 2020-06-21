//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_POINT_H
#define RT_COMPACT_POINT_H
#include "light.h"
#include "interaction.h"

/**
 * 点光源
 */
class PointLight : public Light {
public:

    PointLight(const Transform &LightToWorld,
               const MediumInterface &mediumInterface, const Color &I)
            : Light((int)LightFlags::DeltaPosition, LightToWorld, mediumInterface),
              pLight(LightToWorld(Point3f(0, 0, 0))),
              I(I) {}
    Color Sample_Li(const Interaction &ref, const Point2f &u, Vector3f *wi,
                    Float *pdf, VisibilityTester *vis) const;
    Color Power() const;
    Float Pdf_Li(const Interaction &, const Vector3f &) const;


private:

    const Point3f pLight;
    const Color I;
};

std::shared_ptr<PointLight> CreatePointLight(const Transform &light2world,
                                             const Medium *medium,
                                             const ParamSet &paramSet);



#endif //RT_COMPACT_POINT_H
