//
// Created by Sam on 2020/6/16.
//

#include "box.h"
#include "parser/paramset.h"
/// 常函数
Float BoxFilter::Evaluate(const Point2f &p) const { return 1.; }

BoxFilter *CreateBoxFilter(const ParamSet &ps) {
    Float xw = ps.FindOneFloat("xwidth", 0.5f);
    Float yw = ps.FindOneFloat("ywidth", 0.5f);
    return new BoxFilter(Vector2f(xw, yw));
}