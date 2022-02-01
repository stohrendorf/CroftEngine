#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>

namespace render::scene
{
class Mesh;
class Material;
class MaterialManager;
} // namespace render::scene

namespace render::pass
{
class UIPass
{
public:
  explicit UIPass(scene::MaterialManager& materialManager,
                  const glm::ivec2& renderViewport,
                  const glm::ivec2& displayViewport);
  void bind();

  [[nodiscard]] const auto& getColorBuffer() const
  {
    return m_colorBufferHandle;
  }

  void render(float alpha);

private:
  const gslu::nn_shared<scene::Material> m_material;
  gslu::nn_shared<scene::Mesh> m_mesh;
  gslu::nn_shared<gl::Texture2D<gl::SRGBA8>> m_colorBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>> m_colorBufferHandle;
  gslu::nn_shared<gl::Framebuffer> m_fb;
};
} // namespace render::pass
