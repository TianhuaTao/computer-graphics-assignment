//
// Created by Sam on 2020/6/16.
//

#include "parallel.h"
#include <queue>

void ParallelFor(std::function<void(int64_t)> func, int64_t count,
                 int chunkSize) {
    for (int64_t i = 0; i < count; ++i) func(i);

}


void ParallelFor2D(std::function<void(Point2i)> func, const Point2i &count) {
    int nThreads =NumSystemCores();
    std::queue<std::thread> allThreads;
    int finished = 0;
    for (int y = 0; y < count.y; ++y) {
        for (int x = 0; x < count.x; ++x) {
            if (allThreads.size() >= nThreads +4) { /// 使用 核心数+4 个线程
                allThreads.front().join();
                allThreads.pop();
                finished++;
                std::cout<<finished/ Float(count.x*count.y)*100<<"%"<<endl;
            }
            allThreads.push(std::thread(func, Point2i(x, y)));
        }
    }
    while (!allThreads.empty()){
        allThreads.front().join();
        allThreads.pop();
        finished++;
        std::cout<<finished/ Float(count.x*count.y)*100<<"%"<<endl;
    }
}
thread_local int ThreadIndex;