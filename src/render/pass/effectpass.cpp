#include "effectpass.h"

#include "config.h"
#include "render/renderpipeline.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"
#include "render/scene/rendermode.h"

#include <algorithm>
#include <gl/debuggroup.h>
#include <gl/framebuffer.h>
#include <gl/glassert.h>
#include <gl/pixel.h>
#include <gl/program.h>
#include <gl/renderstate.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturehandle.h>
#include <glm/mat4x4.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <optional>
#include <utility>

namespace render::scene
{
class Node;
}

namespace render::pass
{
EffectPass::EffectPass(gsl::not_null<const RenderPipeline*> renderPipeline,
                       std::string name,
                       gsl::not_null<std::shared_ptr<scene::Material>> material,
                       const gsl::not_null<std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>>>& input)
    : m_renderPipeline{std::move(renderPipeline)}
    , m_name{std::move(name)}
    , m_material{std::move(material)}
    , m_mesh{scene::createScreenQuad(m_material, m_name)}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGB8>>(input->getTexture()->size(), m_name + "-color")}
    , m_colorBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>>(
        m_colorBuffer,
        gslu::make_nn_unique<gl::Sampler>(m_name + "-color-sampler")
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear))}
    , m_fb{gl::FrameBufferBuilder()
             .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
             .build(m_name + "-fb")}
{
  m_mesh->bind("u_input",
               [input](const scene::Node& /*node*/, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               {
                 uniform.set(input);
               });

  m_mesh->getRenderState().merge(m_fb->getRenderState());
}

void EffectPass::render(bool inWater)
{
  SOGLB_DEBUGGROUP(m_name + "-pass");

  m_fb->bind();

  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
  m_mesh->bind("u_inWater",
               [inWater](const scene::Node& /*node*/, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               {
                 uniform.set(inWater ? 1.0f : 0.0f);
               });
  m_mesh->bind("u_time",
               [this](const scene::Node&, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               {
                 const auto now = m_renderPipeline->getLocalTime();
                 uniform.set(gsl::narrow_cast<float>(now.count()));
               });
  m_mesh->render(context);

  if constexpr(FlushPasses)
    GL_ASSERT(gl::api::finish());
}
} // namespace render::pass
