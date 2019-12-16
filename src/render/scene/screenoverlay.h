#pragma once

#include "render/gl/image.h"
#include "render/gl/texture.h"
#include "renderable.h"
#include "renderer.h"
#include "window.h"

#include <memory>

namespace render::scene
{
class Mesh;
class Model;
class ShaderManager;

class ScreenOverlay : public Renderable
{
public:
  ScreenOverlay(const ScreenOverlay&) = delete;

  ScreenOverlay(ScreenOverlay&&) = delete;

  ScreenOverlay& operator=(ScreenOverlay&&) = delete;

  ScreenOverlay& operator=(const ScreenOverlay&) = delete;

  explicit ScreenOverlay(ShaderManager& shaderManager, const Dimension2<size_t>& viewport);

  void init(ShaderManager& shaderManager, const Dimension2<size_t>& viewport);

  ~ScreenOverlay() override;

  void render(RenderContext& context) override;

  [[nodiscard]] const auto& getImage() const
  {
    return m_image;
  }

  [[nodiscard]] auto getTexture() const
  {
    return m_texture;
  }

private:
  const std::shared_ptr<gl::Image<gl::SRGBA8>> m_image{std::make_shared<gl::Image<gl::SRGBA8>>()};

  gsl::not_null<std::shared_ptr<gl::Texture2D<gl::SRGBA8>>> m_texture{std::make_shared<gl::Texture2D<gl::SRGBA8>>()};

  std::shared_ptr<Mesh> m_mesh{nullptr};
};
} // namespace render::scene
