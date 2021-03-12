#pragma once

#include "rendersettings.h"
#include "scene/blur.h"
#include "scene/camera.h"
#include "scene/material.h"
#include "scene/rendercontext.h"

#include <chrono>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <memory>

namespace render
{
namespace scene
{
class ShaderManager;
class MaterialManager;
} // namespace scene

namespace pass
{
class PortalPass;
class GeometryPass;
class SSAOPass;
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
  std::shared_ptr<pass::SSAOPass> m_ssaoPass;
  std::shared_ptr<pass::FXAAPass> m_fxaaPass;
  std::shared_ptr<pass::CompositionPass> m_compositionPass;
  std::shared_ptr<pass::UIPass> m_uiPass;

public:
  explicit RenderPipeline(scene::MaterialManager& materialManager, const glm::ivec2& viewport);

  void bindGeometryFrameBuffer(const glm::ivec2& size);
  void bindPortalFrameBuffer();
  void bindUiFrameBuffer();
  void renderUiFrameBuffer(float alpha);
  void compositionPass(bool water);

  void updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera);

  void resize(scene::MaterialManager& materialManager, const glm::ivec2& viewport, bool force = false);

  void apply(const RenderSettings& renderSettings, scene::MaterialManager& materialManager);
};
} // namespace render
