#include "renderpipeline.h"

#include "pass/compositionpass.h"
#include "pass/fxaapass.h"
#include "pass/geometrypass.h"
#include "pass/hbaopass.h"
#include "pass/linearizedepthpass.h"
#include "pass/portalpass.h"
#include "pass/uipass.h"
#include "scene/materialmanager.h"
#include "scene/mesh.h"
#include "scene/shadercache.h"

#include <gl/texturedepth.h>

namespace render
{
RenderPipeline::RenderPipeline(scene::MaterialManager& materialManager, const glm::ivec2& viewport)
{
  resize(materialManager, viewport, true);
}

void RenderPipeline::compositionPass(const bool water)
{
  BOOST_ASSERT(m_portalPass != nullptr);
  if(m_renderSettings.waterDenoise)
    m_portalPass->renderBlur();
  BOOST_ASSERT(m_hbaoPass != nullptr);
  if(m_renderSettings.hbao)
    m_hbaoPass->render(m_size);
  BOOST_ASSERT(m_fxaaPass != nullptr);
  if(m_renderSettings.fxaa)
    m_fxaaPass->render(m_size);
  BOOST_ASSERT(m_linearizeDepthPass != nullptr);
  m_linearizeDepthPass->render();
  BOOST_ASSERT(m_linearizePortalDepthPass != nullptr);
  m_linearizePortalDepthPass->render();
  BOOST_ASSERT(m_compositionPass != nullptr);
  m_compositionPass->render(water, m_renderSettings);
}

void RenderPipeline::updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera)
{
  BOOST_ASSERT(m_compositionPass != nullptr);
  m_compositionPass->updateCamera(camera);
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
  m_linearizeDepthPass
    = std::make_shared<pass::LinearizeDepthPass>(materialManager, viewport, m_geometryPass->getDepthBuffer());
  m_portalPass = std::make_shared<pass::PortalPass>(materialManager, viewport);
  m_linearizePortalDepthPass
    = std::make_shared<pass::LinearizeDepthPass>(materialManager, viewport, m_portalPass->getDepthBuffer());
  m_hbaoPass = std::make_shared<pass::HBAOPass>(materialManager, viewport, *m_geometryPass);
  m_fxaaPass = std::make_shared<pass::FXAAPass>(materialManager, viewport, *m_geometryPass);
  m_compositionPass = std::make_shared<pass::CompositionPass>(materialManager,
                                                              m_renderSettings,
                                                              viewport,
                                                              *m_portalPass,
                                                              *m_hbaoPass,
                                                              m_renderSettings.fxaa ? m_fxaaPass->getColorBuffer()
                                                                                    : m_geometryPass->getColorBuffer(),
                                                              *m_linearizeDepthPass,
                                                              *m_linearizePortalDepthPass);
  m_uiPass = std::make_shared<pass::UIPass>(materialManager, viewport);
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
