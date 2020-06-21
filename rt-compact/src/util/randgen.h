

#ifndef RT_COMPACT_RANDGEN_H
#define RT_COMPACT_RANDGEN_H

#include <functional>
#include <random>

/**
 * 随机数生成器
 * @return [0, 1) 区间的float
 */
inline float random_float() {
    static std::uniform_real_distribution<float> distribution(0.0, 0x1.fffffep-1);
    static std::mt19937 generator;
    static std::function<float()> rand_generator =
            std::bind(distribution, generator);
    return rand_generator();
}

#endif //RT_COMPACT_RANDGEN_H
