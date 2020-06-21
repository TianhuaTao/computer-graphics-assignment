//
// Created by Sam on 2020/6/15.
//

#ifndef RT_COMPACT_PARSER_H
#define RT_COMPACT_PARSER_H

#include "rt_compact.h"



struct Loc {
    Loc() = default;
    Loc(const std::string &filename) : filename(filename) {}

    std::string filename;
    int line = 1, column = 0;
};


extern Loc *parserLoc;


class string_view {
public:
    string_view(const char *start, size_t size) : ptr(start), length(size) {}
    string_view() : ptr(nullptr), length(0) {}

    const char *data() const { return ptr; }
    size_t size() const { return length; }
    bool empty() const { return length == 0; }

    char operator[](int index) const { return ptr[index]; }
    char back() const { return ptr[length - 1]; }

    const char *begin() const { return ptr; }
    const char *end() const { return ptr + length; }

    bool operator==(const char *str) const {
        int index;
        for (index = 0; *str; ++index, ++str) {
            if (index >= length) return false;
            if (*str != ptr[index]) return false;
        }
        return index == length;
    }
    bool operator!=(const char *str) const { return !(*this == str); }

    void remove_prefix(int n) {
        ptr += n;
        length -= n;
    }
    void remove_suffix(int n) { length -= n; }

private:
    const char *ptr;
    size_t length;
};

// Tokenizer converts a single pbrt scene file into a series of tokens.
class Tokenizer {
public:
    static std::unique_ptr<Tokenizer> CreateFromFile(
            const std::string &filename,
            std::function<void(const char *)> errorCallback);
    static std::unique_ptr<Tokenizer> CreateFromString(
            std::string str, std::function<void(const char *)> errorCallback);

    ~Tokenizer();

    // Returns an empty string_view at EOF. Note that the returned
    // string_view is not guaranteed to be valid after next call to Next().
    string_view Next();

    Loc loc;

private:
    Tokenizer(std::string str, std::function<void(const char *)> errorCallback);


    int getChar() {
        if (pos == end) return EOF;
        int ch = *pos++;
        if (ch == '\n') {
            ++loc.line;
            loc.column = 0;
        } else
            ++loc.column;
        return ch;
    }
    void ungetChar() {
        --pos;
        if (*pos == '\n')

            --loc.line;
    }

    // This function is called if there is an error during lexing.
    std::function<void(const char *)> errorCallback;



    // If the input is stdin, then we copy everything until EOF into this
    // string and then start lexing.  This is a little wasteful (versus
    // tokenizing directly from stdin), but makes the implementation
    // simpler.
    std::string contents;

    // Pointers to the current position inthe file and one past the end of
    // the file.
    const char *pos, *end;

    // If there are escaped characters in the string, we can't just return
    // a string_view into the mapped file. In that case, we handle the
    // escaped characters and return a string_view to sEscaped.  (And
    // thence, string_views from previous calls to Next() must be invalid
    // after a subsequent call, since we may reuse sEscaped.)
    std::string sEscaped;
};


#endif //RT_COMPACT_PARSER_H
