//
// Created by Sam on 2020/6/17.
//

#ifndef RT_COMPACT_RANDOM_H
#define RT_COMPACT_RANDOM_H


#include "sampler.h"

/**
 * 完全使用随机的采样
 */
class RandomSampler : public Sampler {
public:
    RandomSampler(int ns, int seed = 0);
    void StartPixel(const Point2i &);
    Float Get1D();
    Point2f Get2D();
    std::unique_ptr<Sampler> Clone(int seed);

private:

};

Sampler *CreateRandomSampler(const ParamSet &params);


#endif //RT_COMPACT_RANDOM_H
