//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_PROB_H
#define RT_COMPACT_PROB_H

#include "rt_compact.h"
#include "math/geometry.h"

/*
 * 随机采样函数
 */


Vector3f UniformSampleSphere(const Point2f &u);

Float UniformConePdf(Float thetamax);

Point2f ConcentricSampleDisk(const Point2f &u);
Point2f UniformSampleTriangle(const Point2f &u);

inline Vector3f CosineSampleHemisphere(const Point2f &u) {
    Point2f d = ConcentricSampleDisk(u);
    Float z = std::sqrt(std::max((Float)0, 1 - d.x * d.x - d.y * d.y));
    return Vector3f(d.x, d.y, z);
}


inline Float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf) {
    Float f = nf * fPdf, g = ng * gPdf;
    return (f * f) / (f * f + g * g);
}
struct Distribution1D {
    Distribution1D(const Float *f, int n) : func(f, f + n), cdf(n + 1) {
        cdf[0] = 0;
        for (int i = 1; i < n + 1; ++i) cdf[i] = cdf[i - 1] + func[i - 1] / n;

        funcInt = cdf[n];
        if (funcInt == 0) {
            for (int i = 1; i < n + 1; ++i) cdf[i] = Float(i) / Float(n);
        } else {
            for (int i = 1; i < n + 1; ++i) cdf[i] /= funcInt;
        }
    }
    int Count() const { return (int)func.size(); }
    Float SampleContinuous(Float u, Float *pdf, int *off = nullptr) const {
        // Find surrounding CDF segments and _offset_
        int offset = FindInterval((int)cdf.size(),
                                  [&](int index) { return cdf[index] <= u; });
        if (off) *off = offset;
        // Compute offset along CDF segment
        Float du = u - cdf[offset];
        if ((cdf[offset + 1] - cdf[offset]) > 0) {

            du /= (cdf[offset + 1] - cdf[offset]);
        }


        // Compute PDF for sampled offset
        if (pdf) *pdf = (funcInt > 0) ? func[offset] / funcInt : 0;

        return (offset + du) / Count();
    }
    int SampleDiscrete(Float u, Float *pdf = nullptr,
                       Float *uRemapped = nullptr) const {
        // Find surrounding CDF segments and _offset_
        int offset = FindInterval((int)cdf.size(),
                                  [&](int index) { return cdf[index] <= u; });
        if (pdf) *pdf = (funcInt > 0) ? func[offset] / (funcInt * Count()) : 0;
        if (uRemapped)
            *uRemapped = (u - cdf[offset]) / (cdf[offset + 1] - cdf[offset]);

        return offset;
    }


    std::vector<Float> func, cdf;
    Float funcInt;
};
class Distribution2D {
public:
    Distribution2D(const Float *data, int nu, int nv);
    Point2f SampleContinuous(const Point2f &u, Float *pdf) const {
        Float pdfs[2];
        int v;
        Float d1 = pMarginal->SampleContinuous(u[1], &pdfs[1], &v);
        Float d0 = pConditionalV[v]->SampleContinuous(u[0], &pdfs[0]);
        *pdf = pdfs[0] * pdfs[1];
        return Point2f(d0, d1);
    }
    Float Pdf(const Point2f &p) const {
        int iu = Clamp(int(p[0] * pConditionalV[0]->Count()), 0,
                       pConditionalV[0]->Count() - 1);
        int iv =
                Clamp(int(p[1] * pMarginal->Count()), 0, pMarginal->Count() - 1);
        return pConditionalV[iv]->func[iu] / pMarginal->funcInt;
    }

private:

    std::vector<std::unique_ptr<Distribution1D>> pConditionalV;
    std::unique_ptr<Distribution1D> pMarginal;
};
#endif //RT_COMPACT_PROB_H
