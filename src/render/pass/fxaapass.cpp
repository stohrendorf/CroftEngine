#include "fxaapass.h"

#include "config.h"
#include "geometrypass.h"
#include "render/scene/material.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"
#include "render/scene/shadermanager.h"

#include <gl/debuggroup.h>
#include <gl/framebuffer.h>
#include <gl/texture2d.h>

namespace render::pass
{
FXAAPass::FXAAPass(scene::ShaderManager& shaderManager, const glm::ivec2& viewport, const GeometryPass& geometryPass)
    : m_material{std::make_shared<scene::Material>(shaderManager.getFXAA())}
    , m_mesh{scene::createScreenQuad(viewport, m_material)}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "fxaa-color")}
{
  m_colorBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);

  m_mesh->bind("u_input",
               [buffer = geometryPass.getColorBuffer()](const render::scene::Node& /*node*/,
                                                        const render::scene::Mesh& /*mesh*/,
                                                        gl::Uniform& uniform) { uniform.set(buffer); });

  m_fb = gl::FrameBufferBuilder()
           .texture(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
           .build("fxaa-fb");
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void FXAAPass::bind()
{
  m_fb->bindWithAttachments();
}

void FXAAPass::render(const glm::ivec2& size)
{
  SOGLB_DEBUGGROUP("fxaa-pass");
  GL_ASSERT(gl::api::viewport(0, 0, size.x, size.y));
  bind();

  gl::RenderState state;
  state.setBlend(false);
  state.apply(true);
  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};

  m_mesh->render(context);

  if constexpr(FlushPasses)
    GL_ASSERT(gl::api::finish());
}
} // namespace render::pass
