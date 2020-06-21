//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_PARALLEL_H
#define RT_COMPACT_PARALLEL_H

#include "rt_compact.h"
#include "math/geometry.h"
#include <thread>

/**
 * 虚假的一维并行，实际是串行的，因为真正的并行都用在二维上
 * @param func 要运行的函数
 * @param count 运行次数，每次把当前 count 传给 func
 * @param chunkSize
 */
void ParallelFor(std::function<void(int64_t)> func, int64_t count,
                 int chunkSize = 1);

/**
 * 真正的并行
 * 认为每个线程工作都差不多，所以用线程队列
 * 最多有 核心数+4 个线程，尽量使CPU使用率为100%
 * @param func
 * @param count
 */
void ParallelFor2D(std::function<void(Point2i)> func, const Point2i &count);

extern thread_local int ThreadIndex;


class AtomicFloat {
public:

    explicit AtomicFloat(Float v = 0) { bits = FloatToBits(v); }

    operator Float() const { return BitsToFloat(bits); }

    Float operator=(Float v) {
        bits = FloatToBits(v);
        return v;
    }

    void Add(Float v) {
        uint32_t oldBits = bits, newBits;
        do {
            newBits = FloatToBits(BitsToFloat(oldBits) + v);
        } while (!bits.compare_exchange_weak(oldBits, newBits));
    }

private:

    std::atomic<uint32_t> bits;

};

inline int NumSystemCores() {
    return std::max(1u, std::thread::hardware_concurrency());
}

#endif //RT_COMPACT_PARALLEL_H
