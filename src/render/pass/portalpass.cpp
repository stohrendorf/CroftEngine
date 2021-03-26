#include "portalpass.h"

#include <gl/texturedepth.h>

namespace render::pass
{
PortalPass::PortalPass(scene::MaterialManager& materialManager, const glm::vec2& viewport)
    : m_depthBuffer{std::make_shared<gl::TextureDepth<float>>(viewport, "portal-depth")}
    , m_perturbBuffer{std::make_shared<gl::Texture2D<gl::RG32F>>(viewport, "portal-perturb")}
    , m_blur{"perturb", materialManager, 4, true, false}
{
  m_depthBuffer->set(gl::api::TextureMinFilter::Linear).set(gl::api::TextureMagFilter::Linear);
  m_perturbBuffer->set(gl::api::TextureMinFilter::Linear).set(gl::api::TextureMagFilter::Linear);
  m_blur.setInput(m_perturbBuffer);

  m_fb = gl::FrameBufferBuilder()
           .texture(gl::api::FramebufferAttachment::DepthAttachment, m_depthBuffer)
           .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_perturbBuffer)
           .build("portal-fb");
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void PortalPass::bind(const gl::TextureDepth<float>& depth)
{
  gl::Framebuffer::unbindAll();
  m_depthBuffer->copyFrom(depth);
  m_fb->bindWithAttachments();
}
} // namespace render::pass
