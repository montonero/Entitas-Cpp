#pragma once

#ifndef RECT_H__
#define RECT_H__

#include <mathfu/vector.h>

/* using Vec2 = mathfu::vec2; */
using Vec2 = mathfu::Vector<float, 2>;

struct Color {
    uint8_t r, g, b;
    Color() = default;
    Color(uint8_t r, uint8_t g, uint8_t b)
        : r(r)
        , g(g)
        , b(b){};
};

struct Material {
    struct Color color;
    Material() = default;
    Material(Color c)
        : color(c){};
    static Material black() { return Material(Color(0, 0, 0)); }
    static Material blue() { return Material(Color(0, 0, 255)); }
    static Material red() { return Material(Color(255, 0, 0)); }
    static Material yellow() { return Material(Color(255, 255, 0)); }
};

struct Rectangle {
    Rectangle() = default;
    Rectangle(Vec2 position, Vec2 size, Color color)
        : position(position)
        , size(size)
        , color(color){};

    Vec2 position;
    Vec2 size;
    struct Color color;

    const char* name;
    unsigned id;
};

#endif
