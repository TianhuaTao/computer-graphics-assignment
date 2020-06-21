//
// Created by Sam on 2020/6/15.
//

#ifndef RT_COMPACT_IMAGEIO_H
#define RT_COMPACT_IMAGEIO_H

#include "rt_compact.h"
constexpr bool hostLittleEndian =true;
 inline bool WriteImagePFM(const std::string &filename, const Float *rgb,
                          int width, int height) {
    FILE *fp;
    float scale;

    fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        cout << filename<< endl;
        Error("Unable to open output PFM file" );
        return false;
    }

    std::unique_ptr<float[]> scanline(new float[3 * width]);

    // only write 3 channel PFMs here...
    if (fprintf(fp, "PF\n") < 0) goto fail;

    // write the width and height, which must be positive
    if (fprintf(fp, "%d %d\n", width, height) < 0) goto fail;

    // write the scale, which encodes endianness
    scale = hostLittleEndian ? -1.f : 1.f;
    if (fprintf(fp, "%f\n", scale) < 0) goto fail;

    // write the data from bottom left to upper right as specified by
    // http://netpbm.sourceforge.net/doc/pfm.html
    // The raster is a sequence of pixels, packed one after another, with no
    // delimiters of any kind. They are grouped by row, with the pixels in each
    // row ordered left to right and the rows ordered bottom to top.
    for (int y = height - 1; y >= 0; y--) {
        // in case Float is 'double', copy into a staging buffer that's
        // definitely a 32-bit float...
        for (int x = 0; x < 3 * width; ++x) {
            scanline[x] = GammaCorrect(rgb[y * width * 3 + x]);
        }
        if (fwrite(&scanline[0], sizeof(float), width * 3, fp) <
            (size_t)(width * 3))
            goto fail;
    }

    fclose(fp);
    return true;

    fail:
    Error("Error writing PFM file");
    fclose(fp);
    return false;
}
Color *ReadImagePFM(const std::string &filename, int *xres,
                    int *yres);

#endif //RT_COMPACT_IMAGEIO_H
