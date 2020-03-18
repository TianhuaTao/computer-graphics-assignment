#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>

using namespace std;

// TODO: implement this class and add more fields as necessary,
class Triangle : public Object3D {

public:
    Triangle() = delete;

    // a b c are three vertex positions of the triangle
    Triangle(const Vector3f &a, const Vector3f &b, const Vector3f &c, Material *m) : Object3D(m) {
        vertices[0] = a;
        vertices[1] = b;
        vertices[2] = c;
        normal = Vector3f::cross(b - a, c - b).normalized();
    }

    bool intersect(const Ray &ray, Hit &hit, float tmin) override {
        auto &p0 = vertices[0];
        auto &p1 = vertices[1];
        auto &p2 = vertices[2];
        auto rhs = p0 - ray.getOrigin();
        Matrix3f A(ray.getDirection(), p0 - p1, p0 - p2);
        auto ans = A.inverse() * (p0 - ray.getOrigin());
        float t = ans[0];
        float beta = ans[1];
        float gamma = ans[2];
        float alpha = 1 - beta - gamma;
        if (t > 0 && t > tmin && t < hit.getT()
            && 0 <= alpha && 0 <= beta && 0 <= gamma
            && alpha <= 1 && beta <= 1 && gamma <= 1)
        {
            hit.set(t,material, normal);
            return true;
        }
        return false;
    }

    Vector3f normal;
    Vector3f vertices[3];
protected:

};

#endif //TRIANGLE_H
