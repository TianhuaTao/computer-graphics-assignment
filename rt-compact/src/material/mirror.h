//
// Created by Sam on 2020/6/17.
//

#ifndef RT_COMPACT_MIRROR_H
#define RT_COMPACT_MIRROR_H
#include "material.h"
#include "texture/texture.h"
class MirrorMaterial : public Material {
public:
    MirrorMaterial(const std::shared_ptr<Texture<Color>> &r
                   ) {
        Kr = r;
    }
    void ComputeScatteringFunctions(SurfaceHit *si, MemoryBank &bank,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

private:

    std::shared_ptr<Texture<Color>> Kr;
};

MirrorMaterial *CreateMirrorMaterial(const TextureParams &mp);



#endif //RT_COMPACT_MIRROR_H
