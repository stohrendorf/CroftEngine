#include "portalpass.h"

#include "geometrypass.h"
#include "gl/renderstate.h"
#include "render/scene/blur.h"

#include <functional>
#include <gl/constants.h>
#include <gl/debuggroup.h>
#include <gl/framebuffer.h>
#include <gl/pixel.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturehandle.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <limits>
#include <memory>
#include <string>

namespace render::material
{
class MaterialManager;
}

namespace render::pass
{
PortalPass::PortalPass(material::MaterialManager& materialManager,
                       const gslu::nn_shared<GeometryPass>& geometryPass,
                       const glm::vec2& viewport)
    : m_geometryPass{geometryPass}
    , m_positionBuffer{std::make_shared<gl::Texture2D<gl::Scalar32F>>(viewport, "portal-position")}
    , m_positionBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::Scalar32F>>>(
        m_positionBuffer,
        gsl::make_unique<gl::Sampler>("portal-position" + gl::SamplerSuffix)
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Nearest) | set(gl::api::TextureMagFilter::Nearest))}
    , m_perturbBuffer{std::make_shared<gl::Texture2D<gl::RGB32F>>(viewport, "portal-perturb")}
    , m_perturbBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::RGB32F>>>(
        m_perturbBuffer,
        gsl::make_unique<gl::Sampler>("portal-perturb" + gl::SamplerSuffix) | set(gl::api::TextureMinFilter::Linear)
          | set(gl::api::TextureMagFilter::Linear))}
    , m_blur{"perturb", materialManager, 4, true}
    , m_fb{gl::FrameBufferBuilder()
             .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_perturbBuffer)
             .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment1, m_positionBuffer)
             .textureNoBlend(gl::api::FramebufferAttachment::DepthAttachment, m_geometryPass->getDepthBuffer())
             .build("portal-fb")}
{
  m_blur.setInput(gsl::not_null{m_perturbBufferHandle});
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void PortalPass::render(const std::function<void(const gl::RenderState&)>& doRender)
{
  SOGLB_DEBUGGROUP("portal-pass");

  m_positionBuffer->clear(gl::Scalar32F{-std::numeric_limits<float>::infinity()});
  m_fb->bind();
  doRender(m_fb->getRenderState());
  m_fb->unbind();
}
} // namespace render::pass
