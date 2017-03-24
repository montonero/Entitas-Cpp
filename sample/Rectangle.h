#pragma once

#ifndef RECT_H__
#define RECT_H__

#include <SDLpp.h>
#include <mathfu/vector.h>

using Vec2 = mathfu::Vector<float, 2>;

struct Material {
    sdl::Color color;
    std::shared_ptr<sdl::Texture> texture;
    Material() = default;
    Material(sdl::Color c)
        : color(c){};
    static Material black() { return Material(sdl::Color(0, 0, 0)); }
    static Material blue() { return Material(sdl::Color(0, 0, 255)); }
    static Material red() { return Material(sdl::Color(255, 0, 0)); }
    static Material yellow() { return Material(sdl::Color(255, 255, 0)); }
};


#endif
