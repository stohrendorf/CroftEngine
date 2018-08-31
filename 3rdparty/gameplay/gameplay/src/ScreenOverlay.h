#pragma once

#include "Drawable.h"
#include "Game.h"
#include "gl/image.h"
#include "Mesh.h"

#include "gl/texture.h"

#include <memory>

namespace gameplay
{
class ScreenOverlay : public Drawable
{
public:
    ScreenOverlay(const ScreenOverlay&) = delete;

    ScreenOverlay& operator=(const ScreenOverlay&) = delete;

    explicit ScreenOverlay(const Rectangle& viewport);

    ~ScreenOverlay() override;

    void draw(RenderContext& context) override;

    const gsl::not_null<std::shared_ptr<gl::Image<gl::RGBA8>>>& getImage() const
    {
        return m_image;
    }

    RenderState& getRenderState() override
    {
        return m_renderState;
    }

private:
    RenderState m_renderState;

    gsl::not_null<std::shared_ptr<gl::Image<gl::RGBA8>>> m_image;

    gsl::not_null<std::shared_ptr<gl::Texture>> m_texture{std::make_shared<gl::Texture>( GL_TEXTURE_2D )};

    std::shared_ptr<Mesh> m_mesh{nullptr};

    gsl::not_null<std::shared_ptr<Model>> m_model{std::make_shared<Model>()};
};
}
