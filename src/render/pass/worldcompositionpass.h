#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
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
class GeometryPass;

class WorldCompositionPass
{
public:
  explicit WorldCompositionPass(scene::MaterialManager& materialManager,
                                const RenderSettings& renderSettings,
                                const glm::ivec2& viewport,
                                const GeometryPass& geometryPass,
                                const PortalPass& portalPass);

  void updateCamera(const gslu::nn_shared<scene::Camera>& camera);

  void render(bool inWater);

  [[nodiscard]] const auto& getColorBuffer() const
  {
    return m_colorBufferHandle;
  }

  [[nodiscard]] const auto& getFramebuffer() const
  {
    return m_fb;
  }

private:
  gslu::nn_shared<scene::Material> m_noWaterMaterial;
  gslu::nn_shared<scene::Material> m_inWaterMaterial;

  gslu::nn_shared<scene::Mesh> m_noWaterMesh;
  gslu::nn_shared<scene::Mesh> m_inWaterMesh;
  gslu::nn_shared<gl::Texture2D<gl::SRGB8>> m_colorBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>> m_colorBufferHandle;
  gslu::nn_shared<gl::Framebuffer> m_fb;
};
} // namespace render::pass
