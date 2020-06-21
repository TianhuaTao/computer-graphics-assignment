//
// Created by Sam on 2020/6/15.
//

#ifndef RT_COMPACT_COLOR_H
#define RT_COMPACT_COLOR_H

#include "rt_compact.h"
enum class SpectrumType { Reflectance, Illuminant };


// Color Utility Declarations

extern bool SpectrumSamplesSorted(const Float *lambda, const Float *vals,
                                  int n);
extern void SortSpectrumSamples(Float *lambda, Float *vals, int n);
extern Float AverageSpectrumSamples(const Float *lambda, const Float *vals,
                                    int n, Float lambdaStart, Float lambdaEnd);
inline void XYZToRGB(const Float xyz[3], Float rgb[3]) {
    rgb[0] = 3.240479f * xyz[0] - 1.537150f * xyz[1] - 0.498535f * xyz[2];
    rgb[1] = -0.969256f * xyz[0] + 1.875991f * xyz[1] + 0.041556f * xyz[2];
    rgb[2] = 0.055648f * xyz[0] - 0.204043f * xyz[1] + 1.057311f * xyz[2];
}

inline void RGBToXYZ(const Float rgb[3], Float xyz[3]) {
    xyz[0] = 0.412453f * rgb[0] + 0.357580f * rgb[1] + 0.180423f * rgb[2];
    xyz[1] = 0.212671f * rgb[0] + 0.715160f * rgb[1] + 0.072169f * rgb[2];
    xyz[2] = 0.019334f * rgb[0] + 0.119193f * rgb[1] + 0.950227f * rgb[2];
}


extern Float InterpolateSpectrumSamples(const Float *lambda, const Float *vals,
                                        int n, Float l);
extern void Blackbody(const Float *lambda, int n, Float T, Float *Le);
extern void BlackbodyNormalized(const Float *lambda, int n, Float T,
                                Float *vals);

// Spectral Data Declarations
static const int nCIESamples = 471;
extern const Float CIE_X[nCIESamples];
extern const Float CIE_Y[nCIESamples];
extern const Float CIE_Z[nCIESamples];
extern const Float CIE_lambda[nCIESamples];
static const Float CIE_Y_integral = 106.856895;
static const int nRGB2SpectSamples = 32;


template <int nSpectrumSamples>
class CoefficientSpectrum {
public:
    CoefficientSpectrum(Float v = 0.f) {
        for (int i = 0; i < nSpectrumSamples; ++i) c[i] = v;
        assert(!HasNaNs());
    }


    CoefficientSpectrum &operator+=(const CoefficientSpectrum &s2) {
        assert(!s2.HasNaNs());
        for (int i = 0; i < nSpectrumSamples; ++i) c[i] += s2.c[i];
        return *this;
    }
    CoefficientSpectrum operator+(const CoefficientSpectrum &s2) const {
        assert(!s2.HasNaNs());
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] += s2.c[i];
        return ret;
    }
    CoefficientSpectrum operator-(const CoefficientSpectrum &s2) const {
        assert(!s2.HasNaNs());
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] -= s2.c[i];
        return ret;
    }
    CoefficientSpectrum operator/(const CoefficientSpectrum &s2) const {
        assert(!s2.HasNaNs());
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSpectrumSamples; ++i) {

            ret.c[i] /= s2.c[i];
        }
        return ret;
    }
    CoefficientSpectrum operator*(const CoefficientSpectrum &sp) const {
        assert(!sp.HasNaNs());
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] *= sp.c[i];
        return ret;
    }
    CoefficientSpectrum &operator*=(const CoefficientSpectrum &sp) {
        assert(!sp.HasNaNs());
        for (int i = 0; i < nSpectrumSamples; ++i) c[i] *= sp.c[i];
        return *this;
    }
    CoefficientSpectrum operator*(Float a) const {
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] *= a;
        assert(!ret.HasNaNs());
        return ret;
    }
    CoefficientSpectrum &operator*=(Float a) {
        for (int i = 0; i < nSpectrumSamples; ++i) c[i] *= a;
        assert(!HasNaNs());
        return *this;
    }
    friend inline CoefficientSpectrum operator*(Float a,
                                                const CoefficientSpectrum &s) {
        assert(!std::isnan(a) && !s.HasNaNs());
        return s * a;
    }
    CoefficientSpectrum operator/(Float a) const {
        assert(a!= 0);
        assert(!std::isnan(a));
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] /= a;
        assert(!ret.HasNaNs());
        return ret;
    }
    CoefficientSpectrum &operator/=(Float a) {
        assert(a!= 0);
        assert(!std::isnan(a));
        for (int i = 0; i < nSpectrumSamples; ++i) c[i] /= a;
        return *this;
    }
    bool operator==(const CoefficientSpectrum &sp) const {
        for (int i = 0; i < nSpectrumSamples; ++i)
            if (c[i] != sp.c[i]) return false;
        return true;
    }
    bool operator!=(const CoefficientSpectrum &sp) const {
        return !(*this == sp);
    }
    bool IsBlack() const {
        for (int i = 0; i < nSpectrumSamples; ++i)
            if (c[i] != 0.) return false;
        return true;
    }
    friend CoefficientSpectrum Sqrt(const CoefficientSpectrum &s) {
        CoefficientSpectrum ret;
        for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] = std::sqrt(s.c[i]);
        assert(!ret.HasNaNs());
        return ret;
    }
    template <int n>
    friend inline CoefficientSpectrum<n> Pow(const CoefficientSpectrum<n> &s,
                                             Float e);
    CoefficientSpectrum operator-() const {
        CoefficientSpectrum ret;
        for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] = -c[i];
        return ret;
    }
    friend CoefficientSpectrum Exp(const CoefficientSpectrum &s) {
        CoefficientSpectrum ret;
        for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] = std::exp(s.c[i]);
        assert(!ret.HasNaNs());
        return ret;
    }
    friend std::ostream &operator<<(std::ostream &os,
                                    const CoefficientSpectrum &s) {
        return os << s.ToString();
    }
    std::string ToString() const {
        std::string str = "[ ";
        for (int i = 0; i < nSpectrumSamples; ++i) {
            str += std::to_string( c[i]);
            if (i + 1 < nSpectrumSamples) str += ", ";
        }
        str += " ]";
        return str;
    }
    CoefficientSpectrum Clamp(Float low = 0, Float high = Infinity) const {
        CoefficientSpectrum ret;
        for (int i = 0; i < nSpectrumSamples; ++i)
            ret.c[i] = ::Clamp(c[i], low, high);
        assert(!ret.HasNaNs());
        return ret;
    }
    Float MaxComponentValue() const {
        Float m = c[0];
        for (int i = 1; i < nSpectrumSamples; ++i)
            m = std::max(m, c[i]);
        return m;
    }
    bool HasNaNs() const {
        for (int i = 0; i < nSpectrumSamples; ++i)
            if (std::isnan(c[i])) return true;
        return false;
    }

    Float &operator[](int i) {
        assert(i >= 0 && i < nSpectrumSamples);
        return c[i];
    }
    Float operator[](int i) const {
        assert(i >= 0 && i < nSpectrumSamples);
        return c[i];
    }


    static const int nSamples = nSpectrumSamples;

protected:

    Float c[nSpectrumSamples];
};

class RGBColor : public CoefficientSpectrum<3> {
    using CoefficientSpectrum<3>::c;

public:

    RGBColor(Float v = 0.f) : CoefficientSpectrum<3>(v) {}
    RGBColor(const CoefficientSpectrum<3> &v) : CoefficientSpectrum<3>(v) {}
    RGBColor(const RGBColor &s,
             SpectrumType type = SpectrumType::Reflectance) {
        *this = s;
    }
    static RGBColor FromRGB(const Float rgb[3],
                            SpectrumType type = SpectrumType::Reflectance) {
        RGBColor s;
        s.c[0] = rgb[0];
        s.c[1] = rgb[1];
        s.c[2] = rgb[2];
        assert(!s.HasNaNs());
        return s;
    }
    void ToRGB(Float *rgb) const {
        rgb[0] = c[0];
        rgb[1] = c[1];
        rgb[2] = c[2];
    }
    const RGBColor &ToRGBSpectrum() const { return *this; }
    void ToXYZ(Float xyz[3]) const { RGBToXYZ(c, xyz); }
    static RGBColor FromXYZ(const Float xyz[3],
                            SpectrumType type = SpectrumType::Reflectance) {
        RGBColor r;
        XYZToRGB(xyz, r.c);
        return r;
    }
    Float y() const {
        const Float YWeight[3] = {0.212671f, 0.715160f, 0.072169f};
        return YWeight[0] * c[0] + YWeight[1] * c[1] + YWeight[2] * c[2];
    }
    static RGBColor FromSampled(const Float *lambda, const Float *v, int n) {

        if (!SpectrumSamplesSorted(lambda, v, n)) {
            std::vector<Float> slambda(&lambda[0], &lambda[n]);
            std::vector<Float> sv(&v[0], &v[n]);
            SortSpectrumSamples(&slambda[0], &sv[0], n);
            return FromSampled(&slambda[0], &sv[0], n);
        }
        Float xyz[3] = {0, 0, 0};
        for (int i = 0; i < nCIESamples; ++i) {
            Float val = InterpolateSpectrumSamples(lambda, v, n, CIE_lambda[i]);
            xyz[0] += val * CIE_X[i];
            xyz[1] += val * CIE_Y[i];
            xyz[2] += val * CIE_Z[i];
        }
        Float scale = Float(CIE_lambda[nCIESamples - 1] - CIE_lambda[0]) /
                      Float(CIE_Y_integral * nCIESamples);
        xyz[0] *= scale;
        xyz[1] *= scale;
        xyz[2] *= scale;
        return FromXYZ(xyz);
    }
};


template <int nSpectrumSamples>
inline CoefficientSpectrum<nSpectrumSamples> Pow(
        const CoefficientSpectrum<nSpectrumSamples> &s, Float e) {
    CoefficientSpectrum<nSpectrumSamples> ret;
    for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] = std::pow(s.c[i], e);
    assert(!ret.HasNaNs());
    return ret;
}

inline RGBColor Lerp(Float t, const RGBColor &s1, const RGBColor &s2) {
    return (1 - t) * s1 + t * s2;
}


#endif //RT_COMPACT_COLOR_H
