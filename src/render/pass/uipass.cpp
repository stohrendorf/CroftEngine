#include "uipass.h"

#include "config.h"
#include "render/scene/material.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"
#include "render/scene/shadermanager.h"
#include "util.h"

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
  m_fb->bindWithAttachments();
}

UIPass::UIPass(scene::ShaderManager& shaderManager, const glm::ivec2& viewport)
    : m_shader{shaderManager.getFlat(true, false)}
    , m_material{std::make_shared<scene::Material>(m_shader)}
    , m_mesh{createFbMesh(viewport, m_shader->getHandle())}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "ui-color")}
{
  m_colorBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Nearest)
    .set(gl::api::TextureMagFilter::Nearest);

  m_mesh->bind("u_input",
               [this](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform) {
                 uniform.set(m_colorBuffer);
               });
  m_mesh->getMaterialGroup().set(scene::RenderMode::Full, m_material);

  m_fb
    = gl::FrameBufferBuilder().texture(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer).build("ui-fb");
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void UIPass::render(float alpha)
{
  SOGLB_DEBUGGROUP("ui-pass");
  gl::Framebuffer::unbindAll();

  gl::RenderState state;
  state.setBlend(true);
  state.apply(true);
  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
  m_mesh->bind("u_alphaMultiplier",
               [alpha](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform) {
                 uniform.set(alpha);
               });
  m_mesh->render(context);

  if constexpr(FlushPasses)
    GL_ASSERT(gl::api::finish());
}
} // namespace render::pass
