#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>

// IWYU pragma: no_forward_declare gl::Framebuffer
// IWYU pragma: no_forward_declare gl::Texture2D
// IWYU pragma: no_forward_declare gl::TextureHandle
// IWYU pragma: no_forward_declare gsl::not_null

namespace render
{
struct RenderSettings;
}

namespace render::scene
{
class Camera;
class MaterialManager;
class Material;
class Mesh;
} // namespace render::scene

namespace render::pass
{
class PortalPass;
class HBAOPass;
class GeometryPass;

class CompositionPass
{
public:
  explicit CompositionPass(scene::MaterialManager& materialManager,
                           const RenderSettings& renderSettings,
                           const glm::ivec2& viewport,
                           const GeometryPass& geometryPass,
                           const PortalPass& portalPass,
                           const HBAOPass& hbaoPass,
                           const std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>& colorBuffer);

  void updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera);

  void render(bool water, const RenderSettings& renderSettings);

private:
  std::shared_ptr<scene::Material> m_compositionMaterial;
  std::shared_ptr<scene::Material> m_waterCompositionMaterial;

  std::shared_ptr<scene::Mesh> m_mesh;
  std::shared_ptr<scene::Mesh> m_waterMesh;
  std::shared_ptr<scene::Mesh> m_crtMesh;
  std::shared_ptr<gl::Texture2D<gl::SRGBA8>> m_colorBuffer;
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>> m_colorBufferHandle;
  std::shared_ptr<gl::Framebuffer> m_fb;
};
} // namespace render::pass
