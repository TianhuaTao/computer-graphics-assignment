//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_CAMERA_H
#define RT_COMPACT_CAMERA_H

#include "rt_compact.h"
#include "math/transform.h"
#include "math/geometry.h"
#include "film.h"

class CameraSample;
class VisibilityTester;
class Camera {
public:

    Camera(const AnimatedTransform &CameraToWorld, Float shutterOpen,
           Float shutterClose, Film *film, const Medium *medium);
    virtual ~Camera();
    virtual Float GenerateRay(const CameraSample &sample, Ray *ray) const = 0;
    virtual Float GenerateRayDifferential(const CameraSample &sample,
                                          RayDifferential *rd) const;


    AnimatedTransform CameraToWorld;
    const Float shutterOpen, shutterClose;
    Film *film;
    const Medium *medium;
};

struct CameraSample {
    Point2f pFilm;
    Point2f pLens;
    Float time;
};


class ProjectiveCamera : public Camera {
public:

    ProjectiveCamera(const AnimatedTransform &CameraToWorld,
                     const Transform &CameraToScreen,
                     const Bounds2f &screenWindow, Float shutterOpen,
                     Float shutterClose, Float lensr, Float focald, Film *film,
                     const Medium *medium)
            : Camera(CameraToWorld, shutterOpen, shutterClose, film, medium),
              CameraToScreen(CameraToScreen) {

        lensRadius = lensr;
        focalDistance = focald;


        ScreenToRaster =
                Scale(film->fullResolution.x, film->fullResolution.y, 1) *
                Scale(1 / (screenWindow.pMax.x - screenWindow.pMin.x),
                      1 / (screenWindow.pMin.y - screenWindow.pMax.y), 1) *
                Translate(Vector3f(-screenWindow.pMin.x, -screenWindow.pMax.y, 0));
        RasterToScreen = Inverse(ScreenToRaster);
        RasterToCamera = Inverse(CameraToScreen) * RasterToScreen;
    }

protected:

    Transform CameraToScreen, RasterToCamera;
    Transform ScreenToRaster, RasterToScreen;
    Float lensRadius, focalDistance;
};



#endif //RT_COMPACT_CAMERA_H
