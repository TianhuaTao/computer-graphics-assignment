//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_CONSTANT_H
#define RT_COMPACT_CONSTANT_H

#include "rt_compact.h"
#include "texture.h"
#include "parser/paramset.h"
/**
 * 常纹理，是 Color 或者 Float 的包装类
 */
template <typename T>
class ConstantTexture : public Texture<T> {
public:
    ConstantTexture(const T &value) : value(value) {}
    T Evaluate(const SurfaceHit &) const { return value; }

private:
    T value;
};

ConstantTexture<Float> *CreateConstantFloatTexture(const Transform &tex2world,
                                                   const TextureParams &tp);
ConstantTexture<Color> *CreateConstantSpectrumTexture(
        const Transform &tex2world, const TextureParams &tp);


#endif //RT_COMPACT_CONSTANT_H
