//
// Created by Sam on 2020/6/16.
//

#include "film.h"
#include "util/imageio.h"
#include "parser/paramset.h"
#include "filter/filter.h"

Film::Film(const Point2i &resolution, const Bounds2f &cropWindow,
           std::unique_ptr<Filter> filt, Float diagonal,
           const std::string &filename, Float scale, Float maxSampleLuminance)
        : fullResolution(resolution),
          diagonal(diagonal * .001),
          filter(std::move(filt)),
          filename(filename),
          scale(scale),
          maxSampleLuminance(maxSampleLuminance) {
    croppedPixelBounds =
            Bounds2i(Point2i(std::ceil(fullResolution.x * cropWindow.pMin.x),
                             std::ceil(fullResolution.y * cropWindow.pMin.y)),
                     Point2i(std::ceil(fullResolution.x * cropWindow.pMax.x),
                             std::ceil(fullResolution.y * cropWindow.pMax.y)));

    // Allocate film image storage
    pixels = std::unique_ptr<Pixel[]>(new Pixel[croppedPixelBounds.Area()]);


    int offset = 0;
    for (int y = 0; y < filterTableWidth; ++y) {
        for (int x = 0; x < filterTableWidth; ++x, ++offset) {
            Point2f p;
            p.x = (x + 0.5f) * filter->radius.x / filterTableWidth;
            p.y = (y + 0.5f) * filter->radius.y / filterTableWidth;
            filterTable[offset] = filter->Evaluate(p);
        }
    }
}

Bounds2i Film::GetSampleBounds() const {
    Bounds2f floatBounds(Floor(Point2f(croppedPixelBounds.pMin) +
                               Vector2f(0.5f, 0.5f) - filter->radius),
                         Ceil(Point2f(croppedPixelBounds.pMax) -
                              Vector2f(0.5f, 0.5f) + filter->radius));
    return (Bounds2i) floatBounds;
}



std::unique_ptr<FilmBlock> Film::GetFilmTile(const Bounds2i &sampleBounds) {
    Vector2f halfPixel = Vector2f(0.5f, 0.5f);
    Bounds2f floatBounds = (Bounds2f) sampleBounds;
    Point2i p0 = (Point2i) Ceil(floatBounds.pMin - halfPixel - filter->radius);
    Point2i p1 = (Point2i) Floor(floatBounds.pMax - halfPixel + filter->radius) +
                 Point2i(1, 1);
    Bounds2i tilePixelBounds = Intersect(Bounds2i(p0, p1), croppedPixelBounds);
    return std::unique_ptr<FilmBlock>(new FilmBlock(
            tilePixelBounds, filter->radius, filterTable, filterTableWidth,
            maxSampleLuminance));
}


void Film::MergeFilmTile(std::unique_ptr<FilmBlock> tile) {

    std::lock_guard<std::mutex> lock(mutex);
    for (Point2i pixel : tile->GetPixelBounds()) {
        const FilmBlockPixel &tilePixel = tile->GetPixel(pixel);
        Pixel &mergePixel = GetPixel(pixel);
        Float xyz[3];
        tilePixel.contribSum.ToXYZ(xyz);
        for (int i = 0; i < 3; ++i) mergePixel.xyz[i] += xyz[i];
        mergePixel.filterWeightSum += tilePixel.filterWeightSum;
    }
}

void Film::WriteImage(Float splatScale) {

    std::unique_ptr<Float[]> rgb(new Float[3 * croppedPixelBounds.Area()]);
    int offset = 0;
    for (Point2i p : croppedPixelBounds) {
        Pixel &pixel = GetPixel(p);
        XYZToRGB(pixel.xyz, &rgb[3 * offset]);

        Float filterWeightSum = pixel.filterWeightSum;
        if (filterWeightSum != 0) {
            Float invWt = (Float) 1 / filterWeightSum;
            rgb[3 * offset] = std::max((Float) 0, rgb[3 * offset] * invWt);
            rgb[3 * offset + 1] =
                    std::max((Float) 0, rgb[3 * offset + 1] * invWt);
            rgb[3 * offset + 2] =
                    std::max((Float) 0, rgb[3 * offset + 2] * invWt);
        }

        Float splatRGB[3];
        Float splatXYZ[3] = {pixel.splatXYZ[0], pixel.splatXYZ[1],
                             pixel.splatXYZ[2]};
        XYZToRGB(splatXYZ, splatRGB);
        rgb[3 * offset] += splatScale * splatRGB[0];
        rgb[3 * offset + 1] += splatScale * splatRGB[1];
        rgb[3 * offset + 2] += splatScale * splatRGB[2];

        rgb[3 * offset] *= scale;
        rgb[3 * offset + 1] *= scale;
        rgb[3 * offset + 2] *= scale;
        ++offset;
    }

    // Write RGB image

    ::WriteImagePFM(filename, &rgb[0], fullResolution.x, fullResolution.y);
}

Film *CreateFilm(const ParamSet &params, std::unique_ptr<Filter> filter) {
    std::string filename;


    filename = params.FindOneString("filename", "output.pfm");

    int xres = params.FindOneInt("xresolution", 1280);
    int yres = params.FindOneInt("yresolution", 720);

    Bounds2f crop;
    int cwi;
    const Float *cr = params.FindFloat("cropwindow", &cwi);
    if (cr && cwi == 4) {
        crop.pMin.x = Clamp(std::min(cr[0], cr[1]), 0.f, 1.f);
        crop.pMax.x = Clamp(std::max(cr[0], cr[1]), 0.f, 1.f);
        crop.pMin.y = Clamp(std::min(cr[2], cr[3]), 0.f, 1.f);
        crop.pMax.y = Clamp(std::max(cr[2], cr[3]), 0.f, 1.f);
    } else if (cr)
        Error("%d values supplied for \"cropwindow\". Expected 4.", cwi);
    else
        crop = Bounds2f(Point2f(0, 0),
                        Point2f(1, 1));

    Float scale = params.FindOneFloat("scale", 1.);
    Float diagonal = params.FindOneFloat("diagonal", 35.);
    Float maxSampleLuminance = params.FindOneFloat("maxsampleluminance",
                                                   Infinity);
    return new Film(Point2i(xres, yres), crop, std::move(filter), diagonal,
                    filename, scale, maxSampleLuminance);
}
