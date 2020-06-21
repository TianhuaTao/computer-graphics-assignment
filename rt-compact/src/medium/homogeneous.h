//
// Created by Sam on 2020/6/19.
//

#ifndef RT_COMPACT_HOMOGENEOUS_H
#define RT_COMPACT_HOMOGENEOUS_H

#include "medium.h"
#include "color.h"
/**
 * 均匀介质
 */
class HomogeneousMedium : public Medium {
public:

    HomogeneousMedium(const Color &sigma_a, const Color &sigma_s, Float g)
            : sigma_a(sigma_a),
              sigma_s(sigma_s),
              sigma_t(sigma_s + sigma_a),
              g(g) {}
    Color Tr(const Ray &ray, Sampler &sampler) const override;
    Color Sample(const Ray &ray, Sampler &sampler, MemoryBank &bank,
                 MediumInteraction *mi) const override;

private:
    const Color sigma_a, sigma_s, sigma_t;
    const Float g;
};


#endif //RT_COMPACT_HOMOGENEOUS_H
