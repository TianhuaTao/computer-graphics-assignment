//
// Created by Sam on 2020/6/16.
//

#include "rt_compact.h"
#include "parser/parser.h"


inline void stringPrintfRecursive(std::string *s, const char *fmt) {
    const char *c = fmt;
    // No args left; make sure there aren't any extra formatting
    // specifiers.
    while (*c) {
        if (*c == '%') {
            ++c;
        }
        *s += *c++;
    }
}

// 1. Copy from fmt to *s, up to the next formatting directive.
// 2. Advance fmt past the next formatting directive and return the
//    formatting directive as a string.
inline std::string copyToFormatString(const char **fmt_ptr, std::string *s) {
    const char *&fmt = *fmt_ptr;
    while (*fmt) {
        if (*fmt != '%') {
            *s += *fmt;
            ++fmt;
        } else if (fmt[1] == '%') {
            // "%%"; let it pass through
            *s += '%';
            *s += '%';
            fmt += 2;
        } else
            // fmt is at the start of a formatting directive.
            break;
    }

    std::string nextFmt;
    if (*fmt) {
        do {
            nextFmt += *fmt;
            ++fmt;
            // Incomplete (but good enough?) test for the end of the
            // formatting directive: a new formatting directive starts, we
            // hit whitespace, or we hit a comma.
        } while (*fmt && *fmt != '%' && !isspace(*fmt) && *fmt != ',' &&
                 *fmt != '[' && *fmt != ']' && *fmt != '(' && *fmt != ')');
    }

    return nextFmt;
}

template <typename T>
inline std::string formatOne(const char *fmt, T v) {
    // Figure out how much space we need to allocate; add an extra
    // character for the '\0'.
    size_t size = snprintf(nullptr, 0, fmt, v) + 1;
    std::string str;
    str.resize(size);
    snprintf(&str[0], size, fmt, v);
    str.pop_back();  // remove trailing NUL
    return str;
}

// General-purpose version of stringPrintfRecursive; add the formatted
// output for a single StringPrintf() argument to the final result string
// in *s.
template <typename T, typename... Args>
inline void stringPrintfRecursive(std::string *s, const char *fmt, T v,
                                  Args... args) {
    std::string nextFmt = copyToFormatString(&fmt, s);
    *s += formatOne(nextFmt.c_str(), v);
    stringPrintfRecursive(s, fmt, args...);
}

// Special case of StringPrintRecursive for float-valued arguments.
template <typename... Args>
inline void stringPrintfRecursive(std::string *s, const char *fmt, float v,
                                  Args... args) {
    std::string nextFmt = copyToFormatString(&fmt, s);
    if (nextFmt == "%f")
        // Always use enough precision so that the printed value gives
        // the exact floating-point value if it's used to initialize a
        // float.
        // https://randomascii.wordpress.com/2012/03/08/float-precisionfrom-zero-to-100-digits-2/
        *s += formatOne("%.9g", v);
    else
        // If a specific formatting string other than "%f" was specified,
        // just use that.
        *s += formatOne(nextFmt.c_str(), v);

    // Go forth and print the next arg.
    stringPrintfRecursive(s, fmt, args...);
}

// Specialization for doubles that always uses enough precision.  (It seems
// that this is the version that is actually called for floats.  I thought
// that float->double promotion wasn't supposed to happen in this case?)
template <typename... Args>
inline void stringPrintfRecursive(std::string *s, const char *fmt, double v,
                                  Args... args) {
    std::string nextFmt = copyToFormatString(&fmt, s);
    if (nextFmt == "%f")
        *s += formatOne("%.17g", v);
    else
        *s += formatOne(nextFmt.c_str(), v);
    stringPrintfRecursive(s, fmt, args...);
}

// StringPrintf() is a replacement for sprintf() (and the like) that
// returns the result as a std::string. This gives convenience/control
// of printf-style formatting in a more C++-ish way.
//
// Floating-point values with the formatting string "%f" are handled
// specially so that enough digits are always printed so that the original
// float/double can be reconstituted exactly from the printed digits.
template <typename... Args>
inline std::string StringPrintf(const char *fmt, Args... args) {
    std::string ret;
    stringPrintfRecursive(&ret, fmt, args...);
    return ret;
}
// Error Reporting Functions
template <typename... Args>
static std::string StringVaprintf(const std::string &fmt, va_list args) {
    // Figure out how much space we need to allocate; add an extra
    // character for '\0'.
    va_list argsCopy;
    va_copy(argsCopy, args);
    size_t size = vsnprintf(nullptr, 0, fmt.c_str(), args) + 1;
    std::string str;
    str.resize(size);
    vsnprintf(&str[0], size, fmt.c_str(), argsCopy);
    str.pop_back();  // remove trailing NUL
    return str;
}

static void processError(Loc *loc, const char *format, va_list args,
                         const char *errorType) {
    // Build up an entire formatted error string and print it all at once;
    // this way, if multiple threads are printing messages at once, they
    // don't get jumbled up...
    std::string errorString;

    // Print line and position in input file, if available
    if (loc)
        errorString = StringPrintf("%s:%d:%d: ", loc->filename.c_str(),
                                   loc->line, loc->column);

    errorString += errorType;
    errorString += ": ";
    errorString += StringVaprintf(format, args);

    // Print the error message (but not more than one time).
    static std::string lastError;
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    if (errorString != lastError) {
        fprintf(stderr, "%s\n", errorString.c_str());
        lastError = errorString;
    }
}

void Warning(const char *format, ...) {
    va_list args;
    va_start(args, format);
    processError(parserLoc, format, args, "Warning");
    va_end(args);
}

void Error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    processError(parserLoc, format, args, "Error");
    va_end(args);
}
