//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_DISTANT_H
#define RT_COMPACT_DISTANT_H

#include "light.h"
#include "scene.h"
/**
 * 遥远的平行光，比如太阳
 */
class DistantLight : public Light {
public:
    DistantLight(const Transform &LightToWorld, const Color &L,
                 const Vector3f &w);
    void Preprocess(const Scene &scene) override {
        scene.WorldBound().BoundingSphere(&worldCenter, &worldRadius);
    }
    Color Sample_Li(const Interaction &ref, const Point2f &u, Vector3f *wi,
                    Float *pdf, VisibilityTester *vis) const override;
    Color Power() const override;
    Float Pdf_Li(const Interaction &, const Vector3f &) const override;


private:
    const Color L;
    const Vector3f wLight;
    Point3f worldCenter;
    Float worldRadius;
};

std::shared_ptr<DistantLight> CreateDistantLight(const Transform &light2world,
                                                 const ParamSet &paramSet);

#endif //RT_COMPACT_DISTANT_H
