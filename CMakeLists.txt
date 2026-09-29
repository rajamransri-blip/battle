cmake_minimum_required(VERSION 3.22)

project(XBattle C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(PLATFORM "Android" CACHE STRING "" FORCE)
set(GRAPHICS "GRAPHICS_API_OPENGL_ES2" CACHE STRING "" FORCE)

include(FetchContent)

FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(raylib)

file(
    GLOB_RECURSE
    GAME_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_library(
    main
    SHARED
    ${GAME_SOURCES}
)

target_include_directories(
    main
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_definitions(
    main
    PRIVATE
    PLATFORM_ANDROID
    GRAPHICS_API_OPENGL_ES2
)

target_compile_options(
    main
    PRIVATE
    -O3
    -ffast-math
    -fdata-sections
    -ffunction-sections
)

target_link_options(
    main
    PRIVATE
    -Wl,--gc-sections
    -Wl,--strip-all
    -Wl,-u,ANativeActivity_onCreate
)

target_link_libraries(
    main
    raylib
    android
    log
    EGL
    GLESv2
    OpenSLES
    dl
    m
)

set_target_properties(
    main
    PROPERTIES
    OUTPUT_NAME "main"
)
