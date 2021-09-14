#include "geometrypass.h"

#include <gl/framebuffer.h>
#include <gl/pixel.h>
#include <gl/renderstate.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturedepth.h>
#include <gl/texturehandle.h>
#include <utility>

namespace render::pass
{
GeometryPass::~GeometryPass() = default;

// NOLINTNEXTLINE(readability-make-member-function-const)
void GeometryPass::bind(const glm::ivec2& size)
{
  gl::Framebuffer::unbindAll();
  gl::RenderState::getWantedState().setViewport(size);
  m_fb->bindWithAttachments();
}

GeometryPass::GeometryPass(const glm::ivec2& viewport)
    : m_depthBuffer{std::make_shared<gl::TextureDepth<float>>(viewport, "geometry-depth")}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "geometry-color")}
    , m_positionBuffer{std::make_shared<gl::Texture2D<gl::RGB32F>>(viewport, "geometry-position")}
    , m_normalBuffer{std::make_shared<gl::Texture2D<gl::RGB16F>>(viewport, "geometry-normal")}
{
  auto sampler = std::make_unique<gl::Sampler>("geometry-depth");
  sampler->set(gl::api::TextureMinFilter::Linear).set(gl::api::TextureMagFilter::Linear);
  m_depthBufferHandle = std::make_shared<gl::TextureHandle<gl::TextureDepth<float>>>(m_depthBuffer, std::move(sampler));

  sampler = std::make_unique<gl::Sampler>("geometry-color");
  sampler->set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);
  m_colorBufferHandle
    = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>(m_colorBuffer, std::move(sampler));

  sampler = std::make_unique<gl::Sampler>("geometry-position");
  sampler->set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Nearest)
    .set(gl::api::TextureMagFilter::Nearest);
  m_positionBufferHandle
    = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::RGB32F>>>(m_positionBuffer, std::move(sampler));

  sampler = std::make_unique<gl::Sampler>("geometry-normal");
  sampler->set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Nearest)
    .set(gl::api::TextureMagFilter::Nearest);
  m_normalBufferHandle
    = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::RGB16F>>>(m_normalBuffer, std::move(sampler));

  m_fb = gl::FrameBufferBuilder()
           .texture(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
           .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment1, m_normalBuffer)
           .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment2, m_positionBuffer)
           .textureNoBlend(gl::api::FramebufferAttachment::DepthAttachment, m_depthBuffer)
           .build("geometry-fb");
}
} // namespace render::pass
