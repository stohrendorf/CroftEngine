#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <string>

namespace render::material
{
class Material;
}

namespace render::scene
{
class Mesh;
}

namespace render::pass
{
class Framebuffer final
{
public:
  explicit Framebuffer(const std::string& name, gslu::nn_shared<material::Material> material, const glm::ivec2& size);

  [[nodiscard]] const auto& getOutput() const
  {
    return m_colorBufferHandle;
  }

  [[nodiscard]] const auto& getDepthBuffer() const
  {
    return m_depthBuffer;
  }

  [[nodiscard]] const auto& getFramebuffer() const
  {
    return m_fb;
  }

  void bind();

  void render();

private:
  const gslu::nn_shared<material::Material> m_material;
  gslu::nn_shared<scene::Mesh> m_mesh;
  gslu::nn_shared<gl::TextureDepth<float>> m_depthBuffer;
  gslu::nn_shared<gl::Texture2D<gl::SRGBA8>> m_colorBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>> m_colorBufferHandle;
  gslu::nn_shared<gl::Framebuffer> m_fb;
};
} // namespace render::pass
