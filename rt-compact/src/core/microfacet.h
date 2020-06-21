//
// Created by Sam on 2020/6/17.
//

#ifndef RT_COMPACT_MICROFACET_H
#define RT_COMPACT_MICROFACET_H

#include "math/geometry.h"

/**
 * 微平面模型
 */
class MicrofacetDistribution {
public:
    virtual ~MicrofacetDistribution();
    virtual Float D(const Vector3f &wh) const = 0;
    virtual Float Lambda(const Vector3f &w) const = 0;
    Float G1(const Vector3f &w) const {
        return 1 / (1 + Lambda(w));
    }
    virtual Float G(const Vector3f &wo, const Vector3f &wi) const {
        return 1 / (1 + Lambda(wo) + Lambda(wi));
    }
    virtual Vector3f Sample_wh(const Vector3f &wo, const Point2f &u) const = 0;
    Float Pdf(const Vector3f &wo, const Vector3f &wh) const;


protected:
    MicrofacetDistribution(bool sampleVisibleArea)
            : sampleVisibleArea(sampleVisibleArea) {}

    const bool sampleVisibleArea;
};



/**
 * Trowbridge-Reitz 分布
 */
class TrowbridgeReitzDistribution : public MicrofacetDistribution {
public:
    static inline Float RoughnessToAlpha(Float roughness);
    TrowbridgeReitzDistribution(Float alphax, Float alphay,
                                bool samplevis = true)
            : MicrofacetDistribution(samplevis), alphax(alphax), alphay(alphay) {}
    Float D(const Vector3f &wh) const;
    Vector3f Sample_wh(const Vector3f &wo, const Point2f &u) const;


private:
    Float Lambda(const Vector3f &w) const;
    const Float alphax, alphay;
};

inline Float TrowbridgeReitzDistribution::RoughnessToAlpha(Float roughness) {
    roughness = std::max(roughness, (Float)1e-3);
    Float x = std::log(roughness);
    return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x +
           0.000640711f * x * x * x * x;
}

#endif //RT_COMPACT_MICROFACET_H
