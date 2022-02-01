#pragma once

#include "render/scene/blur.h"

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>

namespace render::scene
{
class MaterialManager;
class Camera;
class Material;
class Mesh;
} // namespace render::scene

namespace render::pass
{
class GeometryPass;

class HBAOPass
{
public:
  explicit HBAOPass(scene::MaterialManager& materialManager,
                    const glm::ivec2& viewport,
                    const GeometryPass& geometryPass);
  void updateCamera(const gslu::nn_shared<scene::Camera>& camera);

  void render();

  [[nodiscard]] auto getBlurredTexture() const
  {
    return m_blur.getBlurredTexture();
  }

private:
  const gslu::nn_shared<scene::Material> m_material;

  gslu::nn_shared<scene::Mesh> m_renderMesh;

  gslu::nn_shared<gl::Texture2D<gl::ScalarByte>> m_aoBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::ScalarByte>>> m_aoBufferHandle;
  scene::SeparableBlur<gl::ScalarByte> m_blur;
  gslu::nn_shared<gl::Framebuffer> m_fb;
};
} // namespace render::pass
