#include "linearizedepthpass.h"

#include "config.h"
#include "render/scene/material.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"

#include <gl/debuggroup.h>
#include <gl/framebuffer.h>
#include <gl/texture2d.h>

namespace render::pass
{
LinearizeDepthPass::LinearizeDepthPass(scene::MaterialManager& materialManager,
                                       const glm::ivec2& viewport,
                                       const std::shared_ptr<gl::TextureDepth<float>>& depth)
    : m_material{materialManager.getLinearDepth()}
    , m_renderMesh{scene::createScreenQuad(m_material, "linearize-depth")}
    , m_linearDepth{std::make_shared<gl::Texture2D<gl::Scalar32F>>(viewport, "linearize-depth")}
{
  m_renderMesh->bind(
    "u_depth", [depth](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform) {
      uniform.set(depth);
    });

  m_linearDepth->set(gl::api::TextureMinFilter::Nearest).set(gl::api::TextureMagFilter::Nearest);

  m_fb = gl::FrameBufferBuilder()
           .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_linearDepth)
           .build("linearize-depth-fb");
}

void LinearizeDepthPass::render()
{
  SOGLB_DEBUGGROUP("linearize-depth-pass");
  m_fb->bindWithAttachments();

  gl::RenderState state;
  state.setBlend(false);
  state.apply(true);
  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};

  m_renderMesh->render(context);

  if constexpr(FlushPasses)
    GL_ASSERT(gl::api::finish());
}

void LinearizeDepthPass::updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera)
{
  m_material->getUniformBlock("Camera")->bindCameraBuffer(camera);
}
} // namespace render::pass
