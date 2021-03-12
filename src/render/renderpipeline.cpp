#include "renderpipeline.h"

#include "pass/compositionpass.h"
#include "pass/fxaapass.h"
#include "pass/geometrypass.h"
#include "pass/portalpass.h"
#include "pass/ssaopass.h"
#include "pass/uipass.h"
#include "scene/materialmanager.h"
#include "scene/mesh.h"
#include "scene/shadermanager.h"

#include <gl/texturedepth.h>

namespace render
{
RenderPipeline::RenderPipeline(scene::MaterialManager& materialManager, const glm::ivec2& viewport)
{
  resize(materialManager, viewport, true);
}

void RenderPipeline::compositionPass(const bool water)
{
  if(m_renderSettings.waterDenoise)
  {
    BOOST_ASSERT(m_portalPass != nullptr);
    m_portalPass->renderBlur();
  }
  BOOST_ASSERT(m_ssaoPass != nullptr);
  m_ssaoPass->render(m_size / 2);
  BOOST_ASSERT(m_fxaaPass != nullptr);
  m_fxaaPass->render(m_size);
  BOOST_ASSERT(m_compositionPass != nullptr);
  m_compositionPass->render(water, m_renderSettings);
}

void RenderPipeline::updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera)
{
  BOOST_ASSERT(m_compositionPass != nullptr);
  m_compositionPass->updateCamera(camera);
  BOOST_ASSERT(m_ssaoPass != nullptr);
  m_ssaoPass->updateCamera(camera);
}

void RenderPipeline::apply(const RenderSettings& renderSettings, scene::MaterialManager& materialManager)
{
  m_renderSettings = renderSettings;
  resize(materialManager, m_size, true);
}

void RenderPipeline::resize(scene::MaterialManager& materialManager, const glm::ivec2& viewport, bool force)
{
  if(!force && m_size == viewport)
  {
    return;
  }

  m_size = viewport;

  m_geometryPass = std::make_shared<pass::GeometryPass>(viewport);
  m_portalPass = std::make_shared<pass::PortalPass>(*materialManager.getShaderManager(), viewport);
  m_ssaoPass = std::make_shared<pass::SSAOPass>(*materialManager.getShaderManager(), viewport / 2, *m_geometryPass);
  m_fxaaPass = std::make_shared<pass::FXAAPass>(*materialManager.getShaderManager(), viewport, *m_geometryPass);
  m_compositionPass = std::make_shared<pass::CompositionPass>(
    materialManager, m_renderSettings, viewport, *m_geometryPass, *m_portalPass, *m_ssaoPass, *m_fxaaPass);
  m_uiPass = std::make_shared<pass::UIPass>(*materialManager.getShaderManager(), viewport);
}

void RenderPipeline::bindPortalFrameBuffer()
{
  BOOST_ASSERT(m_portalPass != nullptr);
  BOOST_ASSERT(m_geometryPass != nullptr);
  m_portalPass->bind(*m_geometryPass->getDepthBuffer());
}

void RenderPipeline::bindUiFrameBuffer()
{
  BOOST_ASSERT(m_uiPass != nullptr);
  m_uiPass->bind();
}

void RenderPipeline::bindGeometryFrameBuffer(const glm::ivec2& size)
{
  BOOST_ASSERT(m_geometryPass != nullptr);
  m_geometryPass->bind(size);
}

void RenderPipeline::renderUiFrameBuffer(float alpha)
{
  BOOST_ASSERT(m_uiPass != nullptr);
  m_uiPass->render(alpha);
}
} // namespace render
