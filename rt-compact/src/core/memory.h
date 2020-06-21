//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_MEMORY_H
#define RT_COMPACT_MEMORY_H

#include "rt_compact.h"


#define BANK_ALLOC(bank, Type) new ((bank).Alloc(sizeof(Type))) Type

void *AllocMem(size_t sz);

template<typename T>
T *AllocMem(size_t count) {
    return (T *) AllocMem(count * sizeof(T));
}

void FreeMem(void *);

/**
 * 防止内存泄漏的管理类
 * 通过 MemoryBank 分配的内存，会在析构的时候统一释放
 */
class MemoryBank {
public:

    MemoryBank() {}

    ~MemoryBank() {
        for (void *p: malloced_ptr)
            free(p);
    }

    void *Alloc(size_t nBytes) {
        void *p = malloc(nBytes);
        malloced_ptr.push_back(p);
        return p;
    }

    template<typename T>
    T *Alloc(size_t n = 1, bool runConstructor = true) {
        T *ret = (T *) Alloc(n * sizeof(T));
        if (runConstructor)
            for (size_t i = 0; i < n; ++i) new(&ret[i]) T();
        return ret;
    }

private:
    MemoryBank(const MemoryBank &) = delete;
    MemoryBank &operator=(const MemoryBank &) = delete;

    std::vector<void *> malloced_ptr;
};

/**
 * 2D 数组，方便访问元素
 * @tparam T
 */
template<typename T>
class Array2D {
public:

    Array2D(int uRes, int vRes, const T *d = nullptr)
            : uRes(uRes), vRes(vRes) {
        int nAlloc = uRes * vRes;
        data = AllocMem<T>(nAlloc);
        for (int i = 0; i < nAlloc; ++i) new(&data[i]) T();
        if (d)
            for (int v = 0; v < vRes; ++v)
                for (int u = 0; u < uRes; ++u) (*this)(u, v) = d[v * uRes + u];
    }

    int uSize() const { return uRes; }

    int vSize() const { return vRes; }

    ~Array2D() {
        for (int i = 0; i < uRes * vRes; ++i) data[i].~T();
        FreeMem(data);
    }

    T &operator()(int u, int v) {
        int offset = v * uRes + u;
        return data[offset];
    }

    const T &operator()(int u, int v) const {
        int offset = v * uRes + u;
        return data[offset];
    }

private:

    T *data;
    const int uRes, vRes;
};


#endif //RT_COMPACT_MEMORY_H
