#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/fwd.hpp>

namespace render::scene
{
class Mesh;
class ShaderProgram;
class Material;
class ShaderManager;
} // namespace render::scene

namespace render::pass
{
class UIPass
{
public:
  explicit UIPass(scene::ShaderManager& shaderManager, const glm::ivec2& viewport);
  void bind();

  [[nodiscard]] const auto& getColorBuffer() const
  {
    return m_colorBuffer;
  }

  void render(float alpha);

private:
  const std::shared_ptr<scene::ShaderProgram> m_shader;
  const std::shared_ptr<scene::Material> m_material;
  std::shared_ptr<scene::Mesh> m_mesh;
  std::shared_ptr<gl::Texture2D<gl::SRGBA8>> m_colorBuffer;
  std::shared_ptr<gl::Framebuffer> m_fb;
};
} // namespace render::pass
