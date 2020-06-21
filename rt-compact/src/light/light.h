//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_LIGHT_H
#define RT_COMPACT_LIGHT_H

#include <math/transform.h>
#include "rt_compact.h"
#include "math/geometry.h"
#include "medium/medium.h"
#include "interaction.h"
// LightFlags Declarations
enum class LightFlags : int {
    DeltaPosition = 1,
    DeltaDirection = 2,
    Area = 4,
    Infinite = 8
};

/**
 * 灯光是否由 Delta 分布描述
 * @param flags
 * @return
 */
inline bool IsDeltaLight(int flags) {
    return flags & (int)LightFlags::DeltaPosition ||
           flags & (int)LightFlags::DeltaDirection;
}

class VisibilityTester;

/**
 * 灯光基类
 */
class Light {
public:
    virtual ~Light();
    Light(int flags, const Transform &LightToWorld,
          const MediumInterface &mediumInterface, int nSamples = 1);
    /**
     * 计算灯对一点的辐射度
     * @param ref 界面
     * @param u 对灯采样的点
     * @param wi 入射方向
     * @param pdf 概率分布
     * @param vis 检验可见性
     * @return 此灯到达该点的 radiance
     */
    virtual Color Sample_Li(const Interaction &ref, const Point2f &u,
                            Vector3f *wi, Float *pdf,
                            VisibilityTester *vis) const = 0;
    /**
     * 灯总功率
     * @return
     */
    virtual Color Power() const = 0;
    virtual void Preprocess(const Scene &scene) {}

    /**
     * 返回灯光对未击中 scene 的光线的贡献，默认为 0，
     * 但在 infinite 灯光中会有值
     * @param r
     * @return
     */
    virtual Color Le(const RayDifferential &r) const;

    /**
     * 光在 wi 方向上的分布密度
     * @param ref
     * @param wi
     * @return
     */
    virtual Float Pdf_Li(const Interaction &ref, const Vector3f &wi) const = 0;



    const int flags;

    const MediumInterface mediumInterface;

protected:

    const Transform LightToWorld, WorldToLight;
};

class VisibilityTester {
public:
    VisibilityTester() {}
    VisibilityTester(const Interaction &p0, const Interaction &p1)
            : p0(p0), p1(p1) {}
    const Interaction &P0() const { return p0; }
    const Interaction &P1() const { return p1; }
    bool Unoccluded(const Scene &scene) const;
    Color Tr(const Scene &scene, Sampler &sampler) const;

private:
    Interaction p0, p1;
};

/**
 * 带有形状的灯光
 */
class AreaLight : public Light {
public:

    AreaLight(const Transform &LightToWorld, const MediumInterface &medium,
              int nSamples);

    virtual Color L(const Interaction &intr, const Vector3f &w) const = 0;
};
#endif //RT_COMPACT_LIGHT_H
