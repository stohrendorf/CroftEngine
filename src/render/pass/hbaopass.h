#pragma once

#include "render/scene/blur.h"

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>

// IWYU pragma: no_forward_declare gl::Framebuffer
// IWYU pragma: no_forward_declare gl::Texture2D
// IWYU pragma: no_forward_declare gl::TextureHandle
// IWYU pragma: no_forward_declare gsl::not_null

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
  void updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera);

  void render(const glm::ivec2& size);

  [[nodiscard]] auto getBlurredTexture() const
  {
    return m_blur.getBlurredTexture();
  }

private:
  const gsl::not_null<std::shared_ptr<scene::Material>> m_material;

  gsl::not_null<std::shared_ptr<scene::Mesh>> m_renderMesh;

  gsl::not_null<std::shared_ptr<gl::Texture2D<gl::ScalarByte>>> m_aoBuffer;
  gsl::not_null<std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::ScalarByte>>>> m_aoBufferHandle;
  gsl::not_null<std::shared_ptr<gl::Framebuffer>> m_fb;

  scene::SeparableBlur<gl::ScalarByte> m_blur;
};
} // namespace render::pass
