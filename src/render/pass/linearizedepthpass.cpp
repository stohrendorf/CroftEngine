#include "linearizedepthpass.h"

#include "config.h"
#include "render/scene/material.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"
#include "render/scene/rendermode.h"
#include "render/scene/uniformparameter.h"

#include <algorithm>
#include <gl/debuggroup.h>
#include <gl/framebuffer.h>
#include <gl/glassert.h>
#include <gl/pixel.h>
#include <gl/program.h>
#include <gl/renderstate.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturedepth.h> // IWYU pragma: keep
#include <gl/texturehandle.h>
#include <glm/mat4x4.hpp>
#include <gsl/gsl-lite.hpp>
#include <optional>
#include <utility>

namespace render::scene
{
class Node;
}

namespace render::pass
{
LinearizeDepthPass::LinearizeDepthPass(scene::MaterialManager& materialManager,
                                       const glm::ivec2& viewport,
                                       const std::shared_ptr<gl::TextureHandle<gl::TextureDepth<float>>>& depth,
                                       const std::string& prefix)
    : m_material{materialManager.getLinearDepth()}
    , m_renderMesh{scene::createScreenQuad(m_material, "linearize-depth")}
{
  m_renderMesh->bind(
    "u_depth",
    [depth](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
    { uniform.set(depth); });

  auto sampler = std::make_unique<gl::Sampler>("linearize-depth-sampler");
  sampler->set(gl::api::TextureMinFilter::Nearest).set(gl::api::TextureMagFilter::Nearest);
  m_linearDepth = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::Scalar32F>>>(
    std::make_shared<gl::Texture2D<gl::Scalar32F>>(viewport, prefix + "linearize-depth-texture"), std::move(sampler));

  m_fb = gl::FrameBufferBuilder()
           .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_linearDepth->getTexture())
           .build("linearize-depth-fb");
}

void LinearizeDepthPass::render()
{
  SOGLB_DEBUGGROUP("linearize-depth-pass");
  m_fb->bindWithAttachments();

  gl::RenderState::resetWantedState();
  gl::RenderState::getWantedState().setBlend(false);
  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};

  m_renderMesh->render(context);

  if constexpr(FlushPasses)
    GL_ASSERT(gl::api::finish());
}

void LinearizeDepthPass::updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera)
{
  m_material->getUniformBlock("Camera")->bindCameraBuffer(camera);
}
} // namespace render::pass
