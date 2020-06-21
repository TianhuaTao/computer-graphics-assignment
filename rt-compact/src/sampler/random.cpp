//
// Created by Sam on 2020/6/17.
//

#include "random.h"
#include <util/randgen.h>
#include <parser/paramset.h>
RandomSampler::RandomSampler(int ns, int seed) : Sampler(ns) {}

Float RandomSampler::Get1D() {
    return random_float();
}

Point2f RandomSampler::Get2D() {
    return {random_float(),random_float()};
}

std::unique_ptr<Sampler> RandomSampler::Clone(int seed) {
    return std::unique_ptr<Sampler>(new RandomSampler(*this));
}

void RandomSampler::StartPixel(const Point2i &p) {

    for (size_t i = 0; i < sampleArray1D.size(); ++i)
        for (size_t j = 0; j < sampleArray1D[i].size(); ++j)
            sampleArray1D[i][j] = random_float();

    for (size_t i = 0; i < sampleArray2D.size(); ++i)
        for (size_t j = 0; j < sampleArray2D[i].size(); ++j)
            sampleArray2D[i][j] = {random_float(), random_float()};
    Sampler::StartPixel(p);
}

Sampler *CreateRandomSampler(const ParamSet &params) {
    int ns = params.FindOneInt("pixelsamples", 4);
    return new RandomSampler(ns);
}