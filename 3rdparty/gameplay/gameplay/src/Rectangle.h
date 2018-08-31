#pragma once

namespace gameplay
{
struct Point
{
    float x = 0;
    float y = 0;
};


class Rectangle
{
public:
    float x = 0;

    float y = 0;

    float width = 0;

    float height = 0;

    Rectangle() = default;

    Rectangle(float width, float height)
            : x{0}
            , y{0}
            , width{width}
            , height{height}
    {
    }

    Rectangle(float x, float y, float width, float height)
            : x{x}
            , y{y}
            , width{width}
            , height{height}
    {
    }

    Rectangle(const Rectangle& copy)
            : x{copy.x}
            , y{copy.y}
            , width{copy.width}
            , height{copy.height}
    {
    }

    ~Rectangle() = default;

    bool isEmpty() const
    {
        return width == 0 && height == 0;
    }

    Rectangle& operator=(const Rectangle& r)
    {
        x = r.x;
        y = r.y;
        width = r.width;
        height = r.height;
        return *this;
    }

    bool operator==(const Rectangle& r) const
    {
        return (x == r.x && width == r.width && y == r.y && height == r.height);
    }

    bool operator!=(const Rectangle& r) const
    {
        return !(*this == r);
    }
};
}
