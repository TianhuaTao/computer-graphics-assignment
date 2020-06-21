//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_SAMPLER_H
#define RT_COMPACT_SAMPLER_H

#include "rt_compact.h"
#include "math/geometry.h"

struct CameraSample;
class Sampler {
public:
    virtual ~Sampler();
    Sampler(int64_t samplesPerPixel);
    virtual void StartPixel(const Point2i &p);
    virtual Float Get1D() {return random_float();}
    virtual Point2f Get2D() {return {random_float(),random_float()};}
    CameraSample GetCameraSample(const Point2i &pRaster);

    virtual bool StartNextSample();
    virtual std::unique_ptr<Sampler> Clone(int seed) = 0;



    const int64_t samplesPerPixel;

protected:

    int64_t currentPixelSampleIndex;
    std::vector<std::vector<Float>> sampleArray1D;
    std::vector<std::vector<Point2f>> sampleArray2D;

private:

};


#endif //RT_COMPACT_SAMPLER_H
