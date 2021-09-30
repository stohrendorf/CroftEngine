#include "renderpipeline.h"

#include "pass/fxaapass.h"
#include "pass/geometrypass.h"
#include "pass/hbaopass.h"
#include "pass/portalpass.h"
#include "pass/uipass.h"
#include "pass/worldcompositionpass.h"
#include "rendersettings.h"

#include <boost/assert.hpp>
#include <gl/texturedepth.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>

namespace render
{
RenderPipeline::RenderPipeline(scene::MaterialManager& materialManager, const glm::ivec2& viewport)
{
  resize(materialManager, viewport, true);
}

void RenderPipeline::worldCompositionPass(const bool water)
{
  BOOST_ASSERT(m_portalPass != nullptr);
  if(m_renderSettings.waterDenoise)
    m_portalPass->renderBlur();
  BOOST_ASSERT(m_hbaoPass != nullptr);
  if(m_renderSettings.hbao)
    m_hbaoPass->render();
  BOOST_ASSERT(m_fxaaPass != nullptr);
  if(m_renderSettings.fxaa)
    m_fxaaPass->render();
  BOOST_ASSERT(m_worldCompositionPass != nullptr);
  m_worldCompositionPass->render(water, m_renderSettings);
}

void RenderPipeline::updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera)
{
  BOOST_ASSERT(m_worldCompositionPass != nullptr);
  m_worldCompositionPass->updateCamera(camera);
  BOOST_ASSERT(m_hbaoPass != nullptr);
  if(m_renderSettings.hbao)
    m_hbaoPass->updateCamera(camera);
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
  m_portalPass = std::make_shared<pass::PortalPass>(materialManager, m_geometryPass->getDepthBuffer(), viewport);
  m_hbaoPass = std::make_shared<pass::HBAOPass>(materialManager, viewport, *m_geometryPass);
  m_fxaaPass = std::make_shared<pass::FXAAPass>(materialManager, viewport, m_geometryPass->getColorBuffer());
  m_worldCompositionPass = std::make_shared<pass::WorldCompositionPass>(
    materialManager,
    m_renderSettings,
    viewport,
    *m_geometryPass,
    *m_portalPass,
    *m_hbaoPass,
    m_renderSettings.fxaa ? m_fxaaPass->getColorBuffer() : m_geometryPass->getColorBuffer());
  m_uiPass = std::make_shared<pass::UIPass>(materialManager, viewport);
}

gl::RenderState RenderPipeline::bindPortalFrameBuffer()
{
  BOOST_ASSERT(m_portalPass != nullptr);
  BOOST_ASSERT(m_geometryPass != nullptr);
  return m_portalPass->bind(*m_geometryPass->getPositionBuffer());
}

void RenderPipeline::bindUiFrameBuffer()
{
  BOOST_ASSERT(m_uiPass != nullptr);
  m_uiPass->bind();
}

void RenderPipeline::bindGeometryFrameBuffer(float farPlane)
{
  BOOST_ASSERT(m_geometryPass != nullptr);
  m_geometryPass->getColorBuffer()->getTexture()->clear({0, 0, 0});
  m_geometryPass->getPositionBuffer()->getTexture()->clear({0.0f, 0.0f, -farPlane});
  m_geometryPass->getDepthBuffer()->clear(gl::ScalarDepth{1.0f});
  return m_geometryPass->bind();
}

void RenderPipeline::renderUiFrameBuffer(float alpha)
{
  BOOST_ASSERT(m_uiPass != nullptr);
  m_uiPass->render(alpha);
}
} // namespace render
