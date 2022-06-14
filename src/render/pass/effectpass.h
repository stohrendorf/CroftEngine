#pragma once

#include "render/scene/mesh.h"

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <string>
#include <utility>

namespace render
{
class RenderPipeline;
}

namespace render::scene
{
class Material;
class Mesh;
} // namespace render::scene

namespace render::pass
{
class EffectPass final
{
public:
  explicit EffectPass(gsl::not_null<const RenderPipeline*> renderPipeline,
                      std::string name,
                      const gslu::nn_shared<scene::Material>& material,
                      const gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>>& input);

  void render(bool inWater);

  [[nodiscard]] const auto& getOutput() const
  {
    return m_colorBufferHandle;
  }

  [[nodiscard]] const auto& getFramebuffer() const
  {
    return m_fb;
  }

  template<typename... Args>
  void bind(Args&&... args)
  {
    m_mesh->bind(std::forward<Args>(args)...);
  }

private:
  const gsl::not_null<const RenderPipeline*> m_renderPipeline;
  const std::string m_name;
  gslu::nn_shared<scene::Mesh> m_mesh;
  gslu::nn_shared<gl::Texture2D<gl::SRGB8>> m_colorBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>> m_colorBufferHandle;
  gslu::nn_shared<gl::Framebuffer> m_fb;
};
} // namespace render::pass
