
/* #include <glm/vec3.hpp> */
#include <mathfu/vector.h>


/* struct Vec2 */
/* { */
/*   Vec2() = default; */
/*   Vec2(float x, float y) : x(x), y(y) {}; */
/*   Vec2(int x, int y) : x(x), y(y) {}; */
/*   float x,y; */
/*   void  setBoth(int x, int y) */
/*   { */
/*     x = x; */
/*     y = y; */
/*   } */
/*   static constexpr const char* kName = "Vec2"; */
/* }; */


#ifndef RECT_H__
#define RECT_H__

/* using Vec2 = mathfu::vec2; */
using Vec2 = mathfu::Vector<float, 2>;

struct Color
{
    uint8_t r, g, b;
    Color() = default;
    Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {};
};

struct Material
{
    struct Color color;
    Material() = default;
    Material(Color c) : color(c) {};
    static Material black() { return Material(Color(0, 0, 0)); }
    static Material blue() { return Material(Color(0, 0, 255)); }
    static Material red() { return Material(Color(255, 0, 0)); }
    static Material yellow() { return Material(Color(255, 255, 0)); }
};



struct Rectangle
{
    Rectangle() = default;
    Rectangle(Vec2 position, Vec2 size, Color color)
        : position(position), size(size), color(color) {};

    Vec2 position;
    Vec2 size;
    struct Color color;

    const char* name;
    unsigned id;
};

#endif
