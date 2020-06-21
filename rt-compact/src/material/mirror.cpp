//
// Created by Sam on 2020/6/17.
//

#include "mirror.h"
#include "interaction.h"
#include "reflection/reflection.h"
#include "memory.h"
void MirrorMaterial::ComputeScatteringFunctions(SurfaceHit *si,
                                                MemoryBank &bank,
                                                TransportMode mode,
                                                bool allowMultipleLobes) const {
    si->bsdf = BANK_ALLOC(bank, BSDF)(*si);
    Color R = Kr->Evaluate(*si).Clamp();

    if (!R.IsBlack())
        si->bsdf->Add(BANK_ALLOC(bank, SpecularReflection)(
                R, BANK_ALLOC(bank, FresnelNoOp)()));
}

MirrorMaterial *CreateMirrorMaterial(const TextureParams &mp) {
    std::shared_ptr<Texture<Color>> Kr =
            mp.GetSpectrumTexture("Kr", Color(0.9f));
    return new MirrorMaterial(Kr);
}