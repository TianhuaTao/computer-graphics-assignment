//
// Created by Sam on 2020/6/18.
//

#ifndef RT_COMPACT_MATH_H
#define RT_COMPACT_MATH_H

#include "geometry.h"

// BSDF Inline Functions
inline Float CosTheta(const Vector3f &w) { return w.z; }
inline Float Cos2Theta(const Vector3f &w) { return w.z * w.z; }
inline Float AbsCosTheta(const Vector3f &w) { return std::abs(w.z); }
inline Float Sin2Theta(const Vector3f &w) {
    return std::max((Float)0, (Float)1 - Cos2Theta(w));
}

inline Float SinTheta(const Vector3f &w) { return std::sqrt(Sin2Theta(w)); }

inline Float TanTheta(const Vector3f &w) { return SinTheta(w) / CosTheta(w); }

inline Float Tan2Theta(const Vector3f &w) {
    return Sin2Theta(w) / Cos2Theta(w);
}

inline Float CosPhi(const Vector3f &w) {
    Float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 1 : Clamp(w.x / sinTheta, -1, 1);
}

inline Float SinPhi(const Vector3f &w) {
    Float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 0 : Clamp(w.y / sinTheta, -1, 1);
}

inline Float Cos2Phi(const Vector3f &w) { return CosPhi(w) * CosPhi(w); }

inline Float Sin2Phi(const Vector3f &w) { return SinPhi(w) * SinPhi(w); }

inline Float CosDPhi(const Vector3f &wa, const Vector3f &wb) {
    return Clamp(
            (wa.x * wb.x + wa.y * wb.y) / std::sqrt((wa.x * wa.x + wa.y * wa.y) *
                                                    (wb.x * wb.x + wb.y * wb.y)),
            -1, 1);
}

inline Vector3f Reflect(const Vector3f &wo, const Vector3f &n) {
    return -wo + 2 * Dot(wo, n) * n;
}

inline bool Refract(const Vector3f &wi, const Normal3f &n, Float eta,
                    Vector3f *wt) {
    Float cosThetaI = Dot(n, wi);
    Float sin2ThetaI = std::max(Float(0), Float(1 - cosThetaI * cosThetaI));
    Float sin2ThetaT = eta * eta * sin2ThetaI;

    if (sin2ThetaT >= 1) return false;
    Float cosThetaT = std::sqrt(1 - sin2ThetaT);
    *wt = eta * -wi + (eta * cosThetaI - cosThetaT) * Vector3f(n);
    return true;
}

inline bool SameHemisphere(const Vector3f &w, const Vector3f &wp) {
    return w.z * wp.z > 0;
}

inline bool SameHemisphere(const Vector3f &w, const Normal3f &wp) {
    return w.z * wp.z > 0;
}

#endif //RT_COMPACT_MATH_H
