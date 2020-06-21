#include "rt_compact.h"
#include "api.h"

static void usage(const char *msg = nullptr) {
    if (msg)
        fprintf(stderr, "rt_compact: %s\n\n", msg);
    fprintf(stderr, "usage: rt_compact filename.pbrt\n");
    exit(msg ? 1 : 0);
}

/**
 * 只接收一个参数，为输入文件名
 */
int main(int argc, char *argv[]) {
    if (argc == 1) {
        usage();
        return 0;
    }

    init();
    std::string filename(argv[1]);
    parseFile(filename);
    apiCleanup();

    return 0;
}
