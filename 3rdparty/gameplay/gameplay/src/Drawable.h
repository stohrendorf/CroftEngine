#pragma once

namespace gameplay
{
class RenderContext;


class Drawable
{
public:
    explicit Drawable() = default;

    virtual ~Drawable() = default;

    virtual void draw(RenderContext& context) = 0;
};
}
