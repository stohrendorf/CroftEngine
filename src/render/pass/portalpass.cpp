#include "portalpass.h"

namespace render::pass
{
PortalPass::PortalPass(scene::MaterialManager& materialManager, const glm::vec2& viewport)
    : m_depthBuffer{std::make_shared<gl::TextureDepth<float>>(viewport, "portal-depth")}
    , m_perturbBuffer{std::make_shared<gl::Texture2D<gl::RG32F>>(viewport, "portal-perturb")}
    , m_blur{"perturb", materialManager, 4, true}
{
  auto sampler = std::make_unique<gl::Sampler>("portal-depth");
  sampler->set(gl::api::TextureMinFilter::Linear).set(gl::api::TextureMagFilter::Linear);
  m_depthBufferHandle = std::make_shared<gl::TextureHandle<gl::TextureDepth<float>>>(m_depthBuffer, std::move(sampler));

  sampler = std::make_unique<gl::Sampler>("portal-perturb");
  sampler->set(gl::api::TextureMinFilter::Linear).set(gl::api::TextureMagFilter::Linear);
  m_perturbBufferHandle
    = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::RG32F>>>(m_perturbBuffer, std::move(sampler));
  m_blur.setInput(m_perturbBufferHandle);

  m_fb = gl::FrameBufferBuilder()
           .texture(gl::api::FramebufferAttachment::DepthAttachment, m_depthBuffer)
           .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_perturbBuffer)
           .build("portal-fb");
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void PortalPass::bind(const gl::TextureHandle<gl::TextureDepth<float>>& depth)
{
  gl::Framebuffer::unbindAll();
  m_depthBuffer->copyFrom(*depth.getTexture());
  m_fb->bindWithAttachments();
}
} // namespace render::pass
