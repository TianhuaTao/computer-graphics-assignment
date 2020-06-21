//
// Created by Sam on 2020/6/19.
//

#ifndef RT_COMPACT_SPOT_H
#define RT_COMPACT_SPOT_H
#include "light.h"

/**
 * 聚光灯
 */
class SpotLight : public Light {
public:
    SpotLight(const Transform &LightToWorld, const MediumInterface &m,
              const Color &I, Float totalWidth, Float falloffStart);
    Color Sample_Li(const Interaction &ref, const Point2f &u, Vector3f *wi,
                       Float *pdf, VisibilityTester *vis) const override;
    Float Pdf_Li(const Interaction &, const Vector3f &) const override;

    Float Falloff(const Vector3f &w) const;
    Color Power() const override;


private:
    const Point3f pLight;
    const Color I;
    const Float cosTotalWidth, cosFalloffStart;
};

std::shared_ptr<SpotLight> CreateSpotLight(const Transform &l2w,
                                           const Medium *medium,
                                           const ParamSet &paramSet);


#endif //RT_COMPACT_SPOT_H
