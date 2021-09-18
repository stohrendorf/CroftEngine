#pragma once

#include "rendersettings.h"

#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>

// IWYU pragma: no_forward_declare gsl::not_null

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
class CompositionPass;
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
  std::shared_ptr<pass::CompositionPass> m_compositionPass;
  std::shared_ptr<pass::UIPass> m_uiPass;

public:
  explicit RenderPipeline(scene::MaterialManager& materialManager, const glm::ivec2& viewport);

  void bindGeometryFrameBuffer(const glm::ivec2& size, float farPlane);
  void bindPortalFrameBuffer();
  void bindUiFrameBuffer();
  void renderUiFrameBuffer(float alpha);
  void compositionPass(bool water);

  void updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera);

  void resize(scene::MaterialManager& materialManager, const glm::ivec2& viewport, bool force = false);

  void apply(const RenderSettings& renderSettings, scene::MaterialManager& materialManager);
};
} // namespace render
