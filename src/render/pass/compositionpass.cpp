#include "compositionpass.h"

#include "config.h"
#include "fxaapass.h"
#include "geometrypass.h"
#include "linearizedepthpass.h"
#include "portalpass.h"
#include "render/rendersettings.h"
#include "render/scene/material.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "ssaopass.h"

#include <gl/framebuffer.h>
#include <gl/texture2d.h>
#include <gl/texturedepth.h>

namespace render::pass
{
CompositionPass::CompositionPass(scene::MaterialManager& materialManager,
                                 const RenderSettings& renderSettings,
                                 const glm::ivec2& viewport,
                                 const PortalPass& portalPass,
                                 const SSAOPass& ssaoPass,
                                 const FXAAPass& fxaaPass,
                                 const LinearizeDepthPass& linearizeDepthPass,
                                 const LinearizeDepthPass& linearizePortalDepthPass)
    : m_compositionMaterial{materialManager.getComposition(
      false, renderSettings.lensDistortion, renderSettings.dof, renderSettings.filmGrain)}
    , m_waterCompositionMaterial{materialManager.getComposition(
        true, renderSettings.lensDistortion, renderSettings.dof, renderSettings.filmGrain)}
    , m_mesh{scene::createScreenQuad(m_compositionMaterial)}
    , m_waterMesh{scene::createScreenQuad(m_waterCompositionMaterial)}
    , m_crtMesh{scene::createScreenQuad(materialManager.getCrt())}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "composition-color")}
{
  m_mesh->bind("u_linearPortalDepth",
               [buffer = linearizePortalDepthPass.getTexture()](const render::scene::Node& /*node*/,
                                                                const render::scene::Mesh& /*mesh*/,
                                                                gl::Uniform& uniform) { uniform.set(buffer); });
  if(renderSettings.waterDenoise)
    m_mesh->bind("u_portalPerturb",
                 [texture = portalPass.getBlurredTexture()](const render::scene::Node& /*node*/,
                                                            const render::scene::Mesh& /*mesh*/,
                                                            gl::Uniform& uniform) { uniform.set(texture); });
  else
    m_mesh->bind("u_portalPerturb",
                 [texture = portalPass.getNoisyTexture()](const render::scene::Node& /*node*/,
                                                          const render::scene::Mesh& /*mesh*/,
                                                          gl::Uniform& uniform) { uniform.set(texture); });
  m_mesh->bind("u_linearDepth",
               [buffer = linearizeDepthPass.getTexture()](const render::scene::Node& /*node*/,
                                                          const render::scene::Mesh& /*mesh*/,
                                                          gl::Uniform& uniform) { uniform.set(buffer); });
  m_mesh->bind("u_ao",
               [texture = ssaoPass.getBlurredTexture()](const render::scene::Node& /*node*/,
                                                        const render::scene::Mesh& /*mesh*/,
                                                        gl::Uniform& uniform) { uniform.set(texture); });
  m_mesh->bind("u_texture",
               [buffer = fxaaPass.getColorBuffer()](const render::scene::Node& /*node*/,
                                                    const render::scene::Mesh& /*mesh*/,
                                                    gl::Uniform& uniform) { uniform.set(buffer); });

  m_waterMesh->bind("u_linearPortalDepth",
                    [buffer = linearizePortalDepthPass.getTexture()](const render::scene::Node& /*node*/,
                                                                     const render::scene::Mesh& /*mesh*/,
                                                                     gl::Uniform& uniform) { uniform.set(buffer); });

  if(renderSettings.waterDenoise)
    m_waterMesh->bind("u_portalPerturb",
                      [texture = portalPass.getBlurredTexture()](const render::scene::Node& /*node*/,
                                                                 const render::scene::Mesh& /*mesh*/,
                                                                 gl::Uniform& uniform) { uniform.set(texture); });
  else
    m_waterMesh->bind("u_portalPerturb",
                      [texture = portalPass.getNoisyTexture()](const render::scene::Node& /*node*/,
                                                               const render::scene::Mesh& /*mesh*/,
                                                               gl::Uniform& uniform) { uniform.set(texture); });
  m_waterMesh->bind("u_linearDepth",
                    [buffer = linearizeDepthPass.getTexture()](const render::scene::Node& /*node*/,
                                                               const render::scene::Mesh& /*mesh*/,
                                                               gl::Uniform& uniform) { uniform.set(buffer); });
  m_waterMesh->bind("u_ao",
                    [texture = ssaoPass.getBlurredTexture()](const render::scene::Node& /*node*/,
                                                             const render::scene::Mesh& /*mesh*/,
                                                             gl::Uniform& uniform) { uniform.set(texture); });
  m_waterMesh->bind("u_texture",
                    [buffer = fxaaPass.getColorBuffer()](const render::scene::Node& /*node*/,
                                                         const render::scene::Mesh& /*mesh*/,
                                                         gl::Uniform& uniform) { uniform.set(buffer); });

  m_colorBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::Repeat)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);

  m_crtMesh->bind(
    "u_input", [this](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform) {
      uniform.set(m_colorBuffer);
    });

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
