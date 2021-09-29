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
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGB8>>(viewport, "fxaa-color")}
    , m_colorBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>>(
        m_colorBuffer,
        gslu::make_nn_unique<gl::Sampler>("fxaa-color")
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear))}
    , m_fb{gl::FrameBufferBuilder()
             .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
             .build("fxaa-fb")}
{
  m_mesh->bind("u_input",
               [buffer = geometryPass.getColorBuffer()](
                 const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               { uniform.set(buffer); });

  m_mesh->getRenderState().merge(m_fb->getRenderState());
}

void FXAAPass::render()
{
  SOGLB_DEBUGGROUP("fxaa-pass");

  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
  m_fb->bind();
  m_mesh->render(context);

  if constexpr(FlushPasses)
    GL_ASSERT(gl::api::finish());
}
} // namespace render::pass
