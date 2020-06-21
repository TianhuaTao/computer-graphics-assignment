//
// Created by Sam on 2020/6/16.
//

#ifndef RT_COMPACT_FILM_H
#define RT_COMPACT_FILM_H
#include "rt_compact.h"
#include "math/geometry.h"
#include "color.h"
#include "memory.h"
#include "parallel.h"
#include "filter/filter.h"
struct FilmBlockPixel {
    Color contribSum = 0.f;
    Float filterWeightSum = 0.f;
};
class FilmBlock;
class Film {
public:
    Film(const Point2i &resolution, const Bounds2f &cropWindow,
         std::unique_ptr<Filter> filter, Float diagonal,
         const std::string &filename, Float scale,
         Float maxSampleLuminance = Infinity);
    Bounds2i GetSampleBounds() const;

    std::unique_ptr<FilmBlock> GetFilmTile(const Bounds2i &sampleBounds);
    void MergeFilmTile(std::unique_ptr<FilmBlock> tile);

    void WriteImage(Float splatScale = 1);

    const Point2i fullResolution;
    const Float diagonal;
    std::unique_ptr<Filter> filter;
    const std::string filename;
    Bounds2i croppedPixelBounds;

private:

    struct Pixel {
        Pixel() { xyz[0] = xyz[1] = xyz[2] = filterWeightSum = 0; }
        Float xyz[3];
        Float filterWeightSum;
        Float splatXYZ[3];
        Float pad;
    };
    std::unique_ptr<Pixel[]> pixels;
    static constexpr int filterTableWidth = 16;
    Float filterTable[filterTableWidth * filterTableWidth];
    std::mutex mutex;
    const Float scale;
    const Float maxSampleLuminance;


    Pixel &GetPixel(const Point2i &p) {

        int width = croppedPixelBounds.pMax.x - croppedPixelBounds.pMin.x;
        int offset = (p.x - croppedPixelBounds.pMin.x) +
                     (p.y - croppedPixelBounds.pMin.y) * width;
        return pixels[offset];
    }
};

class FilmBlock {
public:
    FilmBlock(const Bounds2i &pixelBounds, const Vector2f &filterRadius,
              const Float *filterTable, int filterTableSize,
              Float maxSampleLuminance)
            : pixelBounds(pixelBounds),
              filterRadius(filterRadius),
              invFilterRadius(1 / filterRadius.x, 1 / filterRadius.y),
              filterTable(filterTable),
              filterTableSize(filterTableSize),
              maxSampleLuminance(maxSampleLuminance) {
        pixels = std::vector<FilmBlockPixel>(std::max(0, pixelBounds.Area()));
    }
    void AddSample(const Point2f &pFilm, Color L,
                   Float sampleWeight = 1.) {

        if (L.y() > maxSampleLuminance)
            L *= maxSampleLuminance / L.y();
        Point2f pFilmDiscrete = pFilm - Vector2f(0.5f, 0.5f);
        Point2i p0 = (Point2i)Ceil(pFilmDiscrete - filterRadius);
        Point2i p1 =
                (Point2i)Floor(pFilmDiscrete + filterRadius) + Point2i(1, 1);
        p0 = Max(p0, pixelBounds.pMin);
        p1 = Min(p1, pixelBounds.pMax);


        int *ifx = ALLOCA(int, p1.x - p0.x);
        for (int x = p0.x; x < p1.x; ++x) {
            Float fx = std::abs((x - pFilmDiscrete.x) * invFilterRadius.x *
                                filterTableSize);
            ifx[x - p0.x] = std::min((int)std::floor(fx), filterTableSize - 1);
        }
        int *ify = ALLOCA(int, p1.y - p0.y);
        for (int y = p0.y; y < p1.y; ++y) {
            Float fy = std::abs((y - pFilmDiscrete.y) * invFilterRadius.y *
                                filterTableSize);
            ify[y - p0.y] = std::min((int)std::floor(fy), filterTableSize - 1);
        }
        for (int y = p0.y; y < p1.y; ++y) {
            for (int x = p0.x; x < p1.x; ++x) {
                int offset = ify[y - p0.y] * filterTableSize + ifx[x - p0.x];
                Float filterWeight = filterTable[offset];

                FilmBlockPixel &pixel = GetPixel(Point2i(x, y));
                pixel.contribSum += L * sampleWeight * filterWeight;
                pixel.filterWeightSum += filterWeight;
            }
        }
    }
    FilmBlockPixel &GetPixel(const Point2i &p) {

        int width = pixelBounds.pMax.x - pixelBounds.pMin.x;
        int offset =
                (p.x - pixelBounds.pMin.x) + (p.y - pixelBounds.pMin.y) * width;
        return pixels[offset];
    }

    Bounds2i GetPixelBounds() const { return pixelBounds; }

private:
    const Bounds2i pixelBounds;
    const Vector2f filterRadius, invFilterRadius;
    const Float *filterTable;
    const int filterTableSize;
    std::vector<FilmBlockPixel> pixels;
    const Float maxSampleLuminance;
    friend class Film;
};

Film *CreateFilm(const ParamSet &params, std::unique_ptr<Filter> filter);




#endif //RT_COMPACT_FILM_H
