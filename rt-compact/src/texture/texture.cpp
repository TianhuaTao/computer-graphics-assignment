//
// Created by Sam on 2020/6/16.
//

#include "texture.h"
#include "interaction.h"

TextureMapping2D::~TextureMapping2D() { }
TextureMapping3D::~TextureMapping3D() { }

UVMapping2D::UVMapping2D(Float su, Float sv, Float du, Float dv)
        : su(su), sv(sv), du(du), dv(dv) {}
Point2f UVMapping2D::Map(const SurfaceHit &si, Vector2f *dstdx,
                         Vector2f *dstdy) const {
    // Compute texture differentials for 2D identity mapping
    *dstdx = Vector2f(su * si.dudx, sv * si.dvdx);
    *dstdy = Vector2f(su * si.dudy, sv * si.dvdy);
    return Point2f(su * si.uv[0] + du, sv * si.uv[1] + dv);
}

Point2f SphericalMapping2D::Map(const SurfaceHit &si, Vector2f *dstdx,
                                Vector2f *dstdy) const {
    Point2f st = sphere(si.p);
    // Compute texture coordinate differentials for sphere $(u,v)$ mapping
    const Float delta = .1f;
    Point2f stDeltaX = sphere(si.p + delta * si.dpdx);
    *dstdx = (stDeltaX - st) / delta;
    Point2f stDeltaY = sphere(si.p + delta * si.dpdy);
    *dstdy = (stDeltaY - st) / delta;

    // Handle sphere mapping discontinuity for coordinate differentials
    if ((*dstdx)[1] > .5)
        (*dstdx)[1] = 1 - (*dstdx)[1];
    else if ((*dstdx)[1] < -.5f)
        (*dstdx)[1] = -((*dstdx)[1] + 1);
    if ((*dstdy)[1] > .5)
        (*dstdy)[1] = 1 - (*dstdy)[1];
    else if ((*dstdy)[1] < -.5f)
        (*dstdy)[1] = -((*dstdy)[1] + 1);
    return st;
}

Point2f SphericalMapping2D::sphere(const Point3f &p) const {
    Vector3f vec = Normalize(WorldToTexture(p) - Point3f(0, 0, 0));
    Float theta = SphericalTheta(vec), phi = SphericalPhi(vec);
    return Point2f(theta * InvPi, phi * Inv2Pi);
}

Point2f CylindricalMapping2D::Map(const SurfaceHit &si, Vector2f *dstdx,
                                  Vector2f *dstdy) const {
    Point2f st = cylinder(si.p);
    // Compute texture coordinate differentials for cylinder $(u,v)$ mapping
    const Float delta = .01f;
    Point2f stDeltaX = cylinder(si.p + delta * si.dpdx);
    *dstdx = (stDeltaX - st) / delta;
    if ((*dstdx)[1] > .5)
        (*dstdx)[1] = 1.f - (*dstdx)[1];
    else if ((*dstdx)[1] < -.5f)
        (*dstdx)[1] = -((*dstdx)[1] + 1);
    Point2f stDeltaY = cylinder(si.p + delta * si.dpdy);
    *dstdy = (stDeltaY - st) / delta;
    if ((*dstdy)[1] > .5)
        (*dstdy)[1] = 1.f - (*dstdy)[1];
    else if ((*dstdy)[1] < -.5f)
        (*dstdy)[1] = -((*dstdy)[1] + 1);
    return st;
}

Point2f PlanarMapping2D::Map(const SurfaceHit &si, Vector2f *dstdx,
                             Vector2f *dstdy) const {
    Vector3f vec(si.p);
    *dstdx = Vector2f(Dot(si.dpdx, vs), Dot(si.dpdx, vt));
    *dstdy = Vector2f(Dot(si.dpdy, vs), Dot(si.dpdy, vt));
    return Point2f(ds + Dot(vec, vs), dt + Dot(vec, vt));
}

Point3f IdentityMapping3D::Map(const SurfaceHit &si, Vector3f *dpdx,
                               Vector3f *dpdy) const {
    *dpdx = WorldToTexture(si.dpdx);
    *dpdy = WorldToTexture(si.dpdy);
    return WorldToTexture(si.p);
}

Float Lanczos(Float x, Float tau) {
    x = std::abs(x);
    if (x < 1e-5f) return 1;
    if (x > 1.f) return 0;
    x *= Pi;
    Float s = std::sin(x * tau) / (x * tau);
    Float lanczos = std::sin(x) / x;
    return s * lanczos;
}
