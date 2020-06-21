//
// Created by Sam on 2020/6/15.
//

#ifndef RT_COMPACT_ERROR_H
#define RT_COMPACT_ERROR_H

#include <iostream>
#include <string>

// Setup printf format
#ifdef __GNUG__
#define PRINTF_FUNC __attribute__((__format__(__printf__, 1, 2)))
#else
#define PRINTF_FUNC
#endif  // __GNUG__
void Warning(const char *, ...) PRINTF_FUNC;
void Error(const char *, ...) PRINTF_FUNC;

#endif //RT_COMPACT_ERROR_H
