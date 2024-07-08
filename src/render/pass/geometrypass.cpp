#include "geometrypass.h"

#include <functional>
#include <gl/constants.h>
#include <gl/framebuffer.h>
#include <gl/pixel.h>
#include <gl/renderstate.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturedepth.h>
#include <gl/texturehandle.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>

namespace render::pass
{
GeometryPass::GeometryPass(const glm::ivec2& viewport)
    : m_depthBuffer{std::make_shared<gl::TextureDepth<float>>(viewport, "geometry-depth")}
    , m_depthBufferHandle{std::make_shared<gl::TextureHandle<gl::TextureDepth<float>>>(
        m_depthBuffer,
        gsl::make_unique<gl::Sampler>("depth" + gl::SamplerSuffix)
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear))}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "geometry-color")}
    , m_colorBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>(
        m_colorBuffer,
        gsl::make_unique<gl::Sampler>("geometry-color" + gl::SamplerSuffix)
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear))}
    , m_reflectiveBuffer{std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "geometry-reflective")}
    , m_reflectiveBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>(
        m_reflectiveBuffer,
        gsl::make_unique<gl::Sampler>("geometry-reflective" + gl::SamplerSuffix)
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear))}
    , m_positionBuffer{std::make_shared<gl::Texture2D<gl::RGB32F>>(viewport, "geometry-position")}
    , m_positionBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::RGB32F>>>(
        m_positionBuffer,
        gsl::make_unique<gl::Sampler>("geometry-position" + gl::SamplerSuffix)
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Nearest) | set(gl::api::TextureMagFilter::Nearest))}
    , m_normalBuffer{std::make_shared<gl::Texture2D<gl::RGB16F>>(viewport, "geometry-normal")}
    , m_normalBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::RGB16F>>>(
        m_normalBuffer,
        gsl::make_unique<gl::Sampler>("geometry-normal" + gl::SamplerSuffix)
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Nearest) | set(gl::api::TextureMagFilter::Nearest))}
    , m_fb{gl::FrameBufferBuilder()
             .texture(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
             .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment1, m_normalBuffer)
             .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment2, m_positionBuffer)
             .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment3, m_reflectiveBuffer)
             .textureNoBlend(gl::api::FramebufferAttachment::DepthAttachment, m_depthBuffer)
             .build("geometry-fb")}
{
}

GeometryPass::~GeometryPass() = default;

// NOLINTNEXTLINE(readability-make-member-function-const)
void GeometryPass::render(const std::function<void()>& doRender)
{
  m_fb->bind();
  gl::RenderState::resetWantedState();
  gl::RenderState::getWantedState().merge(m_fb->getRenderState());
  gl::RenderState::applyWantedState();
  doRender();
  //m_fb->unbind();
}
} // namespace render::pass
