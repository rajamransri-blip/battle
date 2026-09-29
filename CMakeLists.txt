cmake_minimum_required(VERSION 3.22)

project(XReal3D LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

include(FetchContent)

FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(raylib)

file(GLOB_RECURSE GAME_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_library(main SHARED ${GAME_SOURCES})

target_include_directories(main PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(main
    raylib
    android
    log
    EGL
    GLESv2
    OpenSLES
    dl
)

target_compile_definitions(main PRIVATE
    PLATFORM_ANDROID
    GRAPHICS_API_OPENGL_ES2
)

set_target_properties(main PROPERTIES
    OUTPUT_NAME "main"
)

# Copy game assets into Android asset storage.
add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    "${CMAKE_BINARY_DIR}/android_assets"
)

add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "${CMAKE_BINARY_DIR}/android_assets/assets"
)
