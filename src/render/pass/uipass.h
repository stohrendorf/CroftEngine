#pragma once

#include <functional>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <gslu.h>

namespace render::material
{
class Material;
class MaterialManager;
} // namespace render::material

namespace render::scene
{
class Mesh;
}

namespace render::pass
{
class UIPass
{
public:
  explicit UIPass(material::MaterialManager& materialManager,
                  const glm::ivec2& renderViewport,
                  const glm::ivec2& displayViewport);

  void render(const std::function<void()>& doRender);

  [[nodiscard]] const auto& getColorBuffer() const
  {
    return m_colorBufferHandle;
  }

  void render(float alpha);

private:
  gslu::nn_shared<material::Material> m_material;
  gslu::nn_shared<scene::Mesh> m_mesh;
  gslu::nn_shared<gl::Texture2D<gl::SRGBA8>> m_colorBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>> m_colorBufferHandle;
  gslu::nn_shared<gl::Framebuffer> m_fb;
};
} // namespace render::pass
