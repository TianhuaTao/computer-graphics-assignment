#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement Plane representing an infinite plane
// function: ax+by+cz=d
// choose your representation , add more fields and fill in the functions

class Plane : public Object3D {
public:
    Plane() {

    }

    Plane(const Vector3f &normal, float d, Material *m) : Object3D(m),offset(d), normal(normal.normalized()) {

    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        auto tmp = Vector3f::dot(normal,r.getDirection()) ;
        float D = -offset;
        if(tmp== Vector3f::ZERO)
            return false;
        float t = -(D+Vector3f::dot(normal,r.getOrigin()))/tmp;
        if(t>0&& t>tmin&&t<h.getT()){
            if(Vector3f::dot(normal, r.getDirection())<0)
                h.set(t,material,normal);
            else
                h.set(t,material,-normal);
            return true;
        }
        return false;
    }

protected:
    float offset;
    Vector3f normal;

};

#endif //PLANE_H
		

