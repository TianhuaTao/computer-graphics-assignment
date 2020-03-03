#ifndef SPHERE_H
#define SPHERE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement functions and add more fields as necessary

class Sphere : public Object3D {
public:
    Sphere() :center(Vector3f::ZERO), radius(1){
        // unit ball at the center
    }

    Sphere(const Vector3f &center, float radius, Material *material) : Object3D(material) {
        // 
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        Vector3f oc = r.getOrigin() - center;
        double a = Vector3f::dot(r.getDirection(), r.getDirection());
        double b = Vector3f::dot(oc, r.getDirection());
        double c = Vector3f::dot(oc, oc) - radius * radius;
        double discriminant = b * b - a * c;
        if (discriminant > 0) {
            double temp = (-b - sqrt(discriminant)) / a;
            if (temp < MAXFLOAT && temp > tmin) {
                // rec.t = temp;
                // rec.p = r.point_at_parameter(rec.t);
                // rec.normal = (rec.p - center) / radius;
                // rec.mat_ptr = mat_ptr;
                return true;
            }
            temp = (-b + sqrt(discriminant)) / a;
            if (temp < MAXFLOAT && temp > tmin) {
                // rec.t = temp;
                // rec.p = r.point_at_parameter(rec.t);
                // rec.normal = (rec.p - center) / radius;
                // rec.mat_ptr = mat_ptr;
                return true;
            }
        }
        return false;
    }

protected:
    Vector3f center;

    float radius;
};


#endif
