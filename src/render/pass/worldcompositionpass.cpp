#include "worldcompositionpass.h"

#include "bloompass.h"
#include "config.h"
#include "geometrypass.h"
#include "portalpass.h"
#include "render/material/material.h"
#include "render/material/materialmanager.h"
#include "render/material/rendermode.h"
#include "render/material/uniformparameter.h"
#include "render/rendersettings.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"
#include "render/scene/translucency.h"

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
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <optional>
#include <string>

namespace render::scene
{
class Node;
}

namespace render::pass
{
WorldCompositionPass::WorldCompositionPass(material::MaterialManager& materialManager,
                                           const RenderSettings& renderSettings,
                                           const glm::ivec2& viewport,
                                           const gslu::nn_shared<GeometryPass>& geometryPass,
                                           const gslu::nn_shared<PortalPass>& portalPass)
    : m_geometryPass{geometryPass}
    , m_portalPass{portalPass}
    , m_noWaterMaterial{materialManager.getWorldComposition(false, renderSettings.dof)}
    , m_inWaterMaterial{materialManager.getWorldComposition(true, renderSettings.dof)}
    , m_noWaterMesh{scene::createScreenQuad(m_noWaterMaterial, scene::Translucency::Opaque, "composition-no-water")}
    , m_inWaterMesh{scene::createScreenQuad(m_inWaterMaterial, scene::Translucency::Opaque, "composition-water")}
    , m_bloomMesh{scene::createScreenQuad(materialManager.getBloom(), scene::Translucency::Opaque, "composition-bloom")}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGB8>>(viewport, "composition-color")}
    , m_colorBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>>(
        m_colorBuffer,
        gsl::make_unique<gl::Sampler>("composition-color" + gl::SamplerSuffix)
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::Repeat)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::Repeat)
          | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear))}
    , m_bloomedBuffer{std::make_shared<gl::Texture2D<gl::SRGB8>>(viewport, "composition-bloomed")}
    , m_bloomedBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>>(
        m_bloomedBuffer,
        gsl::make_unique<gl::Sampler>("composition-bloomed" + gl::SamplerSuffix)
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear))}
    , m_fb{gl::FrameBufferBuilder()
             .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
             .textureNoBlend(gl::api::FramebufferAttachment::DepthAttachment, geometryPass->getDepthBuffer())
             .build("composition-fb")}
    , m_bloomPass{std::make_shared<BloomPass<5, gl::SRGB8>>(materialManager, m_colorBufferHandle)}
    , m_fbBloom{gl::FrameBufferBuilder()
                  .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_bloomedBuffer)
                  .build("composition-fb-bloom")}
    , m_bloom{renderSettings.bloom}
{
  m_noWaterMesh->bind("u_portalPosition",
                      [buffer = portalPass->getPositionBuffer()](
                        const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      {
                        uniform.set(buffer);
                      });
  if(renderSettings.waterDenoise)
    m_noWaterMesh->bind("u_portalPerturb",
                        [texture = portalPass->getBlurredTexture()](const render::scene::Node* /*node*/,
                                                                    const render::scene::Mesh& /*mesh*/,
                                                                    gl::Uniform& uniform)
                        {
                          uniform.set(texture);
                        });
  else
    m_noWaterMesh->bind("u_portalPerturb",
                        [texture = portalPass->getNoisyTexture()](const render::scene::Node* /*node*/,
                                                                  const render::scene::Mesh& /*mesh*/,
                                                                  gl::Uniform& uniform)
                        {
                          uniform.set(texture);
                        });
  m_noWaterMesh->bind("u_geometryPosition",
                      [buffer = geometryPass->getPositionBuffer()](
                        const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      {
                        uniform.set(buffer);
                      });
  m_noWaterMesh->bind("u_texture",
                      [buffer = geometryPass->getColorBuffer()](
                        const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      {
                        uniform.set(buffer);
                      });

  m_noWaterMesh->getRenderState().merge(m_fb->getRenderState());

  m_inWaterMesh->bind("u_portalPosition",
                      [buffer = portalPass->getPositionBuffer()](
                        const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      {
                        uniform.set(buffer);
                      });

  if(renderSettings.waterDenoise)
    m_inWaterMesh->bind("u_portalPerturb",
                        [texture = portalPass->getBlurredTexture()](const render::scene::Node* /*node*/,
                                                                    const render::scene::Mesh& /*mesh*/,
                                                                    gl::Uniform& uniform)
                        {
                          uniform.set(texture);
                        });
  else
    m_inWaterMesh->bind("u_portalPerturb",
                        [texture = portalPass->getNoisyTexture()](const render::scene::Node* /*node*/,
                                                                  const render::scene::Mesh& /*mesh*/,
                                                                  gl::Uniform& uniform)
                        {
                          uniform.set(texture);
                        });
  m_inWaterMesh->bind("u_geometryPosition",
                      [buffer = geometryPass->getPositionBuffer()](
                        const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      {
                        uniform.set(buffer);
                      });
  m_inWaterMesh->bind("u_texture",
                      [buffer = geometryPass->getColorBuffer()](
                        const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      {
                        uniform.set(buffer);
                      });

  m_inWaterMesh->getRenderState().merge(m_fb->getRenderState());

  m_bloomMesh->bind(
    "u_input",
    [this](const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
    {
      uniform.set(m_colorBufferHandle);
    });
  m_bloomMesh->bind(
    "u_bloom",
    [this](const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
    {
      uniform.set(m_bloomPass->getOutput());
    });
  m_bloomMesh->getRenderState().merge(m_fbBloom->getRenderState());
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void WorldCompositionPass::updateCamera(const gslu::nn_shared<scene::Camera>& camera)
{
  m_noWaterMaterial->getUniformBlock("Camera")->bindCameraBuffer(camera);
  m_inWaterMaterial->getUniformBlock("Camera")->bindCameraBuffer(camera);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void WorldCompositionPass::render(bool inWater)
{
  SOGLB_DEBUGGROUP("world-composition-pass");

  m_fb->bind();

  scene::RenderContext context{material::RenderMode::FullOpaque, std::nullopt, scene::Translucency::Opaque};
  if(inWater)
    m_inWaterMesh->render(nullptr, context);
  else
    m_noWaterMesh->render(nullptr, context);

  m_fb->unbind();

  if(m_bloom)
  {
    m_bloomPass->render();
    m_fbBloom->bind();
    m_bloomMesh->render(nullptr, context);
    m_fbBloom->unbind();
  }

  if constexpr(FlushPasses)
    GL_ASSERT(gl::api::finish());
}
} // namespace render::pass
