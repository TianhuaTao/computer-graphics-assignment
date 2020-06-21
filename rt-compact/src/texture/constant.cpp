//
// Created by Sam on 2020/6/16.
//

#include "constant.h"

ConstantTexture<Float> *CreateConstantFloatTexture(const Transform &tex2world,
                                                   const TextureParams &tp) {
    return new ConstantTexture<Float>(tp.FindFloat("value", 1.f));
}

ConstantTexture<Color> *CreateConstantSpectrumTexture(
        const Transform &tex2world, const TextureParams &tp) {
    return new ConstantTexture<Color>(
            tp.FindSpectrum("value", Color(1.f)));
}
