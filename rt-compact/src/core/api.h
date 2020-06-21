//
// Created by Sam on 2020/6/15.
//

#ifndef RT_COMPACT_API_H
#define RT_COMPACT_API_H


#include "rt_compact.h"


// API Function Declarations
void init();
void apiCleanup();
void apiIdentity();
void apiTranslate(Float dx, Float dy, Float dz);
void apiRotate(Float angle, Float dx, Float dy, Float dz);
void apiScale(Float sx, Float sy, Float sz);
void apiLookAt(Float ex, Float ey, Float ez, Float lx, Float ly, Float lz,
               Float ux, Float uy, Float uz);
void apiConcatTransform(Float *tr);
void apiTransform(Float *tr);
void apiCoordinateSystem(const std::string &name);
void apiCoordSysTransform(const std::string &name);
void apiActiveTransformAll();
void apiActiveTransformEndTime();
void apiActiveTransformStartTime();
void apiTransformTimes(Float start, Float end);
void apiPixelFilter(const std::string &name, const ParamSet &params);
void apiFilm(const std::string &type, const ParamSet &params);
void apiSampler(const std::string &name, const ParamSet &params);
void apiAccelerator(const std::string &name, const ParamSet &params);
void apiIntegrator(const std::string &name, const ParamSet &params);
void apiCamera(const std::string &name, const ParamSet &params);
void apiMakeNamedMedium(const std::string &name, const ParamSet &params);
void apiMediumInterface(const std::string &insideName,
                        const std::string &outsideName);
void apiWorldBegin();
void apiAttributeBegin();
void apiAttributeEnd();
void apiTransformBegin();
void apiTransformEnd();
void apiTexture(const std::string &name, const std::string &type,
                const std::string &texname, const ParamSet &params);
void apiMaterial(const std::string &name, const ParamSet &params);
void apiMakeNamedMaterial(const std::string &name, const ParamSet &params);
void apiNamedMaterial(const std::string &name);
void apiLightSource(const std::string &name, const ParamSet &params);
void apiAreaLightSource(const std::string &name, const ParamSet &params);
void apiShape(const std::string &name, const ParamSet &params);
void apiReverseOrientation();
void apiObjectBegin(const std::string &name);
void apiObjectEnd();
void apiObjectInstance(const std::string &name);
void apiWorldEnd();
void parseFile(std::string filename);




#endif //RT_COMPACT_API_H
