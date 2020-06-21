//
// Created by Sam on 2020/6/15.
//

#ifndef RT_COMPACT_RT_COMPACT_H
#define RT_COMPACT_RT_COMPACT_H

#include "util/error.h"
#include "util/randgen.h"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#define ALLOCA(TYPE, COUNT) (TYPE *) alloca((COUNT) * sizeof(TYPE))
class Scene;

class ParamSet;

class MemoryBank;

class Integrator;

class BSSRDF;

class MediumInteraction;

class SamplerIntegrator;

template<typename T>
class Vector2;

template<typename T>
class Vector3;

template<typename T>
class Point3;

template<typename T>
class Point2;

template<typename T>
class Normal3;

class Ray;

class RayDifferential;

template<typename T>
class Bounds2;

template<typename T>
class Bounds3;

class Transform;

struct Interaction;

class SurfaceHit;

class Shape;

class Light;

class Primitive;

class Sampler;

class Filter;

class GeometricPrimitive;

class TransformedPrimitive;

template<int nSpectrumSamples>
class CoefficientSpectrum;

class RGBColor;



class Medium;

class BSDF;

typedef float Float;
typedef RGBColor Color;


static constexpr Float MaxFloat = std::numeric_limits<Float>::max();
static constexpr Float Infinity = std::numeric_limits<Float>::infinity();
static constexpr Float MachineEpsilon =
        std::numeric_limits<Float>::epsilon() * 0.5;
static constexpr Float ShadowEpsilon = 0.0001f;
static constexpr Float Pi = 3.14159265358979323846;
static constexpr Float InvPi = 0.31830988618379067154;
static constexpr Float Inv2Pi = 0.15915494309189533577;
static constexpr Float Inv4Pi = 0.07957747154594766788;
static constexpr Float PiOver2 = 1.57079632679489661923;
static constexpr Float PiOver4 = 0.78539816339744830961;
static constexpr Float Sqrt2 = 1.41421356237309504880;


inline uint32_t FloatToBits(float f) {
    uint32_t ui;
    memcpy(&ui, &f, sizeof(float));
    return ui;
}

inline float BitsToFloat(uint32_t ui) {
    float f;
    memcpy(&f, &ui, sizeof(uint32_t));
    return f;
}



inline Float gamma(int n) {
    return (n * MachineEpsilon) / (1 - n * MachineEpsilon);
}

inline Float GammaCorrect(Float value) {
    if (value <= 0.0031308f) return 12.92f * value;
    return 1.055f * std::pow(value, (Float) (1.f / 2.4f)) - 0.055f;
}

inline Float InverseGammaCorrect(Float value) {
    if (value <= 0.04045f) return value * 1.f / 12.92f;
    return std::pow((value + 0.055f) * 1.f / 1.055f, (Float) 2.4f);
}

inline float NextFloatUp(float v) {
    if (std::isinf(v) && v > 0.) return v;
    if (v == -0.f) v = 0.f;

    uint32_t ui = FloatToBits(v);
    if (v >= 0)
        ++ui;
    else
        --ui;
    return BitsToFloat(ui);
}

inline float NextFloatDown(float v) {
    if (std::isinf(v) && v < 0.) return v;
    if (v == 0.f) v = -0.f;
    uint32_t ui = FloatToBits(v);
    if (v > 0)
        --ui;
    else
        ++ui;
    return BitsToFloat(ui);
}



template<typename T, typename U, typename V>
inline T Clamp(T val, U low, V high) {
    if (val < low)
        return low;
    else if (val > high)
        return high;
    else
        return val;
}
inline int32_t RoundUpPow2(int32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return v + 1;
}

template <typename T>
class Array2D;

template <typename T>
inline constexpr bool IsPowerOf2(T v) {
    return v && !(v & (v - 1));
}
template<typename T>
inline T Mod(T a, T b) {
    T result = a - (a / b) * b;
    return (T) ((result < 0) ? result + b : result);
}
inline int Log2I(uint32_t n) {
    return (n > 1) ? 1 + Log2I(n >> 1) : 0;
}

inline int Log2Int(int32_t v) { return  Log2I((uint32_t)v); }



inline bool Quadratic(Float A, Float B, Float C, Float *t0, Float *t1) {

    double discrim = (double)B * (double)B - 4. * (double)A * (double)C;
    if (discrim < 0.) return false;
    double rootDiscrim = std::sqrt(discrim);

    Float q;
    if ((float)B < 0)
        q = -.5 * (B - rootDiscrim);
    else
        q = -.5 * (B + rootDiscrim);
    *t0 = q / A;
    *t1 = C / q;
    if ((float)*t0 > (float)*t1) std::swap(*t0, *t1);
    return true;
}

static const Float OneMinusEpsilon = 0x1.fffffep-1;
inline Float Radians(Float deg) { return (Pi / 180) * deg; }

inline Float Degrees(Float rad) { return (180 / Pi) * rad; }

inline Float Log2(Float x) {
    const Float invLog2 = 1.442695040888963387004650940071;
    return std::log(x) * invLog2;
}

inline Float Lerp(Float t, Float v1, Float v2) { return (1 - t) * v1 + t * v2; }

template<typename Predicate>
int FindInterval(int size, const Predicate &pred) {
    int first = 0, len = size;
    while (len > 0) {
        int half = len >> 1, middle = first + half;
        if (pred(middle)) {
            first = middle + 1;
            len -= half + 1;
        } else
            len = half;
    }
    return Clamp(first - 1, 0, size - 2);
}



using std::cout;
using std::endl;
using std::cerr;
using std::shared_ptr;
using std::unique_ptr;
using std::make_shared;
using std::make_unique;

#endif //RT_COMPACT_RT_COMPACT_H
