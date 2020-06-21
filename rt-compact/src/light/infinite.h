//
// Created by Sam on 2020/6/17.
//

#ifndef RT_COMPACT_INFINITE_H
#define RT_COMPACT_INFINITE_H

#include "mipmap.h"
#include "light.h"
#include "prob.h"
#include "scene.h"

/**
 * 无穷远处的背景灯光，可以使用图片作为背景
 * 需要使用 MIPMAP
 */
class InfiniteAreaLight : public Light {

public:

    InfiniteAreaLight(const Transform &LightToWorld, const Color &power,
                      int nSamples, const std::string &texmap);
    void Preprocess(const Scene &scene) override{
        scene.WorldBound().BoundingSphere(&worldCenter, &worldRadius);
    }
    Color Power() const override;
    Color Le(const RayDifferential &ray) const override;
    Color Sample_Li(const Interaction &ref, const Point2f &u, Vector3f *wi,
                    Float *pdf, VisibilityTester *vis) const override;
    Float Pdf_Li(const Interaction &, const Vector3f &) const override;


private:
    std::unique_ptr<MIPMap<Color>> Lmap;
    Point3f worldCenter;
    Float worldRadius;
    Color color;
    std::unique_ptr<Distribution2D> distribution;
};

std::shared_ptr<InfiniteAreaLight> CreateInfiniteLight(
        const Transform &light2world, const ParamSet &paramSet);


#endif //RT_COMPACT_INFINITE_H
