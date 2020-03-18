#ifndef CAMERA_H
#define CAMERA_H

#include "ray.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>


class Camera {
public:
    Camera(const Vector3f &center, const Vector3f &direction, const Vector3f &up, int imgW, int imgH) {
        this->center = center;
        this->direction = direction.normalized();       // world
        this->horizontal = Vector3f::cross(this->direction, up).normalized();
        this->up = Vector3f::cross(this->horizontal, this->direction).normalized();
        this->width = imgW;
        this->height = imgH;
    }

    // Generate rays for each screen-space coordinate
    virtual Ray generateRay(const Vector2f &point) = 0;

    virtual ~Camera() = default;

    int getWidth() const { return width; }

    int getHeight() const { return height; }

protected:
    // Extrinsic parameters
    Vector3f center;
    Vector3f direction;
    Vector3f up;
    Vector3f horizontal;
    // Intrinsic parameters
    int width;
    int height;
};

// TODO: Implement Perspective camera
// You can add new functions or variables whenever needed.
class PerspectiveCamera : public Camera {

public:
    PerspectiveCamera(const Vector3f &center, const Vector3f &direction,
                      const Vector3f &up, int imgW, int imgH, float angle)
            : Camera(center, direction, up, imgW, imgH),
              fov(angle) {
        // angle is in radian.
        float half_height = tan(fov / 2);
        float half_width = half_height;
        lower_left_corner = center
                            + this->direction
                            - half_height * this->up
                            - half_width * this->horizontal;
        filmHeight = 2 * half_height * this->up;
        filmWidth = 2 * half_width * this->horizontal;
        fx = (float) imgW / half_width / 2.0f;
        fy = (float) imgH / half_height / 2.0f;
        rotation = Matrix3f(this->horizontal, -this->up, this->direction);
    }

    Ray generateRay(const Vector2f &point) override {
        return Ray(center,
                   lower_left_corner + point.x() / width * filmWidth + point.y() / height * filmHeight - center);
    }

protected:
    float fov;  // field of view, vertical, in radian.
    Matrix3f rotation;
private:
    Vector3f lower_left_corner;
    Vector3f filmWidth;
    Vector3f filmHeight;
    float fx, fy;
};

#endif //CAMERA_H
