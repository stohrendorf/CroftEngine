#include "renderpipeline.h"

#include "engine/world/room.h"
#include "material/rendermode.h"
#include "pass/config.h"
#include "pass/edgedetectionpass.h"
#include "pass/effectpass.h"
#include "pass/geometrypass.h"
#include "pass/hbaopass.h"
#include "pass/portalpass.h"
#include "pass/uipass.h"
#include "pass/worldcompositionpass.h"
#include "render/material/materialmanager.h"
#include "render/scene/visitor.h"
#include "rendersettings.h"
#include "scene/node.h"
#include "scene/rendercontext.h"
#include "scene/translucency.h"

#include <boost/assert.hpp>
#include <boost/throw_exception.hpp>
#include <gl/api/gl.hpp>
#include <gl/constants.h>
#include <gl/debuggroup.h>
#include <gl/framebuffer.h>
#include <gl/glassert.h>
#include <gl/pixel.h>
#include <gl/program.h>
#include <gl/renderstate.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturedepth.h>
#include <gl/texturehandle.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <optional>
#include <stdexcept>
#include <string>

namespace render::material
{
class Material;
}

namespace render::scene
{
class Mesh;
}

namespace render
{
RenderPipeline::RenderPipeline(material::MaterialManager& materialManager,
                               const glm::ivec2& renderViewport,
                               const glm::ivec2& uiViewport,
                               const glm::ivec2& displayViewport)
{
  resize(materialManager, renderViewport, uiViewport, displayViewport, true);
}

void RenderPipeline::worldCompositionPass(const std::vector<engine::world::Room>& rooms, const bool inWater)
{
  BOOST_ASSERT(m_portalPass != nullptr);
  if(m_renderSettings.waterDenoise)
    m_portalPass->renderBlur();

  if(m_renderSettings.hbao)
  {
    BOOST_ASSERT(m_hbaoPass != nullptr);
    m_hbaoPass->render();
  }

  if(m_renderSettings.edges)
  {
    BOOST_ASSERT(m_edgePass != nullptr);
    m_edgePass->render();
  }

  BOOST_ASSERT(m_worldCompositionPass != nullptr);
  m_worldCompositionPass->render(inWater);

  {
    SOGLB_DEBUGGROUP("dust");

    render::scene::RenderContext context{
      render::material::RenderMode::FullNonOpaque, std::nullopt, scene::Translucency::NonOpaque};
    for(const auto& room : rooms)
    {
      if(!room.node->isVisible() || room.dust == nullptr)
        continue;

      SOGLB_DEBUGGROUP(room.node->getName() + ":dust");
      auto state = context.getCurrentState();
      state.setScissorTest(true);
      const auto [x, y] = room.node->getCombinedScissors();
      state.setScissorRegion({x.min, y.min}, {x.size(), y.size()});
      context.pushState(state);

      render::scene::Visitor visitor{gsl::not_null{&context}};
      room.dust->accept(visitor);
      visitor.render(std::nullopt);

      context.popState();
    }
  }

  auto finalOutput = m_worldCompositionPass->getFramebuffer();
  for(const auto& effect : m_effects)
  {
    effect->render(inWater);
    finalOutput = effect->getFramebuffer();
  }
  gsl_Assert(m_backbuffer != nullptr);
  gl::RenderState::getWantedState().setViewport(m_displaySize);
  gl::RenderState::applyWantedState();

  finalOutput->blit(*m_backbuffer);
}

void RenderPipeline::updateCamera(const gslu::nn_shared<scene::Camera>& camera)
{
  BOOST_ASSERT(m_worldCompositionPass != nullptr);
  m_worldCompositionPass->updateCamera(camera);
  BOOST_ASSERT(m_hbaoPass != nullptr);
  if(m_renderSettings.hbao)
    m_hbaoPass->updateCamera(camera);
}

void RenderPipeline::apply(const RenderSettings& renderSettings, material::MaterialManager& materialManager)
{
  m_renderSettings = renderSettings;
  resize(materialManager, m_renderSize, m_uiSize, m_displaySize, true);
}

void RenderPipeline::resize(material::MaterialManager& materialManager,
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
  m_portalPass = std::make_shared<pass::PortalPass>(materialManager, gsl::not_null{m_geometryPass}, m_renderSize);
  m_hbaoPass = std::make_shared<pass::HBAOPass>(materialManager, m_renderSize / 4, gsl::not_null{m_geometryPass});
  m_edgePass = std::make_shared<pass::EdgeDetectionPass>(materialManager, m_renderSize, gsl::not_null{m_geometryPass});
  m_worldCompositionPass = std::make_shared<pass::WorldCompositionPass>(
    materialManager, m_renderSettings, m_renderSize, gsl::not_null{m_geometryPass}, gsl::not_null{m_portalPass});
  m_uiPass = std::make_shared<pass::UIPass>(materialManager, m_uiSize, m_displaySize);

  m_backbufferTextureHandle = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>>(
    gsl::make_shared<gl::Texture2D<gl::SRGB8>>(m_displaySize, "backbuffer-texture"),
    gsl::make_unique<gl::Sampler>("backbuffer" + gl::SamplerSuffix));
  m_backbuffer = gl::FrameBufferBuilder{}
                   .texture(gl::api::FramebufferAttachment::ColorAttachment0, m_backbufferTextureHandle->getTexture())
                   .build("backbuffer");

  initWorldEffects(materialManager);
  initBackbufferEffects(materialManager);
}

void RenderPipeline::initWorldEffects(material::MaterialManager& materialManager)
{
  m_effects.clear();

  auto fxSource = m_worldCompositionPass->getColorBuffer();
  auto addEffect = [this, &fxSource](const std::string& name, const gslu::nn_shared<material::Material>& material)
  {
    auto fx = std::make_shared<pass::EffectPass<gl::SRGB8>>(gsl::not_null{this}, "fx:" + name, material, fxSource);
    m_effects.emplace_back(fx);
    fxSource = fx->getOutput();
    return fx;
  };

  if(m_renderSettings.hbao || m_renderSettings.edges)
  {
    auto fx = addEffect("masking", materialManager.getMasking(m_renderSettings.hbao, m_renderSettings.edges));
    if(m_renderSettings.hbao)
    {
      BOOST_ASSERT(m_hbaoPass != nullptr);
      fx->bind("u_ao",
               [hbaoPass = m_hbaoPass](const scene::Node* /*node*/, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               {
                 uniform.set(hbaoPass->getBlurredTexture());
               });
    }
    if(m_renderSettings.edges)
    {
      BOOST_ASSERT(m_edgePass != nullptr);
      fx->bind("u_edges",
               [edgePass = m_edgePass](const scene::Node* /*node*/, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               {
                 uniform.set(edgePass->getTexture());
               });
    }
  }

  if(m_renderSettings.fxaaActive)
  {
    addEffect("fxaa", materialManager.getFXAA(m_renderSettings.fxaaPreset));
  }

  addEffect("underwater-movement", materialManager.getUnderwaterMovement());

  {
    auto fx = addEffect("reflective", materialManager.getReflective());
    BOOST_ASSERT(m_geometryPass != nullptr);
    fx->bind("u_normal",
             [texture = m_geometryPass->getNormalBuffer()](
               const scene::Node* /*node*/, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
             {
               uniform.set(texture);
             });
    fx->bind("u_reflective",
             [texture = m_geometryPass->getReflectiveBuffer()](
               const scene::Node* /*node*/, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
             {
               uniform.set(texture);
             });
  }

  if(m_renderSettings.lensDistortion)
  {
    addEffect("lens", materialManager.getLensDistortion());
  }

  if(m_renderSettings.velvia)
  {
    addEffect("velvia", materialManager.getVelvia());
  }

  addEffect("death", materialManager.getDeath());

  if(m_renderSettings.filmGrain)
  {
    addEffect("filmGrain", materialManager.getFilmGrain());
  }
}
void RenderPipeline::initBackbufferEffects(material::MaterialManager& materialManager)
{
  m_backbufferEffects.clear();

  auto fxSource = gsl::not_null{m_backbufferTextureHandle};
  auto addEffect = [this, &fxSource](const std::string& name, const gslu::nn_shared<material::Material>& material)
  {
    auto fx = std::make_shared<pass::EffectPass<gl::SRGB8>>(gsl::not_null{this}, "postfx:" + name, material, fxSource);
    m_backbufferEffects.emplace_back(fx);
    fxSource = fx->getOutput();
    return fx;
  };

  if(m_renderSettings.crtActive)
  {
    switch(m_renderSettings.crtVersion)
    {
    case 0:
      addEffect("crt0", materialManager.getCRTV0());
      break;
    case 1:
      addEffect("crt1", materialManager.getCRTV1());
      break;
    default:
      BOOST_THROW_EXCEPTION(std::out_of_range("invalid crt version"));
    }
  }

  if((m_renderSettings.brightnessEnabled && m_renderSettings.brightness != 0)
     || (m_renderSettings.contrastEnabled && m_renderSettings.contrast != 0))
  {
    addEffect("brightness-contrast",
              materialManager.getBrightnessContrast(
                m_renderSettings.brightnessEnabled ? m_renderSettings.brightness : int8_t(0),
                m_renderSettings.contrastEnabled ? m_renderSettings.contrast : int8_t(0)));
  }
}

void RenderPipeline::renderPortalFrameBuffer(const std::function<void(const gl::RenderState&)>& doRender)
{
  BOOST_ASSERT(m_portalPass != nullptr);
  m_portalPass->render(doRender);

  if constexpr(render::pass::FlushPasses)
    GL_ASSERT(gl::api::finish());
}

void RenderPipeline::renderUiFrameBuffer(const std::function<void()>& doRender)
{
  BOOST_ASSERT(m_uiPass != nullptr);
  m_uiPass->render(doRender);
}

void RenderPipeline::renderGeometryFrameBuffer(const std::function<void()>& doRender, float farPlane)
{
  SOGLB_DEBUGGROUP("geometry-pass");
  BOOST_ASSERT(m_geometryPass != nullptr);
  m_geometryPass->getColorBuffer()->getTexture()->clear({0, 0, 0, 1});
  m_geometryPass->getPositionBuffer()->getTexture()->clear({0.0f, 0.0f, -farPlane});
  m_geometryPass->getReflectiveBuffer()->getTexture()->clear({0, 0, 0, 0});
  m_geometryPass->getDepthBuffer()->clear(gl::ScalarDepth{1.0f});
  m_geometryPass->getNormalBuffer()->getTexture()->clear(gl::RGB16F{gl::api::core::Half{}});
  m_geometryPass->render(doRender);

  if constexpr(render::pass::FlushPasses)
    GL_ASSERT(gl::api::finish());
}

void RenderPipeline::renderUiFrameBuffer(float alpha)
{
  BOOST_ASSERT(m_uiPass != nullptr);
  m_backbuffer->bind();
  GL_ASSERT(gl::api::textureBarrier());
  m_uiPass->render(alpha);
  m_backbuffer->unbind();
}

void RenderPipeline::withBackbuffer(const std::function<void()>& doRender)
{
  m_backbuffer->bind();
  GL_ASSERT(gl::api::textureBarrier());
  doRender();
  m_backbuffer->unbind();
}

void RenderPipeline::renderBackbufferEffects()
{
  gsl_Assert(m_backbuffer != nullptr);

  gl::RenderState::getWantedState().setViewport(m_displaySize);
  gl::RenderState::applyWantedState();

  auto finalOutput = m_backbuffer;
  for(const auto& effect : m_backbufferEffects)
  {
    effect->render(false);
    finalOutput = effect->getFramebuffer();
  }
  finalOutput->blit(m_displaySize);
}

void RenderPipeline::resetBackbuffer()
{
  m_backbufferTextureHandle->getTexture()->clear({0, 0, 0});
}
} // namespace render
