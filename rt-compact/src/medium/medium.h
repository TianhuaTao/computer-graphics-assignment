//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_MEDIUM_H
#define RT_COMPACT_MEDIUM_H

#include "rt_compact.h"
#include "math/geometry.h"
struct MediumInterface;

class PhaseFunction {
public:

    virtual ~PhaseFunction();
    virtual Float p(const Vector3f &wo, const Vector3f &wi) const = 0;
    virtual Float Sample_p(const Vector3f &wo, Vector3f *wi,
                           const Point2f &u) const = 0;

};



bool GetMediumScatteringProperties(const std::string &name, Color *sigma_a,
                                   Color *sigma_s);


inline Float PhaseHG(Float cosTheta, Float g) {
    Float denom = 1 + g * g + 2 * g * cosTheta;
    return Inv4Pi * (1 - g * g) / (denom * std::sqrt(denom));
}


class Medium {
public:

    virtual ~Medium() {}
    virtual Color Tr(const Ray &ray, Sampler &sampler) const = 0;
    virtual Color Sample(const Ray &ray, Sampler &sampler,
                         MemoryBank &arena,
                         MediumInteraction *mi) const = 0;
};


class HenyeyGreenstein : public PhaseFunction {
public:

    HenyeyGreenstein(Float g) : g(g) {}
    Float p(const Vector3f &wo, const Vector3f &wi) const override;
    Float Sample_p(const Vector3f &wo, Vector3f *wi,
                   const Point2f &sample) const override;


private:
    const Float g;
};


struct MediumInterface {
    MediumInterface() : inside(nullptr), outside(nullptr) {}

    MediumInterface(const Medium *medium) : inside(medium), outside(medium) {}
    MediumInterface(const Medium *inside, const Medium *outside)
            : inside(inside), outside(outside) {}
    bool IsMediumTransition() const { return inside != outside; }
    const Medium *inside, *outside;
};

#endif //RT_COMPACT_MEDIUM_H
