//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_REFLCTION_H
#define RT_COMPACT_REFLCTION_H
#include "rt_compact.h"
#include "math/geometry.h"
#include "interaction.h"
#include "math/math.h"

class MicrofacetDistribution;

enum BxDFType {
    BSDF_REFLECTION = 1 << 0,
    BSDF_TRANSMISSION = 1 << 1,
    BSDF_DIFFUSE = 1 << 2,
    BSDF_GLOSSY = 1 << 3,
    BSDF_SPECULAR = 1 << 4,
    BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION |
               BSDF_TRANSMISSION,
};

/**
 * BRDF 和 BSDF 的公共接口
 */
class BxDF {
public:
    virtual ~BxDF() {}
    BxDF(BxDFType type) : type(type) {}
    bool MatchesFlags(BxDFType t) const { return (type & t) == type; }

    /**
     * 根据入射出射方向，给出分布函数的值
     * @param wo
     * @param wi
     * @return
     */
    virtual Color f(const Vector3f &wo, const Vector3f &wi) const = 0;

    /**
     * 根据入射出射方向，给出概率分布
     * @param wo
     * @param wi
     * @return
     */

    virtual Float Pdf(const Vector3f &wo, const Vector3f &wi) const;
    /**
     * 通过采样，可以处理 delta 分布描述的函数，比如镜面反射这种只在一个方向有强度的类型
     * @param wo
     * @param wi
     * @param sample
     * @param pdf
     * @param sampledType
     * @return
     */
    virtual Color Sample_f(const Vector3f &wo, Vector3f *wi,
                           const Point2f &sample, Float *pdf,
                           BxDFType *sampledType = nullptr) const;


    const BxDFType type;
};

/**
 * BSDF 类
 * 存有若干个 BxDF 的指针，计算时依次叠加 BxDF 的结果
 */
class BSDF {
public:
    BSDF(const SurfaceHit &si, Float eta = 1)
            : eta(eta),
              ns(si.shading.n),
              ng(si.n),
              ss(Normalize(si.shading.dpdu)),
              ts(Cross(ns, ss)) {}
    void Add(BxDF *b) {
        assert(nBxDFs< MaxBxDFs);
        bxdfs[nBxDFs++] = b;
    }
    int NumComponents(BxDFType flags = BSDF_ALL) const;
    Vector3f WorldToLocal(const Vector3f &v) const {
        return Vector3f(Dot(v, ss), Dot(v, ts), Dot(v, ns));
    }
    Vector3f LocalToWorld(const Vector3f &v) const {
        return Vector3f(ss.x * v.x + ts.x * v.y + ns.x * v.z,
                        ss.y * v.x + ts.y * v.y + ns.y * v.z,
                        ss.z * v.x + ts.z * v.y + ns.z * v.z);
    }
    Color f(const Vector3f &woW, const Vector3f &wiW,
            BxDFType flags = BSDF_ALL) const;

    Color Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                   Float *pdf, BxDFType type = BSDF_ALL,
                   BxDFType *sampledType = nullptr) const;
    Float Pdf(const Vector3f &wo, const Vector3f &wi,
              BxDFType flags = BSDF_ALL) const;


    const Float eta;

private:

    ~BSDF() {}


    const Normal3f ns, ng;
    const Vector3f ss, ts;
    int nBxDFs = 0;
    static constexpr int MaxBxDFs = 8;
    BxDF *bxdfs[MaxBxDFs];
};
inline int BSDF::NumComponents(BxDFType flags) const {
    int num = 0;
    for (int i = 0; i < nBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(flags)) ++num;
    return num;
}

/**
 * Lambertian 是最简单的反射模型，它向任何一个方向散射光线
 */
class LambertianReflection : public BxDF {
public:
    LambertianReflection(const Color &R)
            : BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), R(R) {}
    Color f(const Vector3f &wo, const Vector3f &wi) const;


private:
    const Color R;
};



/**
 * Oren-Nayar 漫反射，公式见文档
 */
class OrenNayar : public BxDF {
public:
    Color f(const Vector3f &wo, const Vector3f &wi) const;
    OrenNayar(const Color &R, Float sigma)
            : BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), R(R) {
        sigma = Radians(sigma);
        Float sigma2 = sigma * sigma;
        A = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
        B = 0.45f * sigma2 / (sigma2 + 0.09f);
    }

private:
    const Color R;
    Float A, B;
};

/**
 * Fresnel 反射抽象类
 */
class Fresnel {
public:
    virtual ~Fresnel();
    virtual Color Evaluate(Float cosI) const = 0;
};
class FresnelConductor : public Fresnel {
public:
    Color Evaluate(Float cosThetaI) const;
    FresnelConductor(const Color &etaI, const Color &etaT,
                     const Color &k)
            : etaI(etaI), etaT(etaT), k(k) {}

private:
    Color etaI, etaT, k;
};

class FresnelDielectric : public Fresnel {
public:
    // FresnelDielectric Public Methods
    Color Evaluate(Float cosThetaI) const;
    FresnelDielectric(Float etaI, Float etaT) : etaI(etaI), etaT(etaT) {}

private:
    Float etaI, etaT;
};

/**
 * 100% Fresnel 反射
 */
class FresnelNoOp : public Fresnel {
public:
    Color Evaluate(Float) const { return Color(1.); }
};

class MicrofacetReflection : public BxDF {
public:
    MicrofacetReflection(const Color &R,
                         MicrofacetDistribution *distribution, Fresnel *fresnel)
            : BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY)),
              R(R),
              distribution(distribution),
              fresnel(fresnel) {}
    Color f(const Vector3f &wo, const Vector3f &wi) const override;
    Color Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                   Float *pdf, BxDFType *sampledType) const override;
    Float Pdf(const Vector3f &wo, const Vector3f &wi) const override;

private:
    const Color R;
    const MicrofacetDistribution *distribution;
    const Fresnel *fresnel;
};
class FresnelSpecular : public BxDF {
public:
    FresnelSpecular(const Color &R, const Color &T, Float etaA,
                    Float etaB, TransportMode mode)
            : BxDF(BxDFType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_SPECULAR)),
              R(R),
              T(T),
              etaA(etaA),
              etaB(etaB),
              mode(mode) {}
    Color f(const Vector3f &wo, const Vector3f &wi) const {
        return Color(0.f);
    }
    Color Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                   Float *pdf, BxDFType *sampledType) const;
    Float Pdf(const Vector3f &wo, const Vector3f &wi) const { return 0; }

private:
    const Color R, T;
    const Float etaA, etaB;
    const TransportMode mode;
};

/**
 * 镜面反射
 */
class SpecularReflection : public BxDF {
public:
    SpecularReflection(const Color &R, Fresnel *fresnel)
            : BxDF(BxDFType(BSDF_REFLECTION | BSDF_SPECULAR)),
              R(R),
              fresnel(fresnel) {}
    Color f(const Vector3f &wo, const Vector3f &wi) const  override{
        return Color(0.f);
    }
    Color Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                   Float *pdf, BxDFType *sampledType) const override;
    Float Pdf(const Vector3f &wo, const Vector3f &wi) const override { return 0; }

private:
    const Color R;
    const Fresnel *fresnel;
};

/**
 * 镜面透射
 */
class SpecularTransmission : public BxDF {
public:
    SpecularTransmission(const Color &T, Float etaA, Float etaB,
                         TransportMode mode)
            : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)),
              T(T),
              etaA(etaA),
              etaB(etaB),
              fresnel(etaA, etaB),
              mode(mode) {}
    Color f(const Vector3f &wo, const Vector3f &wi) const  override{
        return Color(0.f);
    }
    Color Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                   Float *pdf, BxDFType *sampledType) const override;
    Float Pdf(const Vector3f &wo, const Vector3f &wi) const  override{ return 0; }

private:
    const Color T;
    const Float etaA, etaB;
    const FresnelDielectric fresnel;
    const TransportMode mode;
};


class MicrofacetTransmission : public BxDF {
public:
    MicrofacetTransmission(const Color &T,
                           MicrofacetDistribution *distribution, Float etaA,
                           Float etaB, TransportMode mode)
            : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_GLOSSY)),
              T(T),
              distribution(distribution),
              etaA(etaA),
              etaB(etaB),
              fresnel(etaA, etaB),
              mode(mode) {}
    Color f(const Vector3f &wo, const Vector3f &wi) const override;
    Color Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                   Float *pdf, BxDFType *sampledType) const override;
    Float Pdf(const Vector3f &wo, const Vector3f &wi) const override;

private:
    const Color T;
    const MicrofacetDistribution *distribution;
    const Float etaA, etaB;
    const FresnelDielectric fresnel;
    const TransportMode mode;
};
#endif //RT_COMPACT_REFLCTION_H
