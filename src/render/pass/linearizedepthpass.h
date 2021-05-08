#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <gl/texturedepth.h>
#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>

namespace render::scene
{
class Camera;
class MaterialManager;
class ShaderProgram;
class Material;
class Mesh;
} // namespace render::scene

namespace render::pass
{
class LinearizeDepthPass
{
public:
  explicit LinearizeDepthPass(scene::MaterialManager& materialManager,
                              const glm::ivec2& viewport,
                              const std::shared_ptr<gl::TextureHandle<gl::TextureDepth<float>>>& depth);

  void render();

  [[nodiscard]] const auto& getTexture() const
  {
    return m_linearDepth;
  }

  void updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera);

private:
  const std::shared_ptr<scene::Material> m_material;
  std::shared_ptr<scene::Mesh> m_renderMesh;
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::Scalar32F>>> m_linearDepth;
  std::shared_ptr<gl::Framebuffer> m_fb;
};
} // namespace render::pass
