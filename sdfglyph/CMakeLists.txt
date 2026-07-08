cmake_minimum_required (VERSION 3.10)
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
project(font-maker)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g")
set(CMAKE_CXX_FLAGS "-Wno-c++11-narrowing")

find_package(Boost 1.73 REQUIRED)
find_package(Freetype REQUIRED)

include_directories(vendor/cxxopts/include)
include_directories(vendor/filesystem/include)
include_directories(vendor/sdf-glyph-foundry/include)
include_directories(vendor/protozero/include)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${FREETYPE_INCLUDE_DIRS})

set(SDFGLYPH_SHARED_SOURCES
    src/glyph_encode.cpp
    src/fontstack.cpp
    src/unicode_range.cpp
    src/complex_range.cpp)

add_executable(font-maker main.cpp ${SDFGLYPH_SHARED_SOURCES})
target_link_libraries(font-maker ${FREETYPE_LIBRARIES})
set_property(TARGET font-maker PROPERTY CXX_STANDARD 17)

add_executable(complex-font-maker complex-scripts.cpp ${SDFGLYPH_SHARED_SOURCES})
target_link_libraries(complex-font-maker ${FREETYPE_LIBRARIES})
set_property(TARGET complex-font-maker PROPERTY CXX_STANDARD 17)
