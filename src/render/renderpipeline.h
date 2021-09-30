#pragma once

#include "rendersettings.h"

#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <memory>

namespace gsl
{
template<class T>
class not_null;
}

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
class FXAAPass;
class WorldCompositionPass;
class UIPass;
} // namespace pass

class RenderPipeline
{
private:
  RenderSettings m_renderSettings{};
  glm::ivec2 m_size{-1};
  std::shared_ptr<pass::PortalPass> m_portalPass;
  std::shared_ptr<pass::GeometryPass> m_geometryPass;
  std::shared_ptr<pass::HBAOPass> m_hbaoPass;
  std::shared_ptr<pass::FXAAPass> m_fxaaPass;
  std::shared_ptr<pass::WorldCompositionPass> m_worldCompositionPass;
  std::shared_ptr<pass::UIPass> m_uiPass;

public:
  explicit RenderPipeline(scene::MaterialManager& materialManager, const glm::ivec2& viewport);

  void bindGeometryFrameBuffer(float farPlane);
  [[nodiscard]] gl::RenderState bindPortalFrameBuffer();
  void bindUiFrameBuffer();
  void renderUiFrameBuffer(float alpha);
  void worldCompositionPass(bool water);

  void updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera);

  void resize(scene::MaterialManager& materialManager, const glm::ivec2& viewport, bool force = false);

  void apply(const RenderSettings& renderSettings, scene::MaterialManager& materialManager);
};
} // namespace render
