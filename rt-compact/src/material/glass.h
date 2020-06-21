//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_GLASS_H
#define RT_COMPACT_GLASS_H


#include <parser/paramset.h>
#include "material.h"
#include "texture/texture.h"
class GlassMaterial : public Material {
public:
    GlassMaterial(const std::shared_ptr<Texture<Color>> &Kr,
    const std::shared_ptr<Texture<Color>> &Kt,
    const std::shared_ptr<Texture<Float>> &uRoughness,
    const std::shared_ptr<Texture<Float>> &vRoughness,
    const std::shared_ptr<Texture<Float>> &index,
    bool remapRoughness)
    : Kr(Kr),
    Kt(Kt),
    uRoughness(uRoughness),
    vRoughness(vRoughness),
    index(index),
    remapRoughness(remapRoughness) {}
    void ComputeScatteringFunctions(SurfaceHit *si, MemoryBank &bank,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

private:

    std::shared_ptr<Texture<Color>> Kr, Kt;
    std::shared_ptr<Texture<Float>> uRoughness, vRoughness;
    std::shared_ptr<Texture<Float>> index;

    bool remapRoughness;
};

GlassMaterial *CreateGlassMaterial(const TextureParams &mp);



#endif //RT_COMPACT_GLASS_H
