#pragma once

#include "Drawable.h"
#include "Game.h"
#include "Mesh.h"
#include "window.h"

#include "render/gl/image.h"
#include "render/gl/texture.h"

#include <memory>

namespace render
{
namespace scene
{
class ScreenOverlay : public Drawable
{
public:
    ScreenOverlay(const ScreenOverlay&) = delete;

    ScreenOverlay(ScreenOverlay&&) = delete;

    ScreenOverlay& operator=(ScreenOverlay&&) = delete;

    ScreenOverlay& operator=(const ScreenOverlay&) = delete;

    explicit ScreenOverlay(const Dimension2<size_t>& viewport);

    void init(const Dimension2<size_t>& viewport);

    ~ScreenOverlay() override;

    void draw(RenderContext& context) override;

    gsl::not_null<std::shared_ptr<gl::Image < gl::RGBA8>>>

    getImage() const
    {
        return m_image;
    }

    render::gl::RenderState& getRenderState() override
    {
        return m_renderState;
    }

private:
    render::gl::RenderState m_renderState;

    std::shared_ptr<gl::Image<gl::RGBA8>> m_image;

    gsl::not_null<std::shared_ptr<gl::Texture>> m_texture{std::make_shared<gl::Texture>( GL_TEXTURE_2D )};

    std::shared_ptr<Mesh> m_mesh{nullptr};

    gsl::not_null<std::shared_ptr<Model>> m_model{std::make_shared<Model>()};
};
}
}
