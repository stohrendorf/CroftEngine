#include "compositionpass.h"

#include "config.h"
#include "hbaopass.h"
#include "linearizedepthpass.h"
#include "portalpass.h"
#include "render/rendersettings.h"
#include "render/scene/material.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"
#include "render/scene/rendermode.h"
#include "render/scene/uniformparameter.h"

#include <algorithm>
#include <gl/debuggroup.h>
#include <gl/framebuffer.h>
#include <gl/glassert.h>
#include <gl/pixel.h>
#include <gl/program.h>
#include <gl/renderstate.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturehandle.h>
#include <glm/mat4x4.hpp>
#include <gsl/gsl-lite.hpp>
#include <optional>
#include <utility>
#include <vector>

namespace render::scene
{
class Node;
}

namespace render::pass
{
CompositionPass::CompositionPass(scene::MaterialManager& materialManager,
                                 const RenderSettings& renderSettings,
                                 const glm::ivec2& viewport,
                                 const PortalPass& portalPass,
                                 const HBAOPass& hbaoPass,
                                 const std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>& colorBuffer,
                                 const LinearizeDepthPass& linearizeDepthPass,
                                 const LinearizeDepthPass& linearizePortalDepthPass)
    : m_compositionMaterial{materialManager.getComposition(false,
                                                           renderSettings.lensDistortion,
                                                           renderSettings.dof,
                                                           renderSettings.filmGrain,
                                                           renderSettings.hbao,
                                                           renderSettings.velvia)}
    , m_waterCompositionMaterial{materialManager.getComposition(true,
                                                                renderSettings.lensDistortion,
                                                                renderSettings.dof,
                                                                renderSettings.filmGrain,
                                                                renderSettings.hbao,
                                                                renderSettings.velvia)}
    , m_mesh{scene::createScreenQuad(m_compositionMaterial, "composition")}
    , m_waterMesh{scene::createScreenQuad(m_waterCompositionMaterial, "composition-water")}
    , m_crtMesh{scene::createScreenQuad(materialManager.getCrt(), "composition-crt")}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "composition-color")}
{
  m_mesh->bind("u_linearPortalDepth",
               [buffer = linearizePortalDepthPass.getTexture()](
                 const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               { uniform.set(buffer); });
  if(renderSettings.waterDenoise)
    m_mesh->bind("u_portalPerturb",
                 [texture = portalPass.getBlurredTexture()](
                   const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                 { uniform.set(texture); });
  else
    m_mesh->bind("u_portalPerturb",
                 [texture = portalPass.getNoisyTexture()](
                   const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                 { uniform.set(texture); });
  m_mesh->bind("u_linearDepth",
               [buffer = linearizeDepthPass.getTexture()](
                 const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               { uniform.set(buffer); });
  if(renderSettings.hbao)
    m_mesh->bind("u_ao",
                 [texture = hbaoPass.getBlurredTexture()](
                   const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                 { uniform.set(texture); });
  m_mesh->bind(
    "u_texture",
    [colorBuffer](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
    { uniform.set(colorBuffer); });

  m_waterMesh->bind("u_linearPortalDepth",
                    [buffer = linearizePortalDepthPass.getTexture()](
                      const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                    { uniform.set(buffer); });

  if(renderSettings.waterDenoise)
    m_waterMesh->bind("u_portalPerturb",
                      [texture = portalPass.getBlurredTexture()](
                        const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      { uniform.set(texture); });
  else
    m_waterMesh->bind("u_portalPerturb",
                      [texture = portalPass.getNoisyTexture()](
                        const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      { uniform.set(texture); });
  m_waterMesh->bind("u_linearDepth",
                    [buffer = linearizeDepthPass.getTexture()](
                      const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                    { uniform.set(buffer); });
  if(renderSettings.hbao)
    m_waterMesh->bind("u_ao",
                      [texture = hbaoPass.getBlurredTexture()](
                        const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      { uniform.set(texture); });
  m_waterMesh->bind(
    "u_texture",
    [colorBuffer](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
    { uniform.set(colorBuffer); });

  auto sampler = std::make_unique<gl::Sampler>("composition-color");
  sampler->set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);
  m_colorBufferHandle
    = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>(m_colorBuffer, std::move(sampler));

  m_crtMesh->bind("u_input",
                  [this](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                  { uniform.set(m_colorBufferHandle); });

  m_fb = gl::FrameBufferBuilder()
           .texture(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
           .build("composition-fb");
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

  gl::RenderState::resetWantedState();
  gl::RenderState::getWantedState().setBlend(false);
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
