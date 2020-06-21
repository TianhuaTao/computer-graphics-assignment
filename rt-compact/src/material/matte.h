//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_MATTE_H
#define RT_COMPACT_MATTE_H
#include "rt_compact.h"
#include "material.h"
#include "texture/texture.h"
#include "parser/paramset.h"

/**
 * 最简单的不光滑散射材质
 */
class MatteMaterial : public Material {
public:
    MatteMaterial(const std::shared_ptr<Texture<Color>> &Kd,
                  const std::shared_ptr<Texture<Float>> &sigma
                 )
            : Kd(Kd), sigma(sigma)
            {}
    void ComputeScatteringFunctions(SurfaceHit *si, MemoryBank &bank,
                                    TransportMode mode,
                                    bool allowMultipleLobes) const;

private:
    /**
     * 光谱漫反射
     */
    std::shared_ptr<Texture<Color>> Kd;

    /**
     * 粗糙度
     */
    std::shared_ptr<Texture<Float>> sigma;

};

MatteMaterial *CreateMatteMaterial(const TextureParams &mp);



#endif //RT_COMPACT_MATTE_H
