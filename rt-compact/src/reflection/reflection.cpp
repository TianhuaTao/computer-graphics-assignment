//
// Created by Sam on 2020/6/16.
//

#include "reflection.h"
#include "prob.h"
#include "microfacet.h"

Float FrDielectric(Float cosThetaI, Float etaI, Float etaT) {
    cosThetaI = Clamp(cosThetaI, -1, 1);
    // Potentially swap indices of refraction
    bool entering = cosThetaI > 0.f;
    if (!entering) {
        std::swap(etaI, etaT);
        cosThetaI = std::abs(cosThetaI);
    }

    // Compute _cosThetaT_ using Snell's law
    Float sinThetaI = std::sqrt(std::max((Float)0, 1 - cosThetaI * cosThetaI));
    Float sinThetaT = etaI / etaT * sinThetaI;

    // Handle total internal reflection
    if (sinThetaT >= 1) return 1;
    Float cosThetaT = std::sqrt(std::max((Float)0, 1 - sinThetaT * sinThetaT));
    Float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
                  ((etaT * cosThetaI) + (etaI * cosThetaT));
    Float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
                  ((etaI * cosThetaI) + (etaT * cosThetaT));
    return (Rparl * Rparl + Rperp * Rperp) / 2;
}

// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
Color FrConductor(Float cosThetaI, const Color &etai,
                  const Color &etat, const Color &k) {
    cosThetaI = Clamp(cosThetaI, -1, 1);
    Color eta = etat / etai;
    Color etak = k / etai;

    Float cosThetaI2 = cosThetaI * cosThetaI;
    Float sinThetaI2 = 1. - cosThetaI2;
    Color eta2 = eta * eta;
    Color etak2 = etak * etak;

    Color t0 = eta2 - etak2 - sinThetaI2;
    Color a2plusb2 = Sqrt(t0 * t0 + 4 * eta2 * etak2);
    Color t1 = a2plusb2 + cosThetaI2;
    Color a = Sqrt(0.5f * (a2plusb2 + t0));
    Color t2 = (Float)2 * cosThetaI * a;
    Color Rs = (t1 - t2) / (t1 + t2);

    Color t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
    Color t4 = t2 * sinThetaI2;
    Color Rp = Rs * (t3 - t4) / (t3 + t4);

    return 0.5 * (Rp + Rs);
}
// BSDF Method Definitions
Color BSDF::f(const Vector3f &woW, const Vector3f &wiW,
              BxDFType flags) const {

    Vector3f wi = WorldToLocal(wiW), wo = WorldToLocal(woW);
    if (wo.z == 0) return 0.;
    bool reflect = Dot(wiW, ng) * Dot(woW, ng) > 0;
    Color f(0.f);
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(flags) &&
            ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
             (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
            f += bxdfs[i]->f(wo, wi);
    return f;
}



Color BSDF::Sample_f(const Vector3f &woWorld, Vector3f *wiWorld,
                     const Point2f &u, Float *pdf, BxDFType type,
                     BxDFType *sampledType) const {


    // 选择采样 BxDF
    int matchingComps = NumComponents(type);
    if (matchingComps == 0) {
        *pdf = 0;
        if (sampledType) *sampledType = BxDFType(0);
        return Color(0);
    }
    int comp =
            std::min((int)std::floor(u[0] * matchingComps), matchingComps - 1);

    BxDF *bxdf = nullptr;
    int count = comp;
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(type) && count-- == 0) {
            bxdf = bxdfs[i];
            break;
        }

    Point2f uRemapped(std::min(u[0] * matchingComps - comp, Float(1.f-1e-7)),
                      u[1]);

    // 对选中 BxDF 采样
    Vector3f wi, wo = WorldToLocal(woWorld);
    if (wo.z == 0) return 0.;
    *pdf = 0;
    if (sampledType) *sampledType = bxdf->type;
    Color f = bxdf->Sample_f(wo, &wi, uRemapped, pdf, sampledType);

    if (*pdf == 0) {
        if (sampledType) *sampledType = BxDFType(0);
        return 0;
    }
    *wiWorld = LocalToWorld(wi);

    // 计算总体 PDF
    if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1)
        for (int i = 0; i < nBxDFs; ++i)
            if (bxdfs[i] != bxdf && bxdfs[i]->MatchesFlags(type))
                *pdf += bxdfs[i]->Pdf(wo, wi);
    if (matchingComps > 1) *pdf /= matchingComps;

    // 计算总体 BSDF
    if (!(bxdf->type & BSDF_SPECULAR)) {
        bool reflect = Dot(*wiWorld, ng) * Dot(woWorld, ng) > 0;
        f = 0.;
        for (int i = 0; i < nBxDFs; ++i)
            if (bxdfs[i]->MatchesFlags(type) &&
                ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
                 (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
                f += bxdfs[i]->f(wo, wi);
    }

    return f;
}

Float BSDF::Pdf(const Vector3f &woWorld, const Vector3f &wiWorld,
                BxDFType flags) const {

    if (nBxDFs == 0.f) return 0.f;
    Vector3f wo = WorldToLocal(woWorld), wi = WorldToLocal(wiWorld);
    if (wo.z == 0) return 0.;
    Float pdf = 0.f;
    int matchingComps = 0;
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(flags)) {
            ++matchingComps;
            pdf += bxdfs[i]->Pdf(wo, wi);
        }
    Float v = matchingComps > 0 ? pdf / matchingComps : 0.f;
    return v;
}


Color LambertianReflection::f(const Vector3f &wo, const Vector3f &wi) const {
    return R * InvPi;
}



Color OrenNayar::f(const Vector3f &wo, const Vector3f &wi) const {
    Float sinThetaI = SinTheta(wi);
    Float sinThetaO = SinTheta(wo);
    // Compute cosine term of Oren-Nayar model
    Float maxCos = 0;
    if (sinThetaI > 1e-4 && sinThetaO > 1e-4) {
        Float sinPhiI = SinPhi(wi), cosPhiI = CosPhi(wi);
        Float sinPhiO = SinPhi(wo), cosPhiO = CosPhi(wo);
        Float dCos = cosPhiI * cosPhiO + sinPhiI * sinPhiO;
        maxCos = std::max((Float)0, dCos);
    }

    // Compute sine and tangent terms of Oren-Nayar model
    Float sinAlpha, tanBeta;
    if (AbsCosTheta(wi) > AbsCosTheta(wo)) {
        sinAlpha = sinThetaO;
        tanBeta = sinThetaI / AbsCosTheta(wi);
    } else {
        sinAlpha = sinThetaI;
        tanBeta = sinThetaO / AbsCosTheta(wo);
    }
    return R * InvPi * (A + B * maxCos * sinAlpha * tanBeta);
}



Color BxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                     Float *pdf, BxDFType *sampledType) const {
    // Cosine-sample the hemisphere, flipping the direction if necessary
    *wi = CosineSampleHemisphere(u);
    if (wo.z < 0) wi->z *= -1;
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

Float BxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const {
    return SameHemisphere(wo, wi) ? AbsCosTheta(wi) * InvPi : 0;
}


Color SpecularTransmission::Sample_f(const Vector3f &wo, Vector3f *wi,
                                     const Point2f &sample, Float *pdf,
                                     BxDFType *sampledType) const {
    // Figure out which $\eta$ is incident and which is transmitted
    bool entering = CosTheta(wo) > 0;
    Float etaI = entering ? etaA : etaB;
    Float etaT = entering ? etaB : etaA;

    // Compute ray direction for specular transmission
    if (!Refract(wo, Faceforward(Normal3f(0, 0, 1), wo), etaI / etaT, wi))
        return 0;
    *pdf = 1;
    Color ft = T * (Color(1.) - fresnel.Evaluate(CosTheta(*wi)));
    // Account for non-symmetry with transmission to different medium
    if (mode == TransportMode::Radiance) ft *= (etaI * etaI) / (etaT * etaT);
    return ft / AbsCosTheta(*wi);
}

Color MicrofacetTransmission::f(const Vector3f &wo,
                                const Vector3f &wi) const {
    if (SameHemisphere(wo, wi)) return 0;  // transmission only

    Float cosThetaO = CosTheta(wo);
    Float cosThetaI = CosTheta(wi);
    if (cosThetaI == 0 || cosThetaO == 0) return Color(0);

    Float eta = CosTheta(wo) > 0 ? (etaB / etaA) : (etaA / etaB);
    Vector3f wh = Normalize(wo + wi * eta);
    if (wh.z < 0) wh = -wh;

    Color F = fresnel.Evaluate(Dot(wo, wh));

    Float sqrtDenom = Dot(wo, wh) + eta * Dot(wi, wh);
    Float factor = (mode == TransportMode::Radiance) ? (1 / eta) : 1;

    return (Color(1.f) - F) * T *
           std::abs(distribution->D(wh) * distribution->G(wo, wi) * eta * eta *
                    AbsDot(wi, wh) * AbsDot(wo, wh) * factor * factor /
                    (cosThetaI * cosThetaO * sqrtDenom * sqrtDenom));
}




Color MicrofacetTransmission::Sample_f(const Vector3f &wo, Vector3f *wi,
                                       const Point2f &u, Float *pdf,
                                       BxDFType *sampledType) const {
    if (wo.z == 0) return 0.;
    Vector3f wh = distribution->Sample_wh(wo, u);
    if (Dot(wo, wh) < 0) return 0.;  // Should be rare

    Float eta = CosTheta(wo) > 0 ? (etaA / etaB) : (etaB / etaA);
    if (!Refract(wo, (Normal3f)wh, eta, wi)) return 0;
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

Float MicrofacetTransmission::Pdf(const Vector3f &wo,
                                  const Vector3f &wi) const {
    if (SameHemisphere(wo, wi)) return 0;
    Float eta = CosTheta(wo) > 0 ? (etaB / etaA) : (etaA / etaB);
    Vector3f wh = Normalize(wo + wi * eta);

    Float sqrtDenom = Dot(wo, wh) + eta * Dot(wi, wh);
    Float dwh_dwi =
            std::abs((eta * eta * Dot(wi, wh)) / (sqrtDenom * sqrtDenom));
    return distribution->Pdf(wo, wh) * dwh_dwi;
}

Color FresnelDielectric::Evaluate(Float cosThetaI) const {
    return FrDielectric(cosThetaI, etaI, etaT);
}


Fresnel::~Fresnel() {}
Color FresnelConductor::Evaluate(Float cosThetaI) const {
    return FrConductor(std::abs(cosThetaI), etaI, etaT, k);
}


Color FresnelSpecular::Sample_f(const Vector3f &wo, Vector3f *wi,
                                const Point2f &u, Float *pdf,
                                BxDFType *sampledType) const {
    Float F = FrDielectric(CosTheta(wo), etaA, etaB);
    if (u[0] < F) {
        // Compute specular reflection for _FresnelSpecular_

        // Compute perfect specular reflection direction
        *wi = Vector3f(-wo.x, -wo.y, wo.z);
        if (sampledType)
            *sampledType = BxDFType(BSDF_SPECULAR | BSDF_REFLECTION);
        *pdf = F;
        return F * R / AbsCosTheta(*wi);
    } else {
        // Compute specular transmission for _FresnelSpecular_

        // Figure out which $\eta$ is incident and which is transmitted
        bool entering = CosTheta(wo) > 0;
        Float etaI = entering ? etaA : etaB;
        Float etaT = entering ? etaB : etaA;

        // Compute ray direction for specular transmission
        if (!Refract(wo, Faceforward(Normal3f(0, 0, 1), wo), etaI / etaT, wi))
            return 0;
        Color ft = T * (1 - F);

        // Account for non-symmetry with transmission to different medium
        if (mode == TransportMode::Radiance)
            ft *= (etaI * etaI) / (etaT * etaT);
        if (sampledType)
            *sampledType = BxDFType(BSDF_SPECULAR | BSDF_TRANSMISSION);
        *pdf = 1 - F;
        return ft / AbsCosTheta(*wi);
    }
}


Color MicrofacetReflection::f(const Vector3f &wo, const Vector3f &wi) const {
    Float cosThetaO = AbsCosTheta(wo), cosThetaI = AbsCosTheta(wi);
    Vector3f wh = wi + wo;
    // Handle degenerate cases for microfacet reflection
    if (cosThetaI == 0 || cosThetaO == 0) return Color(0.);
    if (wh.x == 0 && wh.y == 0 && wh.z == 0) return Color(0.);
    wh = Normalize(wh);
    // For the Fresnel call, make sure that wh is in the same hemisphere
    // as the surface normal, so that TIR is handled correctly.
    Color F = fresnel->Evaluate(Dot(wi, Faceforward(wh, Vector3f(0, 0, 1))));
    return R * distribution->D(wh) * distribution->G(wo, wi) * F /
           (4 * cosThetaI * cosThetaO);
}

Color SpecularReflection::Sample_f(const Vector3f &wo, Vector3f *wi,
                                   const Point2f &sample, Float *pdf,
                                   BxDFType *sampledType) const {
    // 完美镜面反射
    *wi = Vector3f(-wo.x, -wo.y, wo.z);
    *pdf = 1;
    return fresnel->Evaluate(CosTheta(*wi)) * R / AbsCosTheta(*wi);
}



Color MicrofacetReflection::Sample_f(const Vector3f &wo, Vector3f *wi,
                                     const Point2f &u, Float *pdf,
                                     BxDFType *sampledType) const {
    // Sample microfacet orientation $\wh$ and reflected direction $\wi$
    if (wo.z == 0) return 0.;
    Vector3f wh = distribution->Sample_wh(wo, u);
    if (Dot(wo, wh) < 0) return 0.;   // Should be rare
    *wi = Reflect(wo, wh);
    if (!SameHemisphere(wo, *wi)) return Color(0.f);

    // Compute PDF of _wi_ for microfacet reflection
    *pdf = distribution->Pdf(wo, wh) / (4 * Dot(wo, wh));
    return f(wo, *wi);
}

Float MicrofacetReflection::Pdf(const Vector3f &wo, const Vector3f &wi) const {
    if (!SameHemisphere(wo, wi)) return 0;
    Vector3f wh = Normalize(wo + wi);
    return distribution->Pdf(wo, wh) / (4 * Dot(wo, wh));
}
