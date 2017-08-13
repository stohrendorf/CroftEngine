#include "Rectangle.h"


namespace gameplay
{
    Rectangle::Rectangle()
        : x(0)
        , y(0)
        , width(0)
        , height(0)
    {
    }


    Rectangle::Rectangle(float width, float height)
        : x{ 0 }
        , y{ 0 }
        , width{ width }
        , height{ height }
    {
    }


    Rectangle::Rectangle(float x, float y, float width, float height)
        : x{ x }
        , y{ y }
        , width{ width }
        , height{ height }
    {
    }


    Rectangle::Rectangle(const Rectangle& copy)
        : x{copy.x}
        , y{copy.y}
        , width{copy.width}
        , height{copy.height}
    {
    }


    bool Rectangle::isEmpty() const
    {
        return width == 0 && height == 0;
    }


    Rectangle& Rectangle::operator =(const Rectangle& r)
    {
        x = r.x;
        y = r.y;
        width = r.width;
        height = r.height;
        return *this;
    }


    bool Rectangle::operator ==(const Rectangle& r) const
    {
        return (x == r.x && width == r.width && y == r.y && height == r.height);
    }


    bool Rectangle::operator !=(const Rectangle& r) const
    {
        return !(*this == r);
    }
}
