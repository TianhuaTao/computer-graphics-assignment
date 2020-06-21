//
// Created by Sam on 2020/6/16.
//

#include "memory.h"

void *AllocMem(size_t sz) {
    return malloc(sz);
}

void FreeMem(void *ptr) {
    if (!ptr) return;
    free(ptr);
}