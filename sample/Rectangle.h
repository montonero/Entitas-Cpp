
struct Vec2
{
  Vec2() = default;
  Vec2(float x, float y) : x(x), y(y) {};
  Vec2(int x, int y) : x(x), y(y) {};
  float x,y;
  void  setBoth(int x, int y)
  {
    x = x;
    y = y;
  }
  static constexpr const char* kName = "Vec2";
};

struct Color
{
    uint8_t r, g, b;
    Color() = default;
    Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {};
};

struct Material
{
  struct Color color;
};

struct Rectangle
{
    Rectangle() = default;
    Rectangle(Vec2 position, Vec2 size, Color color)
        : position(position), size(size), color(color) {};

    struct Vec2 position;
    struct Vec2 size;
    struct Color color;

    const char* name;
    unsigned id;
};
