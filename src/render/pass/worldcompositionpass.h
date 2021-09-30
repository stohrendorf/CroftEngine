#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>

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

class WorldCompositionPass
{
public:
  explicit WorldCompositionPass(
    scene::MaterialManager& materialManager,
    const RenderSettings& renderSettings,
    const glm::ivec2& viewport,
    const GeometryPass& geometryPass,
    const PortalPass& portalPass,
    const HBAOPass& hbaoPass,
    const gsl::not_null<std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>>>& colorBuffer);

  void updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera);

  void render(bool inWater, const RenderSettings& renderSettings);

private:
  gsl::not_null<std::shared_ptr<scene::Material>> m_noWaterMaterial;
  gsl::not_null<std::shared_ptr<scene::Material>> m_inWaterMaterial;

  gsl::not_null<std::shared_ptr<scene::Mesh>> m_noWaterMesh;
  gsl::not_null<std::shared_ptr<scene::Mesh>> m_inWaterMesh;
  gsl::not_null<std::shared_ptr<scene::Mesh>> m_crtMesh;
  gsl::not_null<std::shared_ptr<gl::Texture2D<gl::SRGB8>>> m_colorBuffer;
  gsl::not_null<std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>>> m_colorBufferHandle;
  gsl::not_null<std::shared_ptr<gl::Framebuffer>> m_fb;
};
} // namespace render::pass
