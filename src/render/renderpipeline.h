#pragma once

#include "rendersettings.h"

#include <chrono>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <vector>

namespace render::scene
{
class MaterialManager;
class Camera;
} // namespace render::scene

namespace render
{
namespace pass
{
class PortalPass;
class GeometryPass;
class HBAOPass;
class WorldCompositionPass;
class UIPass;
class EffectPass;
} // namespace pass

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
  std::shared_ptr<pass::WorldCompositionPass> m_worldCompositionPass;
  std::shared_ptr<pass::UIPass> m_uiPass;

  std::vector<gsl::not_null<std::shared_ptr<pass::EffectPass>>> m_effects{};

public:
  explicit RenderPipeline(scene::MaterialManager& materialManager,
                          const glm::ivec2& renderViewport,
                          const glm::ivec2& uiViewport,
                          const glm::ivec2& displayViewport);

  void bindGeometryFrameBuffer(float farPlane);
  [[nodiscard]] gl::RenderState bindPortalFrameBuffer();
  void bindUiFrameBuffer();
  void renderUiFrameBuffer(float alpha);
  void worldCompositionPass(bool inWater);

  void updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera);

  void resize(scene::MaterialManager& materialManager,
              const glm::ivec2& renderViewport,
              const glm::ivec2& uiViewport,
              const glm::ivec2& displayViewport,
              bool force = false);

  void apply(const RenderSettings& renderSettings, scene::MaterialManager& materialManager);

  [[nodiscard]] auto getLocalTime() const
  {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()
                                                                 - m_creationTime);
  }
};
} // namespace render
