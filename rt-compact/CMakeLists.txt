cmake_minimum_required(VERSION 3.10)
project(rt_compact)

set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


INCLUDE_DIRECTORIES(src)
INCLUDE_DIRECTORIES(src/core)

add_executable(rt_compact src/main/main.cpp src/core/rt_compact.h
        src/parser/parser.cpp src/parser/parser.h src/util/error.h src/core/api.cpp
        src/core/api.h src/util/fileutil.h src/core/color.cpp src/core/color.h
        src/math/transform.cpp src/math/transform.h src/math/geometry.h
        src/shape/shape.cpp src/shape/shape.h src/util/imageio.cpp src/util/imageio.h
        src/shape/sphere.cpp src/shape/sphere.h src/shape/triangle.cpp src/shape/triangle.h
        src/core/interaction.cpp src/core/interaction.h src/core/primitive.cpp src/core/primitive.h
        src/core/integrator.cpp src/core/integrator.h src/core/scene.cpp src/core/scene.h
        src/light/light.cpp src/light/light.h src/camera/camera.cpp src/camera/camera.h
        src/material/material.cpp src/material/material.h src/material/metal.cpp src/material/metal.h
        src/texture/texture.cpp src/texture/texture.h
        src/math/quaternion.cpp src/math/quaternion.h src/camera/film.cpp src/camera/film.h
        src/core/prob.cpp src/core/prob.h src/camera/perspective.cpp src/camera/perspective.h
        src/integrator/path.cpp src/integrator/path.h src/parser/paramset.cpp
        src/parser/paramset.h src/material/glass.cpp src/material/glass.h src/util/error.cpp
        src/sampler/sampler.cpp src/sampler/sampler.h src/material/matte.cpp src/material/matte.h
        src/medium/medium.cpp src/medium/medium.h src/core/memory.cpp src/core/memory.h
        src/util/randgen.h src/texture/constant.cpp src/texture/constant.h
        src/texture/checkboard.cpp src/texture/checkboard.h src/accelerator/bvh.cpp
        src/accelerator/bvh.h src/light/point.cpp src/light/point.h src/core/parallel.cpp
        src/core/parallel.h src/filter/filter.cpp src/filter/filter.h src/filter/box.cpp
        src/filter/box.h src/light/distant.cpp src/light/distant.h src/integrator/whitted.cpp
        src/integrator/whitted.h src/sampler/random.cpp src/sampler/random.h src/reflection/reflection.cpp
        src/reflection/reflection.h src/parser/floatfile.cpp src/parser/floatfile.h src/light/infinite.cpp
        src/light/infinite.h src/core/mipmap.h
        src/light/diffuse.cpp src/light/diffuse.h src/core/microfacet.cpp
        src/core/microfacet.h src/material/mirror.cpp src/material/mirror.h
        src/ext/rply.h src/ext/rply.cpp
        src/math/math.h src/shape/plymesh.cpp src/shape/plymesh.h src/medium/homogeneous.cpp src/medium/homogeneous.h
        src/light/spot.cpp src/light/spot.h src/texture/imagemap.cpp src/texture/imagemap.h)


target_link_libraries(rt_compact PRIVATE Threads::Threads)
