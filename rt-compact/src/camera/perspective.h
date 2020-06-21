//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_PERSPECTIVE_H
#define RT_COMPACT_PERSPECTIVE_H

#include "camera.h"

#include "light/light.h"

class PerspectiveCamera : public ProjectiveCamera {
public:
    PerspectiveCamera(const AnimatedTransform &CameraToWorld,
                      const Bounds2f &screenWindow, Float shutterOpen,
                      Float shutterClose, Float lensRadius, Float focalDistance,
                      Float fov, Film *film, const Medium *medium);
    Float GenerateRay(const CameraSample &sample, Ray *) const;
    Float GenerateRayDifferential(const CameraSample &sample,
                                  RayDifferential *ray) const;



private:
    Vector3f dxCamera, dyCamera;
    Float A;
};

PerspectiveCamera *CreatePerspectiveCamera(const ParamSet &params,
                                           const AnimatedTransform &cam2world,
                                           Film *film, const Medium *medium);



#endif //RT_COMPACT_PERSPECTIVE_H
