#pragma once

namespace gameplay
{
class RenderContext;


class RenderState;


class Drawable
{
public:
    explicit Drawable() = default;

    Drawable(const Drawable&) = delete;

    Drawable(Drawable&&) = delete;

    Drawable& operator=(Drawable&&) = delete;

    Drawable& operator=(const Drawable&) = delete;

    virtual ~Drawable() = default;

    virtual void draw(RenderContext& context) = 0;

    virtual RenderState& getRenderState() = 0;
};
}
