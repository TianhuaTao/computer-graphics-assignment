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

    Sphere(const Vector3f &center, float radius, Material *material) : Object3D(material) ,center(center), radius(radius){
        // 
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        Vector3f oc = r.getOrigin() - center;
        float a = Vector3f::dot(r.getDirection(), r.getDirection());
        float b = Vector3f::dot(oc, r.getDirection());
        float c = Vector3f::dot(oc, oc) - radius * radius;
        float discriminant = b * b - a * c;
        if (discriminant > 0) {
            float temp = (-b - sqrt(discriminant)) / a;
            if (temp < h.getT() && temp > tmin) {
                auto normal = (r.pointAtParameter(temp)-center)/radius;
                h.set(temp,material, normal) ;
                return true;
            }
            temp = (-b + sqrt(discriminant)) / a;
            if (temp < h.getT() && temp > tmin) {
                auto normal = (r.pointAtParameter(temp)-center)/radius;
                h.set(temp,material, normal) ;
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
