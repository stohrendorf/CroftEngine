#include "portalpass.h"

#include "render/scene/blur.h"

#include <gl/pixel.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturedepth.h>
#include <gl/texturehandle.h>
#include <glm/fwd.hpp>
#include <gslu.h>
#include <memory>
#include <utility>

namespace render::scene
{
class MaterialManager;
}

namespace render::pass
{
PortalPass::PortalPass(scene::MaterialManager& materialManager,
                       const gsl::not_null<std::shared_ptr<gl::TextureDepth<float>>>& depthBuffer,
                       const glm::vec2& viewport)
    : m_positionBuffer{std::make_shared<gl::Texture2D<gl::RGB32F>>(viewport, "portal-position")}
    , m_perturbBuffer{std::make_shared<gl::Texture2D<gl::RGB32F>>(viewport, "portal-perturb")}
    , m_blur{"perturb", materialManager, 4, true}
{
  auto sampler = gslu::make_nn_unique<gl::Sampler>("portal-position-sampler");
  sampler->set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);

  m_positionBufferHandle
    = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::RGB32F>>>(m_positionBuffer, std::move(sampler));

  sampler = gslu::make_nn_unique<gl::Sampler>("portal-perturb-sampler");
  sampler->set(gl::api::TextureMinFilter::Linear).set(gl::api::TextureMagFilter::Linear);
  m_perturbBufferHandle
    = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::RGB32F>>>(m_perturbBuffer, std::move(sampler));
  m_blur.setInput(gsl::not_null{m_perturbBufferHandle});

  m_fb = gl::FrameBufferBuilder()
           .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_perturbBuffer)
           .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment1, m_positionBuffer)
           .textureNoBlend(gl::api::FramebufferAttachment::DepthAttachment, depthBuffer)
           .build("portal-fb");
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void PortalPass::bind(const gl::TextureHandle<gl::Texture2D<gl::RGB32F>>& position)
{
  gl::Framebuffer::unbindAll();
  m_positionBuffer->copyFrom(*position.getTexture());
  m_fb->bindWithAttachments();
}
} // namespace render::pass
