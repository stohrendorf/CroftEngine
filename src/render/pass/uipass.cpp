#include "uipass.h"

#include "config.h"
#include "render/material/materialmanager.h"
#include "render/material/rendermode.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"
#include "render/scene/translucency.h"

#include <algorithm>
#include <gl/constants.h>
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

namespace render::scene
{
class Node;
}

namespace render::pass
{
UIPass::UIPass(material::MaterialManager& materialManager,
               const glm::ivec2& renderViewport,
               const glm::ivec2& displayViewport)
    : m_material{materialManager.getFlat(true, false)}
    , m_mesh{scene::createScreenQuad(m_material, scene::Translucency::NonOpaque, "ui")}
    , m_colorBuffer{std::make_shared<gl::Texture2D<gl::SRGBA8>>(renderViewport, "ui-color")}
    , m_colorBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>(
        m_colorBuffer,
        gsl::make_unique<gl::Sampler>("ui-color" + gl::SamplerSuffix)
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Nearest) | set(gl::api::TextureMagFilter::Nearest))}
    , m_fb{gl::FrameBufferBuilder()
             .texture(gl::api::FramebufferAttachment::ColorAttachment0, m_colorBuffer)
             .build("ui-fb")}
{
  m_mesh->bind("u_input",
               [this](const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               {
                 uniform.set(gsl::not_null{m_colorBufferHandle});
               });

  m_mesh->getRenderState().merge(m_fb->getRenderState());
  m_mesh->getRenderState().setViewport(displayViewport);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void UIPass::render(const std::function<void()>& doRender)
{
  SOGLB_DEBUGGROUP("ui-pass");
  gsl_Assert(m_sync == nullptr);
  m_colorBuffer->clear({0, 0, 0, 0});
  m_fb->bind();
  doRender();
  m_fb->unbind();
  m_sync = std::make_unique<gl::FenceSync>();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void UIPass::render(float alpha)
{
  if(m_sync != nullptr)
  {
    m_sync->wait();
    m_sync.reset();
  }

  m_mesh->bind("u_alphaMultiplier",
               [alpha](const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               {
                 uniform.set(alpha);
               });
  scene::RenderContext context{material::RenderMode::FullNonOpaque, std::nullopt, scene::Translucency::NonOpaque};
  m_mesh->render(nullptr, context);

  if constexpr(FlushPasses)
    GL_ASSERT(gl::api::finish());
}
} // namespace render::pass
