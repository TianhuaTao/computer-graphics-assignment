//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_FILTER_H
#define RT_COMPACT_FILTER_H

#include "rt_compact.h"
#include "math/geometry.h"

/**
 * 滤波器基类
 */
class Filter {

public:

    virtual ~Filter();

    Filter(const Vector2f &radius)
            : radius(radius), invRadius(Vector2f(1 / radius.x, 1 / radius.y)) {}


     /**
      * 计算相对于中点的 p 点上滤波器给出的值
      * @param p
      * @return
      */
    virtual Float Evaluate(const Point2f &p) const = 0;

    /// 滤波器的有效半径，超过半径的，滤波器的值为0，中点为（0，0）
    const Vector2f radius, invRadius;
};


#endif //RT_COMPACT_FILTER_H
