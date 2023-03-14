#pragma once

#include "render/pass/effectpass.h"
#include "render/scene/blur.h"

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
class PortalPass;
class GeometryPass;
template<uint8_t NSteps, typename TPixel>
class BloomPass;

class WorldCompositionPass
{
public:
  explicit WorldCompositionPass(material::MaterialManager& materialManager,
                                const RenderSettings& renderSettings,
                                const glm::ivec2& viewport,
                                const gslu::nn_shared<GeometryPass>& geometryPass,
                                const gslu::nn_shared<PortalPass>& portalPass);

  void updateCamera(const gslu::nn_shared<scene::Camera>& camera);

  void render(bool inWater, bool denoiseWater);

  [[nodiscard]] const auto& getColorBuffer() const
  {
    return m_bloom ? m_bloomedBufferHandle : m_colorBufferHandle;
  }

  [[nodiscard]] const auto& getFramebuffer() const
  {
    return m_bloom ? m_fbBloom : m_fb;
  }

private:
  gslu::nn_shared<GeometryPass> m_geometryPass;
  gslu::nn_shared<PortalPass> m_portalPass;
  gslu::nn_shared<material::Material> m_noWaterMaterial;
  gslu::nn_shared<material::Material> m_inWaterMaterial;

  gslu::nn_shared<scene::Mesh> m_noWaterMesh;
  gslu::nn_shared<scene::Mesh> m_inWaterMesh;
  gslu::nn_shared<scene::Mesh> m_bloomMesh;
  gslu::nn_shared<gl::Texture2D<gl::SRGB8>> m_colorBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>> m_colorBufferHandle;
  gslu::nn_shared<gl::Texture2D<gl::SRGB8>> m_bloomedBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>> m_bloomedBufferHandle;
  gslu::nn_shared<gl::Framebuffer> m_fb;

  std::shared_ptr<BloomPass<5, gl::SRGB8>> m_bloomPass;

  gslu::nn_shared<gl::Framebuffer> m_fbBloom;
  const bool m_bloom;
};
} // namespace render::pass
