//
// Created by Sam on 2020/6/16.
//

#include "prob.h"



Point2f ConcentricSampleDisk(const Point2f &u) {
    Point2f uOffset = 2.f * u - Vector2f(1, 1);

    if (uOffset.x == 0 && uOffset.y == 0) return Point2f(0, 0);

    Float theta, r;
    if (std::abs(uOffset.x) > std::abs(uOffset.y)) {
        r = uOffset.x;
        theta = PiOver4 * (uOffset.y / uOffset.x);
    } else {
        r = uOffset.y;
        theta = PiOver2 - PiOver4 * (uOffset.x / uOffset.y);
    }
    return r * Point2f(std::cos(theta), std::sin(theta));
}

Float UniformConePdf(Float cosThetaMax) {
    return 1 / (2 * Pi * (1 - cosThetaMax));
}

Vector3f UniformSampleSphere(const Point2f &u) {
    Float z = 1 - 2 * u[0];
    Float r = std::sqrt(std::max((Float) 0, (Float) 1 - z * z));
    Float phi = 2 * Pi * u[1];
    return Vector3f(r * std::cos(phi), r * std::sin(phi), z);
}



Point2f UniformSampleTriangle(const Point2f &u) {
    Float su0 = std::sqrt(u[0]);
    return Point2f(1 - su0, u[1] * su0);
}



Distribution2D::Distribution2D(const Float *func, int nu, int nv) {
    pConditionalV.reserve(nv);
    for (int v = 0; v < nv; ++v) {
        pConditionalV.emplace_back(new Distribution1D(&func[v * nu], nu));
    }
    std::vector<Float> marginalFunc;
    marginalFunc.reserve(nv);
    for (int v = 0; v < nv; ++v)
        marginalFunc.push_back(pConditionalV[v]->funcInt);
    pMarginal.reset(new Distribution1D(&marginalFunc[0], nv));
}