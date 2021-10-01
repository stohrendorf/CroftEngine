#include "geometrypass.h"

#include <gl/framebuffer.h>
#include <gl/pixel.h>
#include <gl/renderstate.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturedepth.h>
#include <gl/texturehandle.h>
#include <gslu.h>
#include <utility>

namespace render::pass
{
GeometryPass::GeometryPass(const glm::ivec2& viewport)
    : m_depthBuffer{std::make_shared<gl::TextureDepth<float>>(viewport, "geometry-depth")}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGB8>>(viewport, "geometry-color")}
    , m_colorBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>>(
        m_colorBuffer,
        gslu::make_nn_unique<gl::Sampler>("geometry-color")
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear))}
    , m_positionBuffer{std::make_shared<gl::Texture2D<gl::RGB32F>>(viewport, "geometry-position")}
    , m_positionBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::RGB32F>>>(
        m_positionBuffer,
        gslu::make_nn_unique<gl::Sampler>("geometry-position")
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Nearest) | set(gl::api::TextureMagFilter::Nearest))}
    , m_interpolatedPositionBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::RGB32F>>>(
        m_positionBuffer,
        gslu::make_nn_unique<gl::Sampler>("geometry-position-interpolated")
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear))}
    , m_normalBuffer{std::make_shared<gl::Texture2D<gl::RGB16F>>(viewport, "geometry-normal")}
    , m_normalBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::RGB16F>>>(
        m_normalBuffer,
        gslu::make_nn_unique<gl::Sampler>("geometry-normal")
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Nearest) | set(gl::api::TextureMagFilter::Nearest))}
    , m_fb{gl::FrameBufferBuilder()
             .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
             .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment1, m_normalBuffer)
             .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment2, m_positionBuffer)
             .textureNoBlend(gl::api::FramebufferAttachment::DepthAttachment, m_depthBuffer)
             .build("geometry-fb")}
{
}

GeometryPass::~GeometryPass() = default;

// NOLINTNEXTLINE(readability-make-member-function-const)
void GeometryPass::bind()
{
  m_fb->bind();
  gl::RenderState::getWantedState() = m_fb->getRenderState();
  gl::RenderState::applyWantedState();
}
} // namespace render::pass
