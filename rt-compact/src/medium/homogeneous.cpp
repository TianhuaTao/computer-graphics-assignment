//
// Created by Sam on 2020/6/19.
//

#include "homogeneous.h"
#include "sampler/sampler.h"
#include "color.h"
#include "memory.h"
#include "interaction.h"
Color HomogeneousMedium::Tr(const Ray &ray, Sampler &sampler) const {
    return Exp(-sigma_t * std::min(ray.tMax * ray.d.Length(), MaxFloat));
}

Color HomogeneousMedium::Sample(const Ray &ray, Sampler &sampler,
                                MemoryBank &bank,
                                MediumInteraction *mi) const {
    int channel = std::min((int)(sampler.Get1D() * Color::nSamples),
                           Color::nSamples - 1);
    Float dist = -std::log(1 - sampler.Get1D()) / sigma_t[channel];
    Float t = std::min(dist / ray.d.Length(), ray.tMax);
    bool sampledMedium = t < ray.tMax;
    if (sampledMedium)
        *mi = MediumInteraction(ray(t), -ray.d, ray.time, this,
                                BANK_ALLOC(bank, HenyeyGreenstein)(g));


    Color Tr = Exp(-sigma_t * std::min(t, MaxFloat) * ray.d.Length());


    Color density = sampledMedium ? (sigma_t * Tr) : Tr;
    Float pdf = 0;
    for (int i = 0; i < Color::nSamples; ++i) pdf += density[i];
    pdf *= 1 / (Float)Color::nSamples;
    if (pdf == 0) {
        assert(Tr.IsBlack());
        pdf = 1;
    }
    return sampledMedium ? (Tr * sigma_s / pdf) : (Tr / pdf);
}