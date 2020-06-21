//
// Created by Sam on 2020/6/15.
//

#ifndef RT_COMPACT_FILEUTIL_H
#define RT_COMPACT_FILEUTIL_H
#include "rt_compact.h"
#include <cstdlib>
#include <climits>
#include <libgen.h>

static std::string searchDirectory;


inline void SetSearchDirectory(const std::string &dirname) {
    searchDirectory = dirname;
}
inline bool IsAbsolutePath(const std::string &filename) {
    return (!filename.empty()) && filename[0] == '/';
}

inline std::string AbsolutePath(const std::string &filename) {
    char full[PATH_MAX];
    if (realpath(filename.c_str(), full))
        return std::string(full);
    else
        return filename;
}

inline std::string ResolveFilename(const std::string &filename) {
    if (searchDirectory.empty() || filename.empty())
        return filename;
    else if (IsAbsolutePath(filename))
        return filename;
    else if (searchDirectory[searchDirectory.size() - 1] == '/')
        return searchDirectory + filename;
    else
        return searchDirectory + "/" + filename;
}

inline std::string DirectoryContaining(const std::string &filename) {
    char *t = strdup(filename.c_str());
    std::string result = dirname(t);
    free(t);
    return result;
}
#endif //RT_COMPACT_FILEUTIL_H
