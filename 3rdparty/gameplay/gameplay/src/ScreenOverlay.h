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

    const std::shared_ptr<gl::Image<gl::RGBA8>>& getImage() const
    {
        return m_image;
    }

private:
    std::shared_ptr<gl::Image<gl::RGBA8>> m_image{nullptr};

    std::shared_ptr<gl::Texture> m_texture{nullptr};

    std::shared_ptr<Mesh> m_mesh{nullptr};

    std::shared_ptr<Model> m_model{nullptr};
};
}
