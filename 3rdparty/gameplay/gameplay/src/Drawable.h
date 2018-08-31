#pragma once

namespace gameplay
{
class RenderContext;


class RenderState;


class Drawable
{
public:
    explicit Drawable() = default;

    virtual ~Drawable() = default;

    virtual void draw(RenderContext& context) = 0;

    virtual RenderState& getRenderState() = 0;
};
}
