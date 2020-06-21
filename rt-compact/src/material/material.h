//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_MATERIAL_H
#define RT_COMPACT_MATERIAL_H

#include "rt_compact.h"

#include "texture/texture.h"
#include "material.h"
class SurfaceHit;
class TextureParams;
enum class TransportMode { Radiance, Importance };

class Material {
public:
    /**
     * 计算散射方程的主要方法，子类负责重写此方法，把对应的 BxDF 记录在 si 中
     * @param si 存储反射信息
     * @param arena 分配内存
     * @param mode 光线传输模式
     * @param allowMultipleLobes 指示材料是否应使用将多种类型的散射聚集到单个BxDF中的类型。
     */
    virtual void ComputeScatteringFunctions(SurfaceHit *si,
                                            MemoryBank &arena,
                                            TransportMode mode,
                                            bool allowMultipleLobes) const = 0;
    virtual ~Material();

};



#endif //RT_COMPACT_MATERIAL_H
