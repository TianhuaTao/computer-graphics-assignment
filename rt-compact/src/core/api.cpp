//
// Created by Sam on 2020/6/15.
//

#include "api.h"
#include "parser/parser.h"
#include "util/fileutil.h"
#include "integrator/whitted.h"
#include "math/transform.h"
#include "math/geometry.h"
#include "integrator.h"
#include "material/mirror.h"
#include "material/metal.h"
#include "camera/perspective.h"
#include "integrator/path.h"
#include <stack>
#include <material/matte.h>
#include <texture/constant.h>
#include <texture/checkboard.h>
#include <accelerator/bvh.h>
#include <filter/box.h>
#include <light/distant.h>
#include <light/infinite.h>
#include <shape/triangle.h>
#include <light/diffuse.h>
#include <shape/plymesh.h>
#include <medium/homogeneous.h>
#include "shape/sphere.h"
#include "memory.h"
#include "parallel.h"
#include "sampler/random.h"
#include "material/glass.h"
#include "light/spot.h"
#include "texture/imagemap.h"
using namespace std;


constexpr int MaxTransforms = 2;
constexpr int StartTransformBits = 1 << 0;
constexpr int EndTransformBits = 1 << 1;
constexpr int AllTransformsBits = (1 << MaxTransforms) - 1;
struct TransformSet {
    // TransformSet Public Methods
    Transform &operator[](int i) {


        return t[i];
    }
    const Transform &operator[](int i) const {


        return t[i];
    }
    friend TransformSet Inverse(const TransformSet &ts) {
        TransformSet tInv;
        for (int i = 0; i < MaxTransforms; ++i) tInv.t[i] = Inverse(ts.t[i]);
        return tInv;
    }
    bool IsAnimated() const {
        for (int i = 0; i < MaxTransforms - 1; ++i)
            if (t[i] != t[i + 1]) return true;
        return false;
    }

private:
    Transform t[MaxTransforms];
};

struct RenderOptions {
    // RenderOptions Public Methods
    Integrator *MakeIntegrator() const;
    Scene *MakeScene();
    Camera *MakeCamera() const;

    // RenderOptions Public Data
    Float transformStartTime = 0, transformEndTime = 1;
    std::string FilterName = "box";
    ParamSet FilterParams;
    std::string FilmName = "image";
    ParamSet FilmParams;
    std::string SamplerName = "random";
    ParamSet SamplerParams;
    std::string AcceleratorName = "bvh";
    ParamSet AcceleratorParams;
    std::string IntegratorName = "path";
    ParamSet IntegratorParams;
    std::string CameraName = "perspective";
    ParamSet CameraParams;
    TransformSet CameraToWorld;
    std::map<std::string, std::shared_ptr<Medium>> namedMedia;
    std::vector<std::shared_ptr<Light>> lights;
    std::vector<std::shared_ptr<Primitive>> primitives;
    std::map<std::string, std::vector<std::shared_ptr<Primitive>>> instances;
    std::vector<std::shared_ptr<Primitive>> *currentInstance = nullptr;
    bool haveScatteringMedia = false;
};


struct MaterialInstance {
    MaterialInstance() = default;
    MaterialInstance(const std::string &name, const std::shared_ptr<Material> &mtl,
                     ParamSet params)
            : name(name), material(mtl), params(std::move(params)) {}

    std::string name;
    std::shared_ptr<Material> material;
    ParamSet params;
};

struct GraphicsState {
    // Graphics State Methods
    GraphicsState()
            : floatTextures(std::make_shared<FloatTextureMap>()),
              spectrumTextures(std::make_shared<SpectrumTextureMap>()),
              namedMaterials(std::make_shared<NamedMaterialMap>()) {
        ParamSet empty;
        TextureParams tp(empty, empty, *floatTextures, *spectrumTextures);
        std::shared_ptr<Material> mtl(CreateMatteMaterial(tp));
        currentMaterial = std::make_shared<MaterialInstance>("matte", mtl, ParamSet());
    }
    std::shared_ptr<Material> GetMaterialForShape(const ParamSet &geomParams);
    MediumInterface CreateMediumInterface();

    // Graphics State
    std::string currentInsideMedium, currentOutsideMedium;


    using FloatTextureMap = std::map<std::string, std::shared_ptr<Texture<Float>>>;
    std::shared_ptr<FloatTextureMap> floatTextures;
    bool floatTexturesShared = false;

    using SpectrumTextureMap = std::map<std::string, std::shared_ptr<Texture<Color>>>;
    std::shared_ptr<SpectrumTextureMap> spectrumTextures;
    bool spectrumTexturesShared = false;

    using NamedMaterialMap = std::map<std::string, std::shared_ptr<MaterialInstance>>;
    std::shared_ptr<NamedMaterialMap> namedMaterials;
    bool namedMaterialsShared = false;

    std::shared_ptr<MaterialInstance> currentMaterial;
    ParamSet areaLightParams;
    std::string areaLight;
    bool reverseOrientation = false;
};



class TransformCache {
public:
    TransformCache()
            : hashTable(512), hashTableOccupancy(0) {}

    // TransformCache Public Methods
    Transform *Lookup(const Transform &t) {


        int offset = Hash(t) & (hashTable.size() - 1);
        int step = 1;
        while (true) {
            // Keep looking until we find the Transform or determine that
            // it's not present.
            if (!hashTable[offset] || *hashTable[offset] == t)
                break;
            // Advance using quadratic probing.
            offset = (offset + step * step) & (hashTable.size() - 1);
            ++step;
        }

        Transform *tCached = hashTable[offset];
        if (tCached)
        {}
        else {
            tCached = arena.Alloc<Transform>();
            *tCached = t;
            Insert(tCached);
        }
        return tCached;
    }

    void Clear() {
        hashTable.clear();
        hashTable.resize(512);
        hashTableOccupancy = 0;

    }

private:
    void Insert(Transform *tNew);
    void Grow();

    static uint64_t Hash(const Transform &t) {
        const char *ptr = (const char *)(&t.GetMatrix());
        size_t size = sizeof(Matrix4x4);
        uint64_t hash = 14695981039346656037ull;
        while (size > 0) {
            hash ^= *ptr;
            hash *= 1099511628211ull;
            ++ptr;
            --size;
        }
        return hash;
    }

    // TransformCache Private Data
    std::vector<Transform *> hashTable;
    int hashTableOccupancy;
    MemoryBank arena;
};

void TransformCache::Insert(Transform *tNew) {
    if (++hashTableOccupancy == hashTable.size() / 2)
        Grow();

    int baseOffset = Hash(*tNew) & (hashTable.size() - 1);
    for (int nProbes = 0;; ++nProbes) {
        // Quadratic probing.
        int offset = (baseOffset + nProbes/2 + nProbes*nProbes/2) & (hashTable.size() - 1);
        if (hashTable[offset] == nullptr) {
            hashTable[offset] = tNew;
            return;
        }
    }
}

void TransformCache::Grow() {
    std::vector<Transform *> newTable(2 * hashTable.size());

    // Insert current elements into newTable.
    for (Transform *tEntry : hashTable) {
        if (!tEntry) continue;

        int baseOffset = Hash(*tEntry) & (hashTable.size() - 1);
        for (int nProbes = 0;; ++nProbes) {
            // Quadratic probing.
            int offset = (baseOffset + nProbes/2 + nProbes*nProbes/2) & (hashTable.size() - 1);
            if (newTable[offset] == nullptr) {
                newTable[offset] = tEntry;
                break;
            }
        }
    }

    std::swap(hashTable, newTable);
}


// API Static Data
enum class APIState { Uninitialized, OptionsBlock, WorldBlock };
static APIState currentApiState = APIState::Uninitialized;
static TransformSet curTransform;
static uint32_t activeTransformBits = AllTransformsBits;
static std::map<std::string, TransformSet> namedCoordinateSystems;
static std::unique_ptr<RenderOptions> renderOptions;
static GraphicsState graphicsState;
static std::vector<GraphicsState> pushedGraphicsStates;
static std::vector<TransformSet> pushedTransforms;
static std::vector<uint32_t> pushedActiveTransformBits;
static TransformCache transformCache;
int catIndentCount = 0;

// API Forward Declarations
std::vector<std::shared_ptr<Shape>> MakeShapes(const std::string &name,
                                               const Transform *ObjectToWorld,
                                               const Transform *WorldToObject,
                                               bool reverseOrientation,
                                               const ParamSet &paramSet);

// API Macros
#define VERIFY_INITIALIZED(func)                           \
    if (currentApiState == APIState::Uninitialized) {        \
        Error(                                             \
            "init() must be before calling \"%s()\". " \
            "Ignoring.",                                   \
            func);                                         \
        return;                                            \
    } else /* swallow trailing semicolon */
#define VERIFY_OPTIONS(func)                             \
    VERIFY_INITIALIZED(func);                            \
    if (currentApiState == APIState::WorldBlock) {       \
        Error(                                           \
            "Options cannot be set inside world block; " \
            "\"%s\" not allowed.  Ignoring.",            \
            func);                                       \
        return;                                          \
    } else /* swallow trailing semicolon */
#define VERIFY_WORLD(func)                                   \
    VERIFY_INITIALIZED(func);                                \
    if (currentApiState == APIState::OptionsBlock) {         \
        Error(                                               \
            "Scene description must be inside world block; " \
            "\"%s\" not allowed. Ignoring.",                 \
            func);                                           \
        return;                                              \
    } else /* swallow trailing semicolon */
#define FOR_ACTIVE_TRANSFORMS(expr)           \
    for (int i = 0; i < MaxTransforms; ++i)   \
        if (activeTransformBits & (1 << i)) { \
            expr                              \
        }
#define WARN_IF_ANIMATED_TRANSFORM(func)                             \
    do {                                                             \
        if (curTransform.IsAnimated())                               \
            Warning(                                                 \
                "Animated transformations set; ignoring for \"%s\" " \
                "and using the start transform only",                \
                func);                                               \
    } while (false) /* swallow trailing semicolon */

// Object Creation Function Definitions
std::vector<std::shared_ptr<Shape>> MakeShapes(const std::string &name,
                                               const Transform *object2world,
                                               const Transform *world2object,
                                               bool reverseOrientation,
                                               const ParamSet &paramSet) {
    std::vector<std::shared_ptr<Shape>> shapes;
    std::shared_ptr<Shape> s;
    if (name == "sphere")
        s = CreateSphereShape(object2world, world2object, reverseOrientation,
                              paramSet);
        // Create remaining single _Shape_ types


    if (s != nullptr) shapes.push_back(s);
    else if (name == "trianglemesh") {
        shapes = CreateTriangleMeshShape(object2world, world2object,
                                             reverseOrientation, paramSet,
                                             &*graphicsState.floatTextures);
    }
    else if (name == "plymesh")
        shapes = CreatePLYMesh(object2world, world2object, reverseOrientation,
                               paramSet, &*graphicsState.floatTextures);
    else
        Warning("Shape \"%s\" unknown.", name.c_str());

    return shapes;
}


std::shared_ptr<Material> MakeMaterial(const std::string &name,
                                       const TextureParams &mp) {
    Material *material = nullptr;
    if (name == "" || name == "none")
        return nullptr;
    else if (name == "matte")
        material = CreateMatteMaterial(mp);
    else if (name == "glass")
        material = CreateGlassMaterial(mp);
    else if (name == "mirror")
        material = CreateMirrorMaterial(mp);
    else if (name == "metal")
        material = CreateMetalMaterial(mp);
    else {
        Warning("Material \"%s\" unknown. Using \"matte\".", name.c_str());
        material = CreateMatteMaterial(mp);
    }

    mp.ReportUnused();
    if (!material) Error("Unable to create material \"%s\"", name.c_str());

    return std::shared_ptr<Material>(material);
}

std::shared_ptr<Texture<Float>> MakeFloatTexture(const std::string &name,
                                                 const Transform &tex2world,
                                                 const TextureParams &tp) {
    Texture<Float> *tex = nullptr;
    if (name == "constant")
        tex = CreateConstantFloatTexture(tex2world, tp);
    else if (name == "checkerboard")
        tex = CreateCheckerboardFloatTexture(tex2world, tp);
    else if (name == "imagemap")
        tex = CreateImageFloatTexture(tex2world, tp);
    else
        Warning("Float texture \"%s\" unknown.", name.c_str());
    tp.ReportUnused();
    return std::shared_ptr<Texture<Float>>(tex);
}

std::shared_ptr<Texture<Color>> MakeSpectrumTexture(
        const std::string &name, const Transform &tex2world,
        const TextureParams &tp) {
    Texture<Color> *tex = nullptr;
    if (name == "constant")
        tex = CreateConstantSpectrumTexture(tex2world, tp);
    else if (name == "checkerboard")
        tex = CreateCheckerboardSpectrumTexture(tex2world, tp);
    else if (name == "imagemap")
        tex = CreateImageSpectrumTexture(tex2world, tp);
    else
        Warning("Color texture \"%s\" unknown.", name.c_str());
    tp.ReportUnused();
    return std::shared_ptr<Texture<Color>>(tex);
}

std::shared_ptr<Medium> MakeMedium(const std::string &name,
                                   const ParamSet &paramSet,
                                   const Transform &medium2world) {
    Float sig_a_rgb[3] = {.0011f, .0024f, .014f},
            sig_s_rgb[3] = {2.55f, 3.21f, 3.77f};
    Color sig_a = Color::FromRGB(sig_a_rgb),
            sig_s = Color::FromRGB(sig_s_rgb);
    std::string preset = paramSet.FindOneString("preset", "");
    bool found = GetMediumScatteringProperties(preset, &sig_a, &sig_s);
    if (preset != "" && !found)
        Warning("Material preset \"%s\" not found.  Using defaults.",
                preset.c_str());
    Float scale = paramSet.FindOneFloat("scale", 1.f);
    Float g = paramSet.FindOneFloat("g", 0.0f);
    sig_a = paramSet.FindOneSpectrum("sigma_a", sig_a) * scale;
    sig_s = paramSet.FindOneSpectrum("sigma_s", sig_s) * scale;
    Medium *m = NULL;
    if (name == "homogeneous") {
        m = new HomogeneousMedium(sig_a, sig_s, g);
    }  else
        Warning("Medium \"%s\" unknown.", name.c_str());
    paramSet.ReportUnused();
    return std::shared_ptr<Medium>(m);

    return std::shared_ptr<Medium>();
}

std::shared_ptr<Light> MakeLight(const std::string &name,
                                 const ParamSet &paramSet,
                                 const Transform &light2world,
                                 const MediumInterface &mediumInterface) {
    std::shared_ptr<Light> light;
    if (name == "distant")
        light = CreateDistantLight(light2world, paramSet);
    else if (name == "infinite" || name == "exinfinite")
        light = CreateInfiniteLight(light2world, paramSet);
    else if(name == "spot")
        light = CreateSpotLight(light2world, mediumInterface.outside, paramSet);
    else
        Warning("Light \"%s\" unknown.", name.c_str());
    paramSet.ReportUnused();
    return light;
}

std::shared_ptr<AreaLight> MakeAreaLight(const std::string &name,
                                         const Transform &light2world,
                                         const MediumInterface &mediumInterface,
                                         const ParamSet &paramSet,
                                         const std::shared_ptr<Shape> &shape) {
    std::shared_ptr<AreaLight> area;
    if (name == "area" || name == "diffuse")
        area = CreateDiffuseAreaLight(light2world, mediumInterface.outside,
                                      paramSet, shape);
    else
        Warning("Area light \"%s\" unknown.", name.c_str());
    paramSet.ReportUnused();
    return area;
}

std::shared_ptr<Primitive> MakeAccelerator(
        const std::string &name,
        std::vector<std::shared_ptr<Primitive>> prims,
        const ParamSet &paramSet) {
    std::shared_ptr<Primitive> accel;
    if (name == "bvh")
        accel = CreateBVHAccelerator(std::move(prims), paramSet);
    else
        Warning("Accelerator \"%s\" unknown.", name.c_str());
    paramSet.ReportUnused();
    return accel;
}

Camera *MakeCamera(const std::string &name, const ParamSet &paramSet,
                   const TransformSet &cam2worldSet, Float transformStart,
                   Float transformEnd, Film *film) {
    Camera *camera = nullptr;
    MediumInterface mediumInterface = graphicsState.CreateMediumInterface();
    static_assert(MaxTransforms == 2,
                  "TransformCache assumes only two transforms");
    Transform *cam2world[2] = {
            transformCache.Lookup(cam2worldSet[0]),
            transformCache.Lookup(cam2worldSet[1])
    };
    AnimatedTransform animatedCam2World(cam2world[0], transformStart,
                                        cam2world[1], transformEnd);
    if (name == "perspective")
        camera = CreatePerspectiveCamera(paramSet, animatedCam2World, film,
                                         mediumInterface.outside);
    else
        Warning("Camera \"%s\" unknown.", name.c_str());
    paramSet.ReportUnused();
    return camera;
}

std::shared_ptr<Sampler> MakeSampler(const std::string &name,
                                     const ParamSet &paramSet,
                                     const Film *film) {
    Sampler *sampler = nullptr;

    if (name == "random")
        sampler = CreateRandomSampler(paramSet);
    else
        Warning("Sampler \"%s\" unknown.", name.c_str());
    paramSet.ReportUnused();
    return std::shared_ptr<Sampler>(sampler);
}

std::unique_ptr<Filter> MakeFilter(const std::string &name,
                                   const ParamSet &paramSet) {
    Filter *filter = nullptr;
    if (name == "box")
        filter = CreateBoxFilter(paramSet);

    else {
        Error("Filter \"%s\" unknown.", name.c_str());
        exit(1);
    }
    paramSet.ReportUnused();
    return std::unique_ptr<Filter>(filter);
}

Film *MakeFilm(const std::string &name, const ParamSet &paramSet,
               std::unique_ptr<Filter> filter) {
    Film *film = nullptr;
    if (name == "image")
        film = CreateFilm(paramSet, std::move(filter));
    else
        Warning("Film \"%s\" unknown.", name.c_str());
    paramSet.ReportUnused();
    return film;
}

// API Function Definitions
void init() {

    // API Initialization
    if (currentApiState != APIState::Uninitialized)
        Error("init() has already been called.");
    currentApiState = APIState::OptionsBlock;
    renderOptions.reset(new RenderOptions);
    graphicsState = GraphicsState();
    catIndentCount = 0;

}

void apiCleanup() {
    // API Cleanup
    if (currentApiState == APIState::Uninitialized)
        Error("apiCleanup() called without init().");
    else if (currentApiState == APIState::WorldBlock)
        Error("apiCleanup() called while inside world block.");
    currentApiState = APIState::Uninitialized;
}

void apiIdentity() {
    VERIFY_INITIALIZED("Identity");
    FOR_ACTIVE_TRANSFORMS(curTransform[i] = Transform();)
}

void apiTranslate(Float dx, Float dy, Float dz) {
    VERIFY_INITIALIZED("Translate");
    FOR_ACTIVE_TRANSFORMS(curTransform[i] = curTransform[i] *
                                            Translate(Vector3f(dx, dy, dz));)
}

void apiTransform(Float *tr) {
    VERIFY_INITIALIZED("Transform");
    FOR_ACTIVE_TRANSFORMS(
            curTransform[i] = Transform(Matrix4x4(
                    tr[0], tr[4], tr[8], tr[12], tr[1], tr[5], tr[9], tr[13], tr[2],
                    tr[6], tr[10], tr[14], tr[3], tr[7], tr[11], tr[15]));)

}

void apiConcatTransform(Float *tr) {
    VERIFY_INITIALIZED("ConcatTransform");
    FOR_ACTIVE_TRANSFORMS(
            curTransform[i] =
                    curTransform[i] *
                    Transform(Matrix4x4(tr[0], tr[4], tr[8], tr[12], tr[1], tr[5],
                                        tr[9], tr[13], tr[2], tr[6], tr[10], tr[14],
                                        tr[3], tr[7], tr[11], tr[15]));)
}

void apiRotate(Float angle, Float dx, Float dy, Float dz) {
    VERIFY_INITIALIZED("Rotate");
    FOR_ACTIVE_TRANSFORMS(curTransform[i] =
                                  curTransform[i] *
                                  Rotate(angle, Vector3f(dx, dy, dz));)

}

void apiScale(Float sx, Float sy, Float sz) {
    VERIFY_INITIALIZED("Scale");
    FOR_ACTIVE_TRANSFORMS(curTransform[i] =
                                  curTransform[i] * Scale(sx, sy, sz);)
}

void apiLookAt(Float ex, Float ey, Float ez, Float lx, Float ly, Float lz,
               Float ux, Float uy, Float uz) {
    VERIFY_INITIALIZED("LookAt");
    Transform lookAt =
            LookAt(Point3f(ex, ey, ez), Point3f(lx, ly, lz), Vector3f(ux, uy, uz));
    FOR_ACTIVE_TRANSFORMS(curTransform[i] = curTransform[i] * lookAt;);

}

void apiCoordinateSystem(const std::string &name) {
    VERIFY_INITIALIZED("CoordinateSystem");
    namedCoordinateSystems[name] = curTransform;

}

void apiCoordSysTransform(const std::string &name) {
    VERIFY_INITIALIZED("CoordSysTransform");
    if (namedCoordinateSystems.find(name) != namedCoordinateSystems.end())
        curTransform = namedCoordinateSystems[name];
    else
        Warning("Couldn't find named coordinate system \"%s\"", name.c_str());
}

void apiActiveTransformAll() {
    activeTransformBits = AllTransformsBits;
}

void apiActiveTransformEndTime() {
    activeTransformBits = EndTransformBits;
    }

void apiActiveTransformStartTime() {
    activeTransformBits = StartTransformBits;
}

void apiTransformTimes(Float start, Float end) {
    VERIFY_OPTIONS("TransformTimes");
    renderOptions->transformStartTime = start;
    renderOptions->transformEndTime = end;
}

void apiPixelFilter(const std::string &name, const ParamSet &params) {
    VERIFY_OPTIONS("PixelFilter");
    renderOptions->FilterName = name;
    renderOptions->FilterParams = params;
}

void apiFilm(const std::string &type, const ParamSet &params) {
    VERIFY_OPTIONS("Film");
    renderOptions->FilmParams = params;
    renderOptions->FilmName = type;
}

void apiSampler(const std::string &name, const ParamSet &params) {
    VERIFY_OPTIONS("Sampler");
    renderOptions->SamplerName = name;
    renderOptions->SamplerParams = params;
}

void apiAccelerator(const std::string &name, const ParamSet &params) {
    VERIFY_OPTIONS("Accelerator");
    renderOptions->AcceleratorName = name;
    renderOptions->AcceleratorParams = params;
}

void apiIntegrator(const std::string &name, const ParamSet &params) {
    VERIFY_OPTIONS("Integrator");
    renderOptions->IntegratorName = name;
    renderOptions->IntegratorParams = params;
}

void apiCamera(const std::string &name, const ParamSet &params) {
    VERIFY_OPTIONS("Camera");
    renderOptions->CameraName = name;
    renderOptions->CameraParams = params;
    renderOptions->CameraToWorld = Inverse(curTransform);
    namedCoordinateSystems["camera"] = renderOptions->CameraToWorld;
}

void apiMakeNamedMedium(const std::string &name, const ParamSet &params) {
    VERIFY_INITIALIZED("MakeNamedMedium");
    WARN_IF_ANIMATED_TRANSFORM("MakeNamedMedium");
    std::string type = params.FindOneString("type", "");
    if (type == "")
        Error("No parameter string \"type\" found in MakeNamedMedium");
    else {
        std::shared_ptr<Medium> medium =
                MakeMedium(type, params, curTransform[0]);
        if (medium) renderOptions->namedMedia[name] = medium;
    }
}

void apiMediumInterface(const std::string &insideName,
                        const std::string &outsideName) {
    VERIFY_INITIALIZED("MediumInterface");
    graphicsState.currentInsideMedium = insideName;
    graphicsState.currentOutsideMedium = outsideName;
    renderOptions->haveScatteringMedia = true;
}

void apiWorldBegin() {
    VERIFY_OPTIONS("WorldBegin");
    currentApiState = APIState::WorldBlock;
    for (int i = 0; i < MaxTransforms; ++i) curTransform[i] = Transform();
    activeTransformBits = AllTransformsBits;
    namedCoordinateSystems["world"] = curTransform;
}

void apiAttributeBegin() {
    VERIFY_WORLD("AttributeBegin");
    pushedGraphicsStates.push_back(graphicsState);
    graphicsState.floatTexturesShared = graphicsState.spectrumTexturesShared =
    graphicsState.namedMaterialsShared = true;
    pushedTransforms.push_back(curTransform);
    pushedActiveTransformBits.push_back(activeTransformBits);
}

void apiAttributeEnd() {
    VERIFY_WORLD("AttributeEnd");
    if (!pushedGraphicsStates.size()) {
        Error(
                "Unmatched apiAttributeEnd() encountered. "
                "Ignoring it.");
        return;
    }
    graphicsState = std::move(pushedGraphicsStates.back());
    pushedGraphicsStates.pop_back();
    curTransform = pushedTransforms.back();
    pushedTransforms.pop_back();
    activeTransformBits = pushedActiveTransformBits.back();
    pushedActiveTransformBits.pop_back();
}

void apiTransformBegin() {
    VERIFY_WORLD("TransformBegin");
    pushedTransforms.push_back(curTransform);
    pushedActiveTransformBits.push_back(activeTransformBits);
}

void apiTransformEnd() {
    VERIFY_WORLD("TransformEnd");
    if (!pushedTransforms.size()) {
        Error(
                "Unmatched apiTransformEnd() encountered. "
                "Ignoring it.");
        return;
    }
    curTransform = pushedTransforms.back();
    pushedTransforms.pop_back();
    activeTransformBits = pushedActiveTransformBits.back();
    pushedActiveTransformBits.pop_back();
}

void apiTexture(const std::string &name, const std::string &type,
                const std::string &texname, const ParamSet &params) {
    VERIFY_WORLD("Texture");

    TextureParams tp(params, params, *graphicsState.floatTextures,
                     *graphicsState.spectrumTextures);
    if (type == "float") {
        // Create _Float_ texture and store in _floatTextures_
        if (graphicsState.floatTextures->find(name) !=
            graphicsState.floatTextures->end())
            Warning("Texture \"%s\" being redefined", name.c_str());
        WARN_IF_ANIMATED_TRANSFORM("Texture");
        std::shared_ptr<Texture<Float>> ft =
                MakeFloatTexture(texname, curTransform[0], tp);
        if (ft) {

            if (graphicsState.floatTexturesShared) {
                graphicsState.floatTextures =
                        std::make_shared<GraphicsState::FloatTextureMap>(*graphicsState.floatTextures);
                graphicsState.floatTexturesShared = false;
            }
            (*graphicsState.floatTextures)[name] = ft;
        }
    } else if (type == "color" || type == "spectrum") {
        // Create _color_ texture and store in _spectrumTextures_
        if (graphicsState.spectrumTextures->find(name) !=
            graphicsState.spectrumTextures->end())
            Warning("Texture \"%s\" being redefined", name.c_str());
        WARN_IF_ANIMATED_TRANSFORM("Texture");
        std::shared_ptr<Texture<Color>> st =
                MakeSpectrumTexture(texname, curTransform[0], tp);
        if (st) {
            if (graphicsState.spectrumTexturesShared) {
                graphicsState.spectrumTextures =
                        std::make_shared<GraphicsState::SpectrumTextureMap>(*graphicsState.spectrumTextures);
                graphicsState.spectrumTexturesShared = false;
            }
            (*graphicsState.spectrumTextures)[name] = st;
        }
    } else
        Error("Texture type \"%s\" unknown.", type.c_str());
}

void apiMaterial(const std::string &name, const ParamSet &params) {
    VERIFY_WORLD("Material");
    ParamSet emptyParams;
    TextureParams mp(params, emptyParams, *graphicsState.floatTextures,
                     *graphicsState.spectrumTextures);
    std::shared_ptr<Material> mtl = MakeMaterial(name, mp);
    graphicsState.currentMaterial =
            std::make_shared<MaterialInstance>(name, mtl, params);

}

void apiMakeNamedMaterial(const std::string &name, const ParamSet &params) {
    VERIFY_WORLD("MakeNamedMaterial");
    // error checking, warning if replace, what to use for transform?
    ParamSet emptyParams;
    TextureParams mp(params, emptyParams, *graphicsState.floatTextures,
                     *graphicsState.spectrumTextures);
    std::string matName = mp.FindString("type");
    WARN_IF_ANIMATED_TRANSFORM("MakeNamedMaterial");
    if (matName == "")
        Error("No parameter string \"type\" found in MakeNamedMaterial");


        std::shared_ptr<Material> mtl = MakeMaterial(matName, mp);
        if (graphicsState.namedMaterials->find(name) !=
            graphicsState.namedMaterials->end())
            Warning("Named material \"%s\" redefined.", name.c_str());
        if (graphicsState.namedMaterialsShared) {
            graphicsState.namedMaterials =
                    std::make_shared<GraphicsState::NamedMaterialMap>(*graphicsState.namedMaterials);
            graphicsState.namedMaterialsShared = false;
        }
        (*graphicsState.namedMaterials)[name] =
                std::make_shared<MaterialInstance>(matName, mtl, params);

}

void apiNamedMaterial(const std::string &name) {
    VERIFY_WORLD("NamedMaterial");

    auto iter = graphicsState.namedMaterials->find(name);
    if (iter == graphicsState.namedMaterials->end()) {
        Error("NamedMaterial \"%s\" unknown.", name.c_str());
        return;
    }
    graphicsState.currentMaterial = iter->second;
}

void apiLightSource(const std::string &name, const ParamSet &params) {
    VERIFY_WORLD("LightSource");
    WARN_IF_ANIMATED_TRANSFORM("LightSource");
    MediumInterface mi = graphicsState.CreateMediumInterface();
    std::shared_ptr<Light> lt = MakeLight(name, params, curTransform[0], mi);
    if (!lt)
        Error("LightSource: light type \"%s\" unknown.", name.c_str());
    else
        renderOptions->lights.push_back(lt);

}

void apiAreaLightSource(const std::string &name, const ParamSet &params) {
    VERIFY_WORLD("AreaLightSource");
    graphicsState.areaLight = name;
    graphicsState.areaLightParams = params;

}

void apiShape(const std::string &name, const ParamSet &params) {
    VERIFY_WORLD("Shape");
    std::vector<std::shared_ptr<Primitive>> prims;
    std::vector<std::shared_ptr<AreaLight>> areaLights;

    if (!curTransform.IsAnimated()) {
        // Initialize _prims_ and _areaLights_ for static shape

        // Create shapes for shape _name_
        Transform *ObjToWorld = transformCache.Lookup(curTransform[0]);
        Transform *WorldToObj = transformCache.Lookup(Inverse(curTransform[0]));
        std::vector<std::shared_ptr<Shape>> shapes =
                MakeShapes(name, ObjToWorld, WorldToObj,
                           graphicsState.reverseOrientation, params);
        if (shapes.empty()) return;
        std::shared_ptr<Material> mtl = graphicsState.GetMaterialForShape(params);
        params.ReportUnused();
        MediumInterface mi = graphicsState.CreateMediumInterface();
        prims.reserve(shapes.size());
        for (auto s : shapes) {
            // Possibly create area light for shape
            std::shared_ptr<AreaLight> area;
            if (graphicsState.areaLight != "") {
                area = MakeAreaLight(graphicsState.areaLight, curTransform[0],
                                     mi, graphicsState.areaLightParams, s);
                if (area) areaLights.push_back(area);
            }
            prims.push_back(
                    std::make_shared<GeometricPrimitive>(s, mtl, area, mi));
        }
    } else {
        // Initialize _prims_ and _areaLights_ for animated shape

        // Create initial shape or shapes for animated shape
        if (graphicsState.areaLight != "")
            Warning(
                    "Ignoring currently set area light when creating "
                    "animated shape");
        Transform *identity = transformCache.Lookup(Transform());
        std::vector<std::shared_ptr<Shape>> shapes = MakeShapes(
                name, identity, identity, graphicsState.reverseOrientation, params);
        if (shapes.empty()) return;

        // Create _GeometricPrimitive_(s) for animated shape
        std::shared_ptr<Material> mtl = graphicsState.GetMaterialForShape(params);
        params.ReportUnused();
        MediumInterface mi = graphicsState.CreateMediumInterface();
        prims.reserve(shapes.size());
        for (auto s : shapes)
            prims.push_back(
                    std::make_shared<GeometricPrimitive>(s, mtl, nullptr, mi));

        // Create single _TransformedPrimitive_ for _prims_

        // Get _animatedObjectToWorld_ transform for shape
        static_assert(MaxTransforms == 2,
                      "TransformCache assumes only two transforms");
        Transform *ObjToWorld[2] = {
                transformCache.Lookup(curTransform[0]),
                transformCache.Lookup(curTransform[1])
        };
        AnimatedTransform animatedObjectToWorld(
                ObjToWorld[0], renderOptions->transformStartTime, ObjToWorld[1],
                renderOptions->transformEndTime);
        if (prims.size() > 1) {
            std::shared_ptr<Primitive> bvh = std::make_shared<BVH>(prims);
            prims.clear();
            prims.push_back(bvh);
        }
        prims[0] = std::make_shared<TransformedPrimitive>(
                prims[0], animatedObjectToWorld);
    }
    // Add _prims_ and _areaLights_ to scene or current instance
    if (renderOptions->currentInstance) {
        if (areaLights.size())
            Warning("Area lights not supported with object instancing");
        renderOptions->currentInstance->insert(
                renderOptions->currentInstance->end(), prims.begin(), prims.end());
    } else {
        renderOptions->primitives.insert(renderOptions->primitives.end(),
                                         prims.begin(), prims.end());
        if (areaLights.size())
            renderOptions->lights.insert(renderOptions->lights.end(),
                                         areaLights.begin(), areaLights.end());
    }
}


bool shapeMaySetMaterialParameters(const ParamSet &ps) {
    for (const auto &param : ps.textures)
        // Any texture other than one for an alpha mask is almost certainly
        // for a Material (or is unused!).
        if (param->name != "alpha" && param->name != "shadowalpha")
            return true;

    // Special case spheres, which are the most common non-mesh primitive.
    for (const auto &param : ps.floats)
        if (param->nValues == 1 && param->name != "radius")
            return true;

    // Extra special case strings, since plymesh uses "filename", curve "type",
    // and loopsubdiv "scheme".
    for (const auto &param : ps.strings)
        if (param->nValues == 1 && param->name != "filename" &&
            param->name != "type" && param->name != "scheme")
            return true;

    // For all other parameter types, if there is a single value of the
    // parameter, assume it may be for the material. This should be valid
    // (if conservative), since no materials currently take array
    // parameters.
    for (const auto &param : ps.bools)
        if (param->nValues == 1)
            for (const auto &param : ps.bools)
                if (param->nValues == 1)
                    return true;
    for (const auto &param : ps.ints)
        if (param->nValues == 1)
            return true;
    for (const auto &param : ps.point2fs)
        if (param->nValues == 1)
            return true;
    for (const auto &param : ps.vector2fs)
        if (param->nValues == 1)
            return true;
    for (const auto &param : ps.point3fs)
        if (param->nValues == 1)
            return true;
    for (const auto &param : ps.vector3fs)
        if (param->nValues == 1)
            return true;
    for (const auto &param : ps.normals)
        if (param->nValues == 1)
            return true;
    for (const auto &param : ps.spectra)
        if (param->nValues == 1)
            return true;

    return false;
}

std::shared_ptr<Material> GraphicsState::GetMaterialForShape(
        const ParamSet &shapeParams) {

    if (shapeMaySetMaterialParameters(shapeParams)) {
        // Only create a unique material for the shape if the shape's
        // parameters are (apparently) going to provide values for some of
        // the material parameters.
        TextureParams mp(shapeParams, currentMaterial->params, *floatTextures,
                         *spectrumTextures);
        return MakeMaterial(currentMaterial->name, mp);
    } else
        return currentMaterial->material;
}

MediumInterface GraphicsState::CreateMediumInterface() {
    MediumInterface m;
    if (currentInsideMedium != "") {
        if (renderOptions->namedMedia.find(currentInsideMedium) !=
            renderOptions->namedMedia.end())
            m.inside = renderOptions->namedMedia[currentInsideMedium].get();
        else
            Error("Named medium \"%s\" undefined.",
                  currentInsideMedium.c_str());
    }
    if (currentOutsideMedium != "") {
        if (renderOptions->namedMedia.find(currentOutsideMedium) !=
            renderOptions->namedMedia.end())
            m.outside = renderOptions->namedMedia[currentOutsideMedium].get();
        else
            Error("Named medium \"%s\" undefined.",
                  currentOutsideMedium.c_str());
    }
    return m;
}

void apiReverseOrientation() {
    VERIFY_WORLD("ReverseOrientation");
    graphicsState.reverseOrientation = !graphicsState.reverseOrientation;

}

void apiObjectBegin(const std::string &name) {
    VERIFY_WORLD("ObjectBegin");
    apiAttributeBegin();
    if (renderOptions->currentInstance)
        Error("ObjectBegin called inside of instance definition");
    renderOptions->instances[name] = std::vector<std::shared_ptr<Primitive>>();
    renderOptions->currentInstance = &renderOptions->instances[name];
}



void apiObjectEnd() {
    VERIFY_WORLD("ObjectEnd");
    if (!renderOptions->currentInstance)
        Error("ObjectEnd called outside of instance definition");
    renderOptions->currentInstance = nullptr;
    apiAttributeEnd();

}



void apiObjectInstance(const std::string &name) {
    VERIFY_WORLD("ObjectInstance");

    // Perform object instance error checking
    if (renderOptions->currentInstance) {
        Error("ObjectInstance can't be called inside instance definition");
        return;
    }
    if (renderOptions->instances.find(name) == renderOptions->instances.end()) {
        Error("Unable to find instance named \"%s\"", name.c_str());
        return;
    }
    std::vector<std::shared_ptr<Primitive>> &in =
            renderOptions->instances[name];
    if (in.empty()) return;
    if (in.size() > 1) {
        // Create group for instance _Primitive_s
        std::shared_ptr<Primitive> accel(
                MakeAccelerator(renderOptions->AcceleratorName, std::move(in),
                                renderOptions->AcceleratorParams));
        if (!accel) accel = std::make_shared<BVH>(in);
        in.clear();
        in.push_back(accel);
    }
    static_assert(MaxTransforms == 2,
                  "TransformCache assumes only two transforms");
    // Create _animatedInstanceToWorld_ transform for instance
    Transform *InstanceToWorld[2] = {
            transformCache.Lookup(curTransform[0]),
            transformCache.Lookup(curTransform[1])
    };
    AnimatedTransform animatedInstanceToWorld(
            InstanceToWorld[0], renderOptions->transformStartTime,
            InstanceToWorld[1], renderOptions->transformEndTime);
    std::shared_ptr<Primitive> prim(
            std::make_shared<TransformedPrimitive>(in[0], animatedInstanceToWorld));
    renderOptions->primitives.push_back(prim);
}

void apiWorldEnd() {
    VERIFY_WORLD("WorldEnd");
    // Ensure there are no pushed graphics states
    while (pushedGraphicsStates.size()) {
        Warning("Missing end to apiAttributeBegin()");
        pushedGraphicsStates.pop_back();
        pushedTransforms.pop_back();
    }
    while (pushedTransforms.size()) {
        Warning("Missing end to apiTransformBegin()");
        pushedTransforms.pop_back();
    }

    // Create scene and render
    {
        std::unique_ptr<Integrator> integrator(renderOptions->MakeIntegrator());
        std::unique_ptr<Scene> scene(renderOptions->MakeScene());

        if (scene && integrator) integrator->Render(*scene);

    }

    // Clean up after rendering. Do this before reporting stats so that
    // destructors can run and update stats as needed.
    graphicsState = GraphicsState();
    transformCache.Clear();
    currentApiState = APIState::OptionsBlock;
    ImageTexture<Float, Float>::ClearCache();
    ImageTexture<RGBColor, Color>::ClearCache();
    renderOptions.reset(new RenderOptions);



    for (int i = 0; i < MaxTransforms; ++i) curTransform[i] = Transform();
    activeTransformBits = AllTransformsBits;
    namedCoordinateSystems.erase(namedCoordinateSystems.begin(),
                                 namedCoordinateSystems.end());
}

Scene *RenderOptions::MakeScene() {
    std::shared_ptr<Primitive> accelerator =
            MakeAccelerator(AcceleratorName, std::move(primitives), AcceleratorParams);
    if (!accelerator) accelerator = std::make_shared<BVH>(primitives);
    Scene *scene = new Scene(accelerator, lights);
    // Erase primitives and lights from _RenderOptions_
    primitives.clear();
    lights.clear();
    return scene;
}

Integrator *RenderOptions::MakeIntegrator() const {
    std::shared_ptr<const Camera> camera(MakeCamera());
    if (!camera) {
        Error("Unable to create camera");
        return nullptr;
    }

    std::shared_ptr<Sampler> sampler =
            MakeSampler(SamplerName, SamplerParams, camera->film);
    if (!sampler) {
        Error("Unable to create sampler.");
        return nullptr;
    }

    Integrator *integrator = nullptr;

    if (IntegratorName == "whitted")
         integrator = CreateWhittedIntegrator(IntegratorParams, sampler, camera);
    else
     if (IntegratorName == "path")
        integrator = CreatePathIntegrator(IntegratorParams, sampler, camera);
    else {
        Error("Integrator \"%s\" unknown.", IntegratorName.c_str());
        return nullptr;
    }

    if (renderOptions->haveScatteringMedia && IntegratorName != "path" &&IntegratorName != "volpath") {
        Warning(
                "Scene has scattering media but \"%s\" integrator doesn't support "
                "volume scattering. Consider using \"volpath\", \"bdpt\", or "
                "\"mlt\".", IntegratorName.c_str());
    }

    IntegratorParams.ReportUnused();
    // Warn if no light sources are defined
    if (lights.empty())
        Warning(
                "No light sources defined in scene; "
                "rendering a black image.");
    return integrator;
}

Camera *RenderOptions::MakeCamera() const {
    std::unique_ptr<Filter> filter = MakeFilter(FilterName, FilterParams);
    Film *film = MakeFilm(FilmName, FilmParams, std::move(filter));
    if (!film) {
        Error("Unable to create film.");
        return nullptr;
    }
    Camera *camera = ::MakeCamera(CameraName, CameraParams, CameraToWorld,
                                      renderOptions->transformStartTime,
                                      renderOptions->transformEndTime, film);
    return camera;
}
