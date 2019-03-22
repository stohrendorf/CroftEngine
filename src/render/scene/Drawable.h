#pragma once

namespace render
{
namespace gl
{
class RenderState;
}

namespace scene
{
class RenderContext;


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

    virtual render::gl::RenderState& getRenderState() = 0;
};
}
}
