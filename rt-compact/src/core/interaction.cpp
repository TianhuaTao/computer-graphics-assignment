//
// Created by Sam on 2020/6/15.
//

#include "interaction.h"
#include "light/light.h"
#include "math/geometry.h"
#include "shape/shape.h"
#include "primitive.h"
#include "color.h"

SurfaceHit::SurfaceHit(
        const Point3f &p, const Vector3f &pError, const Point2f &uv,
        const Vector3f &wo, const Vector3f &dpdu, const Vector3f &dpdv,
        const Normal3f &dndu, const Normal3f &dndv, Float time, const Shape *shape,
        int faceIndex)
        : Interaction(p, Normal3f(Normalize(Cross(dpdu, dpdv))), pError, wo, time,
                      nullptr),
          uv(uv),
          dpdu(dpdu),
          dpdv(dpdv),
          dndu(dndu),
          dndv(dndv),
          shape(shape)
          {
    shading.n = n;
    shading.dpdu = dpdu;
    shading.dpdv = dpdv;
    shading.dndu = dndu;
    shading.dndv = dndv;

    if (shape &&
        (shape->reverseOrientation ^ shape->transformSwapsHandedness)) {
        n *= -1;
        shading.n *= -1;
    }
}

void SurfaceHit::SetShadingGeometry(const Vector3f &dpdus,
                                    const Vector3f &dpdvs,
                                    const Normal3f &dndus,
                                    const Normal3f &dndvs,
                                    bool orientationIsAuthoritative) {

    shading.n = Normalize((Normal3f)Cross(dpdus, dpdvs));
    if (orientationIsAuthoritative)
        n = Faceforward(n, shading.n);
    else
        shading.n = Faceforward(shading.n, n);


    shading.dpdu = dpdus;
    shading.dpdv = dpdvs;
    shading.dndu = dndus;
    shading.dndv = dndvs;
}

void SurfaceHit::ComputeScatteringFunctions(const RayDifferential &ray,
                                            MemoryBank &arena,
                                            bool allowMultipleLobes,
                                            TransportMode mode) {
    ComputeDifferentials(ray);
    primitive->ComputeScatteringFunctions(this, arena, mode,
                                          allowMultipleLobes);
}

void SurfaceHit::ComputeDifferentials(
        const RayDifferential &ray) const {
    if (ray.hasDifferentials) {

        Float d = Dot(n, Vector3f(p.x, p.y, p.z));
        Float tx =
                -(Dot(n, Vector3f(ray.rxOrigin)) - d) / Dot(n, ray.rxDirection);
        if (std::isinf(tx) || std::isnan(tx)) goto fail;
        Point3f px = ray.rxOrigin + tx * ray.rxDirection;
        Float ty =
                -(Dot(n, Vector3f(ray.ryOrigin)) - d) / Dot(n, ray.ryDirection);
        if (std::isinf(ty) || std::isnan(ty)) goto fail;
        Point3f py = ray.ryOrigin + ty * ray.ryDirection;
        dpdx = px - p;
        dpdy = py - p;

        int dim[2];
        if (std::abs(n.x) > std::abs(n.y) && std::abs(n.x) > std::abs(n.z)) {
            dim[0] = 1;
            dim[1] = 2;
        } else if (std::abs(n.y) > std::abs(n.z)) {
            dim[0] = 0;
            dim[1] = 2;
        } else {
            dim[0] = 0;
            dim[1] = 1;
        }

        Float A[2][2] = {{dpdu[dim[0]], dpdv[dim[0]]},
                         {dpdu[dim[1]], dpdv[dim[1]]}};
        Float Bx[2] = {px[dim[0]] - p[dim[0]], px[dim[1]] - p[dim[1]]};
        Float By[2] = {py[dim[0]] - p[dim[0]], py[dim[1]] - p[dim[1]]};
        if (!SolveLinearSystem2x2(A, Bx, &dudx, &dvdx)) dudx = dvdx = 0;
        if (!SolveLinearSystem2x2(A, By, &dudy, &dvdy)) dudy = dvdy = 0;
    } else {
        fail:
        dudx = dvdx = 0;
        dudy = dvdy = 0;
        dpdx = dpdy = Vector3f(0, 0, 0);
    }
}

Color SurfaceHit::Le(const Vector3f &w) const {
    const AreaLight *area = primitive->GetAreaLight();
    return area ? area->L(*this, w) : Color(0.f);
}