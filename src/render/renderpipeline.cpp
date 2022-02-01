#include "renderpipeline.h"

#include "pass/effectpass.h"
#include "pass/geometrypass.h"
#include "pass/hbaopass.h"
#include "pass/portalpass.h"
#include "pass/uipass.h"
#include "pass/worldcompositionpass.h"
#include "render/scene/materialmanager.h"
#include "rendersettings.h"

#include <boost/assert.hpp>
#include <gl/framebuffer.h>
#include <gl/program.h>
#include <gl/texture2d.h>
#include <gl/texturedepth.h>
#include <gl/texturehandle.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <string>

namespace render::scene
{
class Material;
class Mesh;
class Node;
} // namespace render::scene

namespace render
{
RenderPipeline::RenderPipeline(scene::MaterialManager& materialManager,
                               const glm::ivec2& renderViewport,
                               const glm::ivec2& uiViewport,
                               const glm::ivec2& displayViewport)
{
  resize(materialManager, renderViewport, uiViewport, displayViewport, true);
}

void RenderPipeline::worldCompositionPass(const bool inWater)
{
  BOOST_ASSERT(m_portalPass != nullptr);
  if(m_renderSettings.waterDenoise)
    m_portalPass->renderBlur();
  BOOST_ASSERT(m_hbaoPass != nullptr);
  if(m_renderSettings.hbao)
    m_hbaoPass->render();
  BOOST_ASSERT(m_worldCompositionPass != nullptr);

  m_worldCompositionPass->render(inWater);
  auto finalOutput = m_worldCompositionPass->getFramebuffer();
  for(const auto& effect : m_effects)
  {
    effect->render(inWater);
    finalOutput = effect->getFramebuffer();
  }
  finalOutput->blit(m_displaySize);
}

void RenderPipeline::updateCamera(const gslu::nn_shared<scene::Camera>& camera)
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
  resize(materialManager, m_renderSize, m_uiSize, m_displaySize, true);
}

void RenderPipeline::resize(scene::MaterialManager& materialManager,
                            const glm::ivec2& renderViewport,
                            const glm::ivec2& uiViewport,
                            const glm::ivec2& displayViewport,
                            bool force)
{
  if(!force && m_renderSize == renderViewport && m_uiSize == uiViewport && m_displaySize == displayViewport)
  {
    return;
  }

  m_renderSize = renderViewport;
  m_uiSize = uiViewport;
  m_displaySize = displayViewport;

  m_geometryPass = std::make_shared<pass::GeometryPass>(m_renderSize);
  m_portalPass = std::make_shared<pass::PortalPass>(materialManager, m_geometryPass->getDepthBuffer(), m_renderSize);
  m_hbaoPass = std::make_shared<pass::HBAOPass>(materialManager, m_renderSize, *m_geometryPass);
  m_worldCompositionPass = std::make_shared<pass::WorldCompositionPass>(
    materialManager, m_renderSettings, m_renderSize, *m_geometryPass, *m_portalPass);

  auto fxSource = m_worldCompositionPass->getColorBuffer();
  auto addEffect = [this, &fxSource](const std::string& name, const gslu::nn_shared<render::scene::Material>& material)
  {
    auto fx = std::make_shared<pass::EffectPass>(gsl::not_null{this}, "fx:" + name, material, fxSource);
    m_effects.emplace_back(fx);
    fxSource = fx->getOutput();
    return fx;
  };

  m_effects.clear();
  if(m_renderSettings.hbao)
  {
    auto fx = addEffect("hbao", materialManager.getHBAOFx());
    fx->bind("u_ao",
             [texture = m_hbaoPass->getBlurredTexture()](
               const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
             {
               uniform.set(texture);
             });
  }
  addEffect("underwater-movement", materialManager.getUnderwaterMovement());
  if(m_renderSettings.fxaa)
    addEffect("fxaa", materialManager.getFXAA());
  if(m_renderSettings.lensDistortion)
    addEffect("lens", materialManager.getLensDistortion());
  if(m_renderSettings.velvia)
    addEffect("velvia", materialManager.getVelvia());
  if(m_renderSettings.filmGrain)
    addEffect("filmGrain", materialManager.getFilmGrain());
  if(m_renderSettings.crt)
    addEffect("crt", materialManager.getCRT());
  m_uiPass = std::make_shared<pass::UIPass>(materialManager, m_uiSize, m_displaySize);
}

gl::RenderState RenderPipeline::bindPortalFrameBuffer()
{
  BOOST_ASSERT(m_portalPass != nullptr);
  return m_portalPass->bind();
}

void RenderPipeline::bindUiFrameBuffer()
{
  BOOST_ASSERT(m_uiPass != nullptr);
  m_uiPass->bind();
}

void RenderPipeline::bindGeometryFrameBuffer(float farPlane)
{
  BOOST_ASSERT(m_geometryPass != nullptr);
  m_geometryPass->getColorBuffer()->getTexture()->clear({0, 0, 0, 1});
  m_geometryPass->getPositionBuffer()->getTexture()->clear({0.0f, 0.0f, -farPlane});
  m_geometryPass->getDepthBuffer()->clear(gl::ScalarDepth{1.0f});
  m_geometryPass->bind();
}

void RenderPipeline::renderUiFrameBuffer(float alpha)
{
  BOOST_ASSERT(m_uiPass != nullptr);
  m_uiPass->render(alpha);
}
} // namespace render
