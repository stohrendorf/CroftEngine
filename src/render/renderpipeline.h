#pragma once

#include "rendersettings.h"

#include <chrono>
#include <gl/fencesync.h>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <vector>

namespace render::material
{
class MaterialManager;
}

namespace render::scene
{
class Camera;
}

namespace engine::world
{
struct Room;
}

namespace render::pass
{
class PortalPass;
class GeometryPass;
class HBAOPass;
class EdgeDetectionPass;
class WorldCompositionPass;
class UIPass;
template<typename TPixel>
class EffectPass;
} // namespace render::pass

namespace render
{
class RenderPipeline
{
private:
  const std::chrono::high_resolution_clock::time_point m_creationTime = std::chrono::high_resolution_clock::now();

  RenderSettings m_renderSettings{};
  glm::ivec2 m_renderSize{-1};
  glm::ivec2 m_uiSize{-1};
  glm::ivec2 m_displaySize{-1};
  std::shared_ptr<pass::PortalPass> m_portalPass;
  std::shared_ptr<pass::GeometryPass> m_geometryPass;
  std::shared_ptr<pass::HBAOPass> m_hbaoPass;
  std::shared_ptr<pass::EdgeDetectionPass> m_edgePass;
  std::shared_ptr<pass::WorldCompositionPass> m_worldCompositionPass;
  std::shared_ptr<pass::UIPass> m_uiPass;
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>> m_backbufferTextureHandle;
  std::shared_ptr<gl::Framebuffer> m_backbuffer;
  std::unique_ptr<gl::FenceSync> m_backbufferSync;

  std::vector<gslu::nn_shared<pass::EffectPass<gl::SRGB8>>> m_effects{};
  std::vector<gslu::nn_shared<pass::EffectPass<gl::SRGB8>>> m_backbufferEffects{};

  void initBackbufferEffects(material::MaterialManager& materialManager);
  void initWorldEffects(material::MaterialManager& materialManager);

public:
  explicit RenderPipeline(material::MaterialManager& materialManager,
                          const glm::ivec2& renderViewport,
                          const glm::ivec2& uiViewport,
                          const glm::ivec2& displayViewport);

  void bindGeometryFrameBuffer(float farPlane);
  [[nodiscard]] gl::RenderState bindPortalFrameBuffer();
  void renderUiFrameBuffer(const std::function<void()>& doRender);
  void renderUiFrameBuffer(float alpha);
  void worldCompositionPass(const std::vector<engine::world::Room>& rooms, bool inWater);

  void updateCamera(const gslu::nn_shared<scene::Camera>& camera);

  void resize(material::MaterialManager& materialManager,
              const glm::ivec2& renderViewport,
              const glm::ivec2& uiViewport,
              const glm::ivec2& displayViewport,
              bool force = false);

  void apply(const RenderSettings& renderSettings, material::MaterialManager& materialManager);

  [[nodiscard]] auto getLocalTime() const
  {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()
                                                                 - m_creationTime);
  }

  void clearBackbuffer();
  void withBackbuffer(const std::function<void()>& doRender);

  const auto& getBackbuffer() const
  {
    return m_backbuffer;
  }

  void renderBackbufferEffects();
};
} // namespace render
