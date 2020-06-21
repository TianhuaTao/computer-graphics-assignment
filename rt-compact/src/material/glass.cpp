//
// Created by Sam on 2020/6/16.
//

#include "glass.h"
#include "color.h"
#include "reflection/reflection.h"
#include "memory.h"
#include "microfacet.h"
class BSDF;
void GlassMaterial::ComputeScatteringFunctions(SurfaceHit *si,
                                               MemoryBank &bank,
                                               TransportMode mode,
                                               bool allowMultipleLobes) const {

    Float eta = index->Evaluate(*si);
    Float urough = uRoughness->Evaluate(*si);
    Float vrough = vRoughness->Evaluate(*si);
    Color R = Kr->Evaluate(*si).Clamp();
    Color T = Kt->Evaluate(*si).Clamp();

    si->bsdf = BANK_ALLOC(bank, BSDF)(*si, eta);

    if (R.IsBlack() && T.IsBlack()) return;

    bool isSpecular = urough == 0 && vrough == 0;
    if (isSpecular && allowMultipleLobes) {
        si->bsdf->Add(
                BANK_ALLOC(bank, FresnelSpecular)(R, T, 1.f, eta, mode));
    } else {
        if (remapRoughness) {
            urough = TrowbridgeReitzDistribution::RoughnessToAlpha(urough);
            vrough = TrowbridgeReitzDistribution::RoughnessToAlpha(vrough);
        }
        MicrofacetDistribution *distrib =
                isSpecular ? nullptr
                           : BANK_ALLOC(bank, TrowbridgeReitzDistribution)(
                        urough, vrough);
        if (!R.IsBlack()) {
            Fresnel *fresnel = BANK_ALLOC(bank, FresnelDielectric)(1.f, eta);
            if (isSpecular)
                si->bsdf->Add(
                        BANK_ALLOC(bank, SpecularReflection)(R, fresnel));
            else
                si->bsdf->Add(BANK_ALLOC(bank, MicrofacetReflection)(
                        R, distrib, fresnel));
        }
        if (!T.IsBlack()) {
            if (isSpecular)
                si->bsdf->Add(BANK_ALLOC(bank, SpecularTransmission)(
                        T, 1.f, eta, mode));
            else
                si->bsdf->Add(BANK_ALLOC(bank, MicrofacetTransmission)(
                        T, distrib, 1.f, eta, mode));
        }
    }
}

GlassMaterial *CreateGlassMaterial(const TextureParams &mp) {
    std::shared_ptr<Texture<Color>> Kr =
            mp.GetSpectrumTexture("Kr", Color(1.f));
    std::shared_ptr<Texture<Color>> Kt =
            mp.GetSpectrumTexture("Kt", Color(1.f));
    std::shared_ptr<Texture<Float>> eta = mp.GetFloatTextureOrNull("eta");
    if (!eta) eta = mp.GetFloatTexture("index", 1.5f);
    std::shared_ptr<Texture<Float>> roughu =
            mp.GetFloatTexture("uroughness", 0.f);
    std::shared_ptr<Texture<Float>> roughv =
            mp.GetFloatTexture("vroughness", 0.f);
    bool remapRoughness = mp.FindBool("remaproughness", true);
    return new GlassMaterial(Kr, Kt, roughu, roughv, eta,
                             remapRoughness);
}
