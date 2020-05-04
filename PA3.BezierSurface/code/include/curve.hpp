#ifndef CURVE_HPP
#define CURVE_HPP

#include "object3d.hpp"
#include <utility>
#include <vecmath.h>
#include <vector>

#include <algorithm>

//  (PA3): Implement Bernstein class to compute spline basis function.
//       You may refer to the python-script for implementation.

// The CurvePoint object stores information about a point on a curve
// after it has been tesselated: the vertex (V) and the tangent (T)
// It is the responsiblility of functions that create these objects to fill in all the data.
struct CurvePoint {
    Vector3f V; // Vertex
    Vector3f T; // Tangent  (unit)
};

class Curve : public Object3D {
protected:
    std::vector<Vector3f> controls;

public:
    explicit Curve(std::vector<Vector3f> points) : controls(std::move(points)) {}

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        return false;
    }

    std::vector<Vector3f> &getControls() {
        return controls;
    }

    virtual void discretize(int resolution, std::vector<CurvePoint> &data) = 0;

    void drawGL() override {
        Object3D::drawGL();
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glDisable(GL_LIGHTING);
        glColor3f(1, 1, 0);
        glBegin(GL_LINE_STRIP);
        for (auto &control : controls) {
            glVertex3fv(control);
        }
        glEnd();
        glPointSize(4);
        glBegin(GL_POINTS);
        for (auto &control : controls) {
            glVertex3fv(control);
        }
        glEnd();
        std::vector<CurvePoint> sampledPoints;
        discretize(30, sampledPoints);
        glColor3f(1, 1, 1);
        glBegin(GL_LINE_STRIP);
        for (auto &cp : sampledPoints) {
            glVertex3fv(cp.V);
        }
        glEnd();
        glPopAttrib();
    }
};

class BezierCurve : public Curve {
public:
    explicit BezierCurve(const std::vector<Vector3f> &points) : Curve(points) {
        if (points.size() < 4 || points.size() % 3 != 1) {
            printf("Number of control points of BezierCurve must be 3n+1!\n");
            exit(0);
        }
    }

    void discretize(int resolution, std::vector<CurvePoint> &data) override {
        data.clear();
        //  (PA3): fill in data vector
        int nBezCurvePts = (controls.size() - 1) * resolution;
        for (int k = 0; k <= nBezCurvePts; k++) {
            float t = float(k) / float(nBezCurvePts);
            float p[3];
            float pt[3];
            CurvePoint cp;
            computeBezPt(t, p);
            computeBezTanPt(t, pt);
            cp.V.x() = p[0];
            cp.V.y() = p[1];
            cp.V.z() = p[2];
            cp.T.x() = pt[0];
            cp.T.y() = pt[1];
            cp.T.z() = pt[2];
            cp.T.normalize();
            data.push_back(cp);
        }
    }

protected:
    // n!/(i!(n-i)!)
    int binomialCoeff(int n, int i) {
        int c = 1;
        for (int j = n; j >= i + 1; j--) {
            c *= j;
        }
        for (int j = n - i; j >= 2; j--) {
            c /= j;
        }
        return c;
    }
    float computeBezBlendFcn(int n, int i, float t) {
        if (i < 0 || i > n)
            return 0;
        return binomialCoeff(n, i) * pow(t, i) * pow(1 - t, n - i);
    }
    void computeBezPt(float t, float point[3]) {
        int n = controls.size() - 1;
        point[0] = point[1] = point[2] = 0.0;
        for (int k = 0; k < controls.size(); k++) {
            float bezBlendFcn = computeBezBlendFcn(n, k, t);
            point[0] += controls[k].x() * bezBlendFcn;
            point[1] += controls[k].y() * bezBlendFcn;
            point[2] += controls[k].z() * bezBlendFcn;
        }
    }
    void computeBezTanPt(float t, float point[3]) {
        int n = controls.size() - 1;
        point[0] = point[1] = point[2] = 0.0;
        for (int k = 0; k < controls.size(); k++) {
            float bezBlendFcn = n * (computeBezBlendFcn(n - 1, k - 1, t) - computeBezBlendFcn(n - 1, k, t));
            point[0] += controls[k].x() * bezBlendFcn;
            point[1] += controls[k].y() * bezBlendFcn;
            point[2] += controls[k].z() * bezBlendFcn;
        }
    }
};

class BsplineCurve : public Curve {
public:
    BsplineCurve(const std::vector<Vector3f> &points) : Curve(points) {
        if (points.size() < 4) {
            printf("Number of control points of BspineCurve must be more than 4!\n");
            exit(0);
        }
    }

    void discretize(int resolution, std::vector<CurvePoint> &data) override {
        data.clear();
        //  (PA3): fill in data vector
        int n = controls.size() - 1;
        int nBsplineCurvePts = (n - k + 1) * resolution;
        std::vector<float> knots;
        computeKnots(knots);
        float start = knots[k];
        float end = knots[n + 1];
        const float eps = 1e-5;
        for (int i = 0; i <= nBsplineCurvePts; i++) {
            float t = float(i) / float(nBsplineCurvePts) * (end - start) + start;
            float p[3];
            float pt[3];
            CurvePoint cp;
            computeBsplinePt(t, p, knots);
            computeBsplineTanPt(t, pt, knots);
            cp.V.x() = p[0];
            cp.V.y() = p[1];
            cp.V.z() = p[2];
            cp.T.x() = pt[0];
            cp.T.y() = pt[1];
            cp.T.z() = pt[2];
            cp.T.normalize();
            data.push_back(cp);
        }
    }

protected:
    const int k = 3;
    void computeKnots(std::vector<float> &knots) {
        knots.clear();
        int n = controls.size() - 1;
        for (int i = 0; i < n + k + 2; i++) {
            knots.push_back(float(i) / (n + k + 1));
        }
    }
    float computeBsplineBlendFcn(int p, int i, float t, std::vector<float> &knots) {
        int n = controls.size() - 1;
        if (p == 0) {
            return (knots[i] <= t && t < knots[i + 1]) ? 1 : 0;
        } else {
            float c1, c2;
            c1 = (t - knots[i]) / (knots[i + p] - knots[i]);
            c2 = (knots[i + p + 1] - t) / (knots[i + p + 1] - knots[i + 1]);
            float ans = c1 * computeBsplineBlendFcn(p - 1, i, t, knots) + c2 * computeBsplineBlendFcn(p - 1, i + 1, t, knots);
            return ans;
        }
    }
    void computeBsplinePt(float t, float point[3], std::vector<float> &knots) {
        int n = controls.size() - 1;
        point[0] = point[1] = point[2] = 0.0;
        for (int i = 0; i < controls.size(); i++) {
            float bsplineBlendFcn = computeBsplineBlendFcn(k, i, t, knots);
            point[0] += controls[i].x() * bsplineBlendFcn;
            point[1] += controls[i].y() * bsplineBlendFcn;
            point[2] += controls[i].z() * bsplineBlendFcn;
        }
    }
    void computeBsplineTanPt(float t, float point[3], std::vector<float> &knots) {
        int n = controls.size() - 1;
        point[0] = point[1] = point[2] = 0.0;
        for (int i = 0; i < controls.size(); i++) {
            float bsplineBlendFcn = k * (computeBsplineBlendFcn(k - 1, i, t, knots) / (knots[i + k] - knots[i]) - computeBsplineBlendFcn(k - 1, i + 1, t, knots) / (knots[i + k + 1] - knots[i + 1]));
            point[0] += controls[i].x() * bsplineBlendFcn;
            point[1] += controls[i].y() * bsplineBlendFcn;
            point[2] += controls[i].z() * bsplineBlendFcn;
        }
    }
};

#endif // CURVE_HPP
