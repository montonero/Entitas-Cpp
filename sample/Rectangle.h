#pragma once

#ifndef RECT_H__
#define RECT_H__

#include <SDLpp.h>
#include <mathfu/vector.h>

/* using Vec2 = mathfu::vec2; */
using Vec2 = mathfu::Vector<float, 2>;

struct Material {
    sdl::Color color;
    Material() = default;
    Material(sdl::Color c)
        : color(c){};
    static Material black() { return Material(sdl::Color(0, 0, 0)); }
    static Material blue() { return Material(sdl::Color(0, 0, 255)); }
    static Material red() { return Material(sdl::Color(255, 0, 0)); }
    static Material yellow() { return Material(sdl::Color(255, 255, 0)); }
};

struct Rectangle {
    Rectangle() = default;
    Rectangle(Vec2 position, Vec2 size, sdl::Color color)
        : position(position)
        , size(size)
        , color(color){};

    Vec2 position;
    Vec2 size;
    sdl::Color color;

    const char* name;
    unsigned id;
};

#endif
