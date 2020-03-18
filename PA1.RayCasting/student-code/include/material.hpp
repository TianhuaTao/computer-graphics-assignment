#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>
#include <cmath>
#include "ray.hpp"
#include "hit.hpp"
#include <iostream>

// TODO: Implement Shade function that computes Phong introduced in class.
class Material {
public:

    explicit Material(const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO, float s = 0) :
            diffuseColor(d_color), specularColor(s_color), shininess(s) {

    }

    virtual ~Material() = default;

    virtual Vector3f getDiffuseColor() const {
        return diffuseColor;
    }


    Vector3f Shade(const Ray &ray, const Hit &hit,
                   const Vector3f &dirToLight, const Vector3f &lightColor) {
        Vector3f V = -ray.getDirection().normalized();
        const Vector3f normal = hit.getNormal().normalized();
        Vector3f Rx = 2 * (Vector3f::dot(dirToLight, normal)) * normal - dirToLight;
        auto d_tmp = diffuseColor * relu(Vector3f::dot(dirToLight.normalized(), hit.getNormal()));
        auto s_tmp = specularColor * std::pow(relu(Vector3f::dot(V, Rx)), shininess);
        Vector3f shaded = lightColor * (d_tmp + s_tmp);
        return shaded;
    }

protected:
    Vector3f diffuseColor;
    Vector3f specularColor;
    float shininess;

private:
    float relu(float value) {
        return value > 0 ? value : 0;
    }
};


#endif // MATERIAL_H
