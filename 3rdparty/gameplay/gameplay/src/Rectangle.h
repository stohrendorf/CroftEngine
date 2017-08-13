#pragma once

namespace gameplay
{
    class Rectangle
    {
    public:
        float x;

        float y;

        float width;

        float height;

        Rectangle();

        Rectangle(float width, float height);

        Rectangle(float x, float y, float width, float height);

        Rectangle(const Rectangle& copy);

        ~Rectangle() = default;

        bool isEmpty() const;

        Rectangle& operator =(const Rectangle& r);

        bool operator ==(const Rectangle& r) const;

        bool operator !=(const Rectangle& r) const;
    };
}
