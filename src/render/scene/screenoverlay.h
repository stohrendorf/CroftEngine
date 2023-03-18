#pragma once

#include "renderable.h"

#include <algorithm>
#include <gl/image.h>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/fwd.hpp>
#include <gslu.h>
#include <memory>

namespace render::material
{
class MaterialManager;
}

namespace render::scene
{
class Mesh;
class Node;
class RenderContext;

class ScreenOverlay : public Renderable
{
public:
  ScreenOverlay(const ScreenOverlay&) = delete;
  ScreenOverlay(ScreenOverlay&&) = delete;
  ScreenOverlay& operator=(ScreenOverlay&&) = delete;
  ScreenOverlay& operator=(const ScreenOverlay&) = delete;

  explicit ScreenOverlay();

  void init(material::MaterialManager& materialManager, const glm::ivec2& viewport);

  ~ScreenOverlay() override;

  void render(const Node* node, RenderContext& context) final;
  void render(const Node* node, RenderContext& context, gl::api::core::SizeType instanceCount) override;

  [[nodiscard]] const auto& getImage() const
  {
    return m_image;
  }

  void setAlphaMultiplier(float value)
  {
    m_alphaMultiplier = value;
  }

private:
  const gslu::nn_shared<gl::Image<gl::PremultipliedSRGBA8>> m_image{
    std::make_shared<gl::Image<gl::PremultipliedSRGBA8>>()};
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::PremultipliedSRGBA8>>> m_texture;
  std::shared_ptr<Mesh> m_mesh{nullptr};
  float m_alphaMultiplier{1.0f};
};
} // namespace render::scene
