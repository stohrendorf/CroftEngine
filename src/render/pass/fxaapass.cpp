#include "fxaapass.h"

#include "config.h"
#include "geometrypass.h"
#include "render/scene/materialmanager.h"
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
FXAAPass::FXAAPass(scene::MaterialManager& materialManager,
                   const glm::ivec2& viewport,
                   const GeometryPass& geometryPass)
    : m_material{materialManager.getFXAA()}
    , m_mesh{scene::createScreenQuad(m_material, "fxaa")}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGBA8>>(viewport, "fxaa-color")}
{
  auto sampler = gslu::make_nn_unique<gl::Sampler>("fxaa-color");
  sampler->set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
    .set(gl::api::TextureMinFilter::Linear)
    .set(gl::api::TextureMagFilter::Linear);
  m_colorBufferHandle
    = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>(m_colorBuffer, std::move(sampler));

  m_mesh->bind("u_input",
               [buffer = geometryPass.getColorBuffer()](
                 const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               { uniform.set(buffer); });

  m_fb = gl::FrameBufferBuilder()
           .texture(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
           .build("fxaa-fb");
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void FXAAPass::bind()
{
  m_fb->bindWithAttachments();
}

void FXAAPass::render(const glm::ivec2& size)
{
  SOGLB_DEBUGGROUP("fxaa-pass");
  bind();

  gl::RenderState::resetWantedState();
  gl::RenderState::getWantedState().setBlend(false);
  gl::RenderState::getWantedState().setViewport(size);
  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};

  m_mesh->render(context);

  if constexpr(FlushPasses)
    GL_ASSERT(gl::api::finish());
}
} // namespace render::pass
