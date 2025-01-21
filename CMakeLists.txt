cmake_minimum_required(VERSION 3.14)

find_package(SDL2 REQUIRED)

include("cmake/macros.cmake")

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD)")
endif()

project(retrofiller C)

setup_compiler()

include_directories(${PROJECT_NAME} src sdl)

set(SRC
    src/bmark.c
    src/bmark.h
    src/drawtris.c
    src/drawtris.h
    src/minmax.h
    src/mymath.h
    src/mytime.h
    src/screen.h
    src/tridefs.h
    src/tritmpl.h
    src/tritmpl2.h
    src/util.h

    sdl/sdlmain.c
    sdl/sdltime.c
)

add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SRC})
setup_target(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
