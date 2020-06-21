//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_BOX_H
#define RT_COMPACT_BOX_H

#include "filter.h"

/**
 * 最简单的方形滤波器，对样本平均加权
 * 会损失高频分量，但是实现简单
 */
class BoxFilter : public Filter {
public:
    BoxFilter(const Vector2f &radius) : Filter(radius) {}
    Float Evaluate(const Point2f &p) const;
};

BoxFilter *CreateBoxFilter(const ParamSet &ps);

#endif //RT_COMPACT_BOX_H
