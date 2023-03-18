#pragma once

#include <functional>
#include <gl/fencesync.h>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
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
enum class Translucency;
} // namespace render::scene

namespace render::pass
{
class Framebuffer final
{
public:
  Framebuffer(const Framebuffer&) = delete;
  Framebuffer(Framebuffer&&) = delete;
  void operator=(const Framebuffer&) = delete;
  void operator=(Framebuffer&&) = delete;

  explicit Framebuffer(const std::string& name,
                       gslu::nn_shared<material::Material> material,
                       scene::Translucency translucencySelector,
                       const glm::ivec2& size);

  void render(const std::function<void()>& doRender);

  [[nodiscard]] const auto& getOutput() const
  {
    return m_colorBufferHandle;
  }

  [[nodiscard]] const auto& getDepthBuffer() const
  {
    return m_depthBuffer;
  }

  void render();

  void noWait()
  {
    m_sync.reset();
  }

private:
  gslu::nn_shared<material::Material> m_material;
  gslu::nn_shared<scene::Mesh> m_mesh;
  gslu::nn_shared<gl::TextureDepth<float>> m_depthBuffer;
  gslu::nn_shared<gl::Texture2D<gl::SRGBA8>> m_colorBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>> m_colorBufferHandle;
  gslu::nn_shared<gl::Framebuffer> m_fb;
  scene::Translucency m_translucencySelector;
  std::unique_ptr<gl::FenceSync> m_sync;
};
} // namespace render::pass
