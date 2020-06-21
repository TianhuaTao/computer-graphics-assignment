//
// Created by Sam on 2020/6/16.
//

#include "camera.h"


Camera::~Camera() { delete film; }

Camera::Camera(const AnimatedTransform &CameraToWorld, Float shutterOpen,
               Float shutterClose, Film *film, const Medium *medium)
        : CameraToWorld(CameraToWorld),
          shutterOpen(shutterOpen),
          shutterClose(shutterClose),
          film(film),
          medium(medium) {
    if (CameraToWorld.HasScale())
        Warning("Scaling detected in world-to-camera transformation");
}

Float Camera::GenerateRayDifferential(const CameraSample &sample,
                                      RayDifferential *rd) const {
    Float wt = GenerateRay(sample, rd);
    if (wt == 0) return 0;

    // 偏移x
    Float wtx;
    for (Float eps : { .05, -.05 }) {
        CameraSample sshift = sample;
        sshift.pFilm.x += eps;
        Ray rx;
        wtx = GenerateRay(sshift, &rx);
        rd->rxOrigin = rd->o + (rx.o - rd->o) / eps;
        rd->rxDirection = rd->d + (rx.d - rd->d) / eps;
        if (wtx != 0)
            break;
    }
    if (wtx == 0)
        return 0;

    // 偏移y
    Float wty;
    for (Float eps : { .05, -.05 }) {
        CameraSample sshift = sample;
        sshift.pFilm.y += eps;
        Ray ry;
        wty = GenerateRay(sshift, &ry);
        rd->ryOrigin = rd->o + (ry.o - rd->o) / eps;
        rd->ryDirection = rd->d + (ry.d - rd->d) / eps;
        if (wty != 0)
            break;
    }
    if (wty == 0)
        return 0;

    rd->hasDifferentials = true;
    return wt;
}

