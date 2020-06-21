//
// Created by Sam on 2020/6/16.
//

#include "sampler.h"
#include "camera/camera.h"
Sampler::~Sampler() {}

Sampler::Sampler(int64_t samplesPerPixel) : samplesPerPixel(samplesPerPixel) {}
CameraSample Sampler::GetCameraSample(const Point2i &pRaster) {
    CameraSample cs;
    cs.pFilm = (Point2f)pRaster + Get2D();
    cs.time = Get1D();
    cs.pLens = Get2D();
    return cs;
}

void Sampler::StartPixel(const Point2i &p) {
    currentPixelSampleIndex = 0;
}

bool Sampler::StartNextSample() {

    return ++currentPixelSampleIndex < samplesPerPixel;
}

