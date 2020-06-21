//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_METAL_H
#define RT_COMPACT_METAL_H

#include "rt_compact.h"
#include "material.h"
#include "texture/texture.h"
class MetalMaterial : public Material {
public:

    MetalMaterial(const std::shared_ptr<Texture<Color>> &eta,
                  const std::shared_ptr<Texture<Color>> &k,
                  const std::shared_ptr<Texture<Float>> &roughness,
                  const std::shared_ptr<Texture<Float>> &uRoughness,
                  const std::shared_ptr<Texture<Float>> &vRoughness,
                  bool remapRoughness);
    void ComputeScatteringFunctions(SurfaceHit *si, MemoryBank &bank,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

private:

    std::shared_ptr<Texture<Color>> eta, k;
    std::shared_ptr<Texture<Float>> roughness, uRoughness, vRoughness;

    bool remapRoughness;
};

MetalMaterial *CreateMetalMaterial(const TextureParams &mp);


#endif //RT_COMPACT_METAL_H
