#include "framebuffer.h"

#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"

#include <algorithm>
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
Framebuffer::Framebuffer(const std::string& name,
                         gsl::not_null<std::shared_ptr<scene::Material>> material,
                         const glm::ivec2& size)
    : m_material{std::move(material)}
    , m_mesh{scene::createScreenQuad(m_material, name)}
    , m_depthBuffer{std::make_shared<gl::TextureDepth<float>>(size, name + "-depth")}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGBA8>>(size, name + "-color")}
    , m_colorBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>(
        m_colorBuffer,
        gslu::make_nn_unique<gl::Sampler>(name + "-color-sampler")
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear))}
    , m_fb{gl::FrameBufferBuilder()
             .texture(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
             .textureNoBlend(gl::api::FramebufferAttachment::DepthAttachment, m_depthBuffer)
             .build(name + "-fb")}
{
  m_mesh->bind("u_input",
               [this](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               {
                 uniform.set(m_colorBufferHandle);
               });
  m_mesh->getRenderState().setBlend(0, true);
  m_mesh->getRenderState().setBlendFactors(
    0, gl::api::BlendingFactor::SrcAlpha, gl::api::BlendingFactor::OneMinusSrcAlpha);
}

void Framebuffer::bind()
{
  m_fb->bind();
  gl::RenderState::getWantedState().merge(m_fb->getRenderState());
  gl::RenderState::applyWantedState();
}

void Framebuffer::render()
{
  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
  m_mesh->render(context);
}
} // namespace render::pass
