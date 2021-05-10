#include "uipass.h"

#include "config.h"
#include "render/scene/material.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"

#include <gl/debuggroup.h>
#include <gl/framebuffer.h>
#include <gl/renderstate.h>
#include <gl/texture2d.h>

namespace render::pass
{
// NOLINTNEXTLINE(readability-make-member-function-const)
void UIPass::bind()
{
  gl::Framebuffer::unbindAll();
  m_colorBuffer->clear({0, 0, 0, 0});
  m_fb->bindWithAttachments();
}

UIPass::UIPass(scene::MaterialManager& materialManager, const glm::ivec2& viewport)
    : m_material{materialManager.getFlat(true, false)}
    , m_mesh{scene::createScreenQuad(m_material, "ui")}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "ui-color")}
{
  auto sampler = std::make_unique<gl::Sampler>("ui-color");
  sampler->set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Nearest)
    .set(gl::api::TextureMagFilter::Nearest);
  m_colorBufferHandle
    = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>(m_colorBuffer, std::move(sampler));

  m_mesh->bind("u_input",
               [this](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               { uniform.set(m_colorBufferHandle); });
  m_mesh->getRenderState().setBlend(true);

  m_fb
    = gl::FrameBufferBuilder().texture(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer).build("ui-fb");
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void UIPass::render(float alpha)
{
  SOGLB_DEBUGGROUP("ui-pass");
  gl::Framebuffer::unbindAll();

  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
  m_mesh->bind("u_alphaMultiplier",
               [alpha](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               { uniform.set(alpha); });
  m_mesh->render(context);

  if constexpr(FlushPasses)
    GL_ASSERT(gl::api::finish());
}
} // namespace render::pass
