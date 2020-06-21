//
// Created by Sam on 2020/6/15.
//

#include "imageio.h"
#include "color.h"
static inline int isWhitespace(char c) {
    return c == ' ' || c == '\n' || c == '\t';
}
// Reads a "word" from the fp and puts it into buffer and adds a null
// terminator.  i.e. it keeps reading until whitespace is reached.  Returns
// the number of characters read *not* including the whitespace, and
// returns -1 on an error.
static int readWord(FILE *fp, char *buffer, int bufferLength) {
    int n;
    int c;

    if (bufferLength < 1) return -1;

    n = 0;
    c = fgetc(fp);
    while (c != EOF && !isWhitespace(c) && n < bufferLength) {
        buffer[n] = c;
        ++n;
        c = fgetc(fp);
    }

    if (n < bufferLength) {
        buffer[n] = '\0';
        return n;
    }

    return -1;
}
const int BUFFER_SIZE = 1024;
 Color *ReadImagePFM(const std::string &filename, int *xres,
                                 int *yres) {
    float *data = nullptr;
    Color *rgb = nullptr;
    char buffer[BUFFER_SIZE];
    unsigned int nFloats;
    int nChannels, width, height;
    float scale;
    bool fileLittleEndian;

    FILE *fp = fopen(filename.c_str(), "rb");
    if (!fp) goto fail;

    // read either "Pf" or "PF"
    if (readWord(fp, buffer, BUFFER_SIZE) == -1) goto fail;

    if (strcmp(buffer, "Pf") == 0)
        nChannels = 1;
    else if (strcmp(buffer, "PF") == 0)
        nChannels = 3;
    else
        goto fail;

    // read the rest of the header
    // read width
    if (readWord(fp, buffer, BUFFER_SIZE) == -1) goto fail;
    width = atoi(buffer);
    *xres = width;

    // read height
    if (readWord(fp, buffer, BUFFER_SIZE) == -1) goto fail;
    height = atoi(buffer);
    *yres = height;

    // read scale
    if (readWord(fp, buffer, BUFFER_SIZE) == -1) goto fail;
    sscanf(buffer, "%f", &scale);

    // read the data
    nFloats = nChannels * width * height;
    data = new float[nFloats];
    // Flip in Y, as P*M has the origin at the lower left.
    for (int y = height - 1; y >= 0; --y) {
        if (fread(&data[y * nChannels * width], sizeof(float),
                  nChannels * width, fp) != nChannels * width)
            goto fail;
    }

    // apply endian conversian and scale if appropriate
    fileLittleEndian = (scale < 0.f);
    if (hostLittleEndian ^ fileLittleEndian) {
        uint8_t bytes[4];
        for (unsigned int i = 0; i < nFloats; ++i) {
            memcpy(bytes, &data[i], 4);
            std::swap(bytes[0], bytes[3]);
            std::swap(bytes[1], bytes[2]);
            memcpy(&data[i], bytes, 4);
        }
    }
    if (std::abs(scale) != 1.f)
        for (unsigned int i = 0; i < nFloats; ++i) data[i] *= std::abs(scale);

    // create RGBs...
    rgb = new Color[width * height];
    if (nChannels == 1) {
        for (int i = 0; i < width * height; ++i) rgb[i] = Color(data[i]);
    } else {
        for (int i = 0; i < width * height; ++i) {
            Float frgb[3] = {data[3 * i], data[3 * i + 1], data[3 * i + 2]};
            rgb[i] = Color::FromRGB(frgb);
        }
    }

    delete[] data;
    fclose(fp);

    return rgb;

    fail:
    Error("Error reading PFM file \"%s\"", filename.c_str());
    if (fp) fclose(fp);
    delete[] data;
    delete[] rgb;
    return nullptr;
}