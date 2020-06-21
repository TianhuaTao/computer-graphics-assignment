//
// Created by Sam on 2020/6/19.
//

#ifndef RT_COMPACT_PLYMESH_H
#define RT_COMPACT_PLYMESH_H
#include "shape.h"
#include "texture/texture.h"

/**
 * 基于 triangle mesh 的 ply 拓展，使用 rply 库
 * @param o2w
 * @param w2o
 * @param reverseOrientation
 * @param params
 * @param floatTextures
 * @return
 */
std::vector<std::shared_ptr<Shape>> CreatePLYMesh(
        const Transform *o2w, const Transform *w2o, bool reverseOrientation,
        const ParamSet &params,
        std::map<std::string, std::shared_ptr<Texture<Float>>> *floatTextures =
nullptr);


#endif //RT_COMPACT_PLYMESH_H
