#pragma once

#include "render/gl/renderstate.h"

namespace render
{

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

    render::gl::RenderState& getRenderState()
    {
        return m_renderState;
    }

private:
    render::gl::RenderState m_renderState;
};
}
}
