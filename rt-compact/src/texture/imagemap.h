//
// Created by Sam on 2020/6/19.
//

#ifndef RT_COMPACT_IMAGEMAP_H
#define RT_COMPACT_IMAGEMAP_H

#include "texture.h"
#include "mipmap.h"
#include "color.h"
#include "memory.h"
#include "interaction.h"
struct TexInfo {
    TexInfo(const std::string &f, bool dt, Float ma, ImageWrap wm, Float sc,
            bool gamma)
            : filename(f),
              doTrilinear(dt),
              maxAniso(ma),
              wrapMode(wm),
              scale(sc),
              gamma(gamma) {}
    std::string filename;
    bool doTrilinear;
    Float maxAniso;
    ImageWrap wrapMode;
    Float scale;
    bool gamma;
    bool operator<(const TexInfo &t2) const {
        if (filename != t2.filename) return filename < t2.filename;
        if (doTrilinear != t2.doTrilinear) return doTrilinear < t2.doTrilinear;
        if (maxAniso != t2.maxAniso) return maxAniso < t2.maxAniso;
        if (scale != t2.scale) return scale < t2.scale;
        if (gamma != t2.gamma) return !gamma;
        return wrapMode < t2.wrapMode;
    }
};

/**
 * 图片纹理
 * @tparam Tmemory
 * @tparam Treturn
 */
template <typename Tmemory, typename Treturn>
class ImageTexture : public Texture<Treturn> {
public:
    ImageTexture(std::unique_ptr<TextureMapping2D> m,
                 const std::string &filename, bool doTri, Float maxAniso,
                 ImageWrap wm, Float scale, bool gamma);
    static void ClearCache() {
        textures.erase(textures.begin(), textures.end());
    }
    Treturn Evaluate(const SurfaceHit &si) const {
        Vector2f dstdx, dstdy;
        Point2f st = mapping->Map(si, &dstdx, &dstdy);
        Tmemory mem = mipmap->Lookup(st, dstdx, dstdy);
        Treturn ret;
        convertOut(mem, &ret);
        return ret;
    }

private:
    static MIPMap<Tmemory> *GetTexture(const std::string &filename,
                                       bool doTrilinear, Float maxAniso,
                                       ImageWrap wm, Float scale, bool gamma);
    static void convertIn(const Color &from, Color *to, Float scale,
                          bool gamma) {
        for (int i = 0; i < Color::nSamples; ++i)
            (*to)[i] = scale * (gamma ? InverseGammaCorrect(from[i]) : from[i]);
    }
    static void convertIn(const Color &from, Float *to, Float scale,
                          bool gamma) {
        *to = scale * (gamma ? InverseGammaCorrect(from.y()) : from.y());
    }
    static void convertOut(const Color &from, Color *to) {
        Float rgb[3];
        from.ToRGB(rgb);
        *to = Color::FromRGB(rgb);
    }
    static void convertOut(Float from, Float *to) { *to = from; }

    std::unique_ptr<TextureMapping2D> mapping;
    MIPMap<Tmemory> *mipmap;
    static std::map<TexInfo, std::unique_ptr<MIPMap<Tmemory>>> textures;
};

extern template class ImageTexture<Float, Float>;
extern template class ImageTexture<Color, Color>;

ImageTexture<Float, Float> *CreateImageFloatTexture(const Transform &tex2world,
                                                    const TextureParams &tp);
ImageTexture<Color, Color> *CreateImageSpectrumTexture(
        const Transform &tex2world, const TextureParams &tp);



#endif //RT_COMPACT_IMAGEMAP_H
