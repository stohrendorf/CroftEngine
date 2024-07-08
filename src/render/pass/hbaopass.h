#pragma once

#include "render/scene/blur.h"

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <gslu.h>

namespace render::material
{
class MaterialManager;
class Material;
} // namespace render::material

namespace render::scene
{
class Camera;
class Mesh;
} // namespace render::scene

namespace render::pass
{
class GeometryPass;

class HBAOPass final
{
public:
  explicit HBAOPass(material::MaterialManager& materialManager,
                    const glm::ivec2& viewport,
                    const gslu::nn_shared<GeometryPass>& geometryPass);
  void updateCamera(const gslu::nn_shared<scene::Camera>& camera);

  void render();

  [[nodiscard]] auto getBlurredTexture() const
  {
    return m_blur.getBlurredTexture();
  }

private:
  gslu::nn_shared<GeometryPass> m_geometryPass;
  gslu::nn_shared<material::Material> m_material;

  gslu::nn_shared<scene::Mesh> m_renderMesh;

  gslu::nn_shared<gl::Texture2D<gl::ScalarByte>> m_aoBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::ScalarByte>>> m_aoBufferHandle;
  scene::SeparableBlur<gl::ScalarByte> m_blur;
  gslu::nn_shared<gl::Framebuffer> m_fb;
};
} // namespace render::pass
