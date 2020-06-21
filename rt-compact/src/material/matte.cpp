//
// Created by Sam on 2020/6/16.
//

#include "matte.h"
#include "interaction.h"
#include "reflection/reflection.h"
#include "memory.h"
// MatteMaterial Method Definitions
void MatteMaterial::ComputeScatteringFunctions(SurfaceHit *si,
                                               MemoryBank &bank,
                                               TransportMode mode,
                                               bool allowMultipleLobes) const {

    /* 设置 BRDF */
    si->bsdf = BANK_ALLOC(bank, BSDF)(*si);

    /** 计算 Kd 和 sigma 纹理 */
    Color r = Kd->Evaluate(*si).Clamp();
    Float sig = Clamp(sigma->Evaluate(*si), 0, 90);

    /**
     * 根据粗糙度sig，使用 LambertianReflection 或者 OrenNayar 模型
     */
    if (!r.IsBlack()) {
        if (sig == 0)
            si->bsdf->Add(BANK_ALLOC(bank, LambertianReflection)(r));
        else
            si->bsdf->Add(BANK_ALLOC(bank, OrenNayar)(r, sig));
    }
}

MatteMaterial *CreateMatteMaterial(const TextureParams &mp) {
    std::shared_ptr<Texture<Color>> Kd =
            mp.GetSpectrumTexture("Kd", Color(0.5f));
    std::shared_ptr<Texture<Float>> sigma = mp.GetFloatTexture("sigma", 0.f);

    return new MatteMaterial(Kd, sigma);
}