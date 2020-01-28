#pragma once

#include "renderable.h"
#include "renderer.h"

#include <gl/image.h>
#include <gl/texture2d.h>
#include <gl/window.h>
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

  explicit ScreenOverlay(ShaderManager& shaderManager, const glm::ivec2& viewport);

  void init(ShaderManager& shaderManager, const glm::ivec2& viewport);

  ~ScreenOverlay() override;

  void render(RenderContext& context) override;

  [[nodiscard]] const auto& getImage() const
  {
    return m_image;
  }

  [[nodiscard]] const auto& getTexture() const
  {
    return m_texture;
  }

private:
  const std::shared_ptr<gl::Image<gl::SRGBA8>> m_image{std::make_shared<gl::Image<gl::SRGBA8>>()};
  std::shared_ptr<gl::Texture2D<gl::SRGBA8>> m_texture;
  std::shared_ptr<Mesh> m_mesh{nullptr};
};
} // namespace render::scene
