#include "compositionpass.h"

#include "config.h"
#include "fxaapass.h"
#include "geometrypass.h"
#include "portalpass.h"
#include "render/rendersettings.h"
#include "render/scene/material.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "ssaopass.h"
#include "util.h"

#include <gl/framebuffer.h>
#include <gl/texture2d.h>
#include <gl/texturedepth.h>
#include <random>

namespace render::pass
{
CompositionPass::CompositionPass(scene::MaterialManager& materialManager,
                                 const RenderSettings& renderSettings,
                                 const glm::ivec2& viewport,
                                 const GeometryPass& geometryPass,
                                 const PortalPass& portalPass,
                                 const SSAOPass& ssaoPass,
                                 const FXAAPass& fxaaPass)
    : m_compositionMaterial{materialManager.getComposition(
      false, renderSettings.lensDistortion, renderSettings.dof, renderSettings.filmGrain)}
    , m_waterCompositionMaterial{materialManager.getComposition(
        true, renderSettings.lensDistortion, renderSettings.dof, renderSettings.filmGrain)}
    , m_crtMaterial{materialManager.getCrt()}
    , m_mesh{createFbMesh(viewport, m_compositionMaterial->getShaderProgram()->getHandle())}
    , m_waterMesh{createFbMesh(viewport, m_waterCompositionMaterial->getShaderProgram()->getHandle())}
    , m_crtMesh{createFbMesh(viewport, m_crtMaterial->getShaderProgram()->getHandle())}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "composition-color")}
{
  const glm::ivec2 resolution{256, 256};

  std::uniform_real_distribution<float> randomFloats(0, 1);
  std::default_random_engine generator{}; // NOLINT(cert-msc32-c, cert-msc51-cpp)

  std::vector<gl::ScalarByte> noiseData;
  noiseData.resize(resolution.x * resolution.y);
  for(auto& i : noiseData)
  {
    const auto value = randomFloats(generator);
    i = gl::ScalarByte{static_cast<uint8_t>(value * value * 255)};
  }
  m_noise = std::make_shared<gl::Texture2D<gl::ScalarByte>>(resolution, "composition-noise");
  m_noise->assign(noiseData.data())
    .set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);

  m_compositionMaterial->getUniform("u_portalDepth")->set(portalPass.getDepthBuffer());
  if(renderSettings.waterDenoise)
    m_compositionMaterial->getUniform("u_portalPerturb")->set(portalPass.getBlurredTexture());
  else
    m_compositionMaterial->getUniform("u_portalPerturb")->set(portalPass.getNoisyTexture());
  m_compositionMaterial->getUniform("u_depth")->set(geometryPass.getDepthBuffer());
  m_compositionMaterial->getUniform("u_ao")->set(ssaoPass.getBlurredTexture());
  m_compositionMaterial->getUniform("u_texture")->set(fxaaPass.getColorBuffer());

  m_waterCompositionMaterial->getUniform("u_portalDepth")->set(portalPass.getDepthBuffer());
  if(renderSettings.waterDenoise)
    m_waterCompositionMaterial->getUniform("u_portalPerturb")->set(portalPass.getBlurredTexture());
  else
    m_waterCompositionMaterial->getUniform("u_portalPerturb")->set(portalPass.getNoisyTexture());
  m_waterCompositionMaterial->getUniform("u_depth")->set(geometryPass.getDepthBuffer());
  m_waterCompositionMaterial->getUniform("u_ao")->set(ssaoPass.getBlurredTexture());
  m_waterCompositionMaterial->getUniform("u_texture")->set(fxaaPass.getColorBuffer());

  m_colorBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);

  m_crtMaterial->getUniform("u_input")->set(m_colorBuffer);
  m_crtMaterial->getUniform("u_noise")->set(m_noise);

  m_fb = gl::FrameBufferBuilder()
           .texture(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
           .build("composition-fb");

  m_mesh->getMaterialGroup().set(scene::RenderMode::Full, m_compositionMaterial);
  m_waterMesh->getMaterialGroup().set(scene::RenderMode::Full, m_waterCompositionMaterial);
  m_crtMesh->getMaterialGroup().set(scene::RenderMode::Full, m_crtMaterial);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void CompositionPass::updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera)
{
  m_compositionMaterial->getUniformBlock("Camera")->bindCameraBuffer(camera);
  m_waterCompositionMaterial->getUniformBlock("Camera")->bindCameraBuffer(camera);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void CompositionPass::render(bool water, const RenderSettings& renderSettings)
{
  SOGLB_DEBUGGROUP("postprocess-pass");
  if(renderSettings.crt)
    m_fb->bindWithAttachments();
  else
    gl::Framebuffer::unbindAll();

  gl::RenderState state;
  state.setBlend(false);
  state.apply(true);
  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
  if(water)
    m_waterMesh->render(context);
  else
    m_mesh->render(context);

  if(renderSettings.crt)
  {
    gl::Framebuffer::unbindAll();
    m_crtMesh->render(context);
  }

  if constexpr(FlushPasses)
    GL_ASSERT(gl::api::finish());
}
} // namespace render::pass
