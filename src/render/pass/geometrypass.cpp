#include "geometrypass.h"

#include <gl/framebuffer.h>
#include <gl/renderstate.h>
#include <gl/texture2d.h>
#include <gl/texturedepth.h>

namespace render::pass
{
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
  m_depthBuffer->set(gl::api::TextureMinFilter::Linear).set(gl::api::TextureMagFilter::Linear);
  m_colorBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);
  m_positionBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Nearest)
    .set(gl::api::TextureMagFilter::Nearest);
  m_normalBuffer->set(gl::api::TextureParameterName::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureParameterName::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Nearest)
    .set(gl::api::TextureMagFilter::Nearest);

  m_fb = gl::FrameBufferBuilder()
           .texture(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
           .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment1, m_normalBuffer)
           .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment2, m_positionBuffer)
           .textureNoBlend(gl::api::FramebufferAttachment::DepthAttachment, m_depthBuffer)
           .build("geometry-fb");
}
} // namespace render::pass
