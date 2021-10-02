#include "hbaopass.h"

#include "config.h"
#include "geometrypass.h"
#include "render/scene/material.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"
#include "render/scene/rendermode.h"
#include "render/scene/uniformparameter.h"

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
HBAOPass::HBAOPass(scene::MaterialManager& materialManager,
                   const glm::ivec2& viewport,
                   const GeometryPass& geometryPass)
    : m_material{materialManager.getHBAO()}
    , m_renderMesh{scene::createScreenQuad(m_material, "hbao")}
    , m_aoBuffer{std::make_shared<gl::Texture2D<gl::ScalarByte>>(viewport, "hbao-ao")}
    , m_aoBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::ScalarByte>>>(
        m_aoBuffer,
        gslu::make_nn_unique<gl::Sampler>("hbao-ao-sampler")
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear))}
    , m_blur{"hbao", materialManager, 2, false}
    , m_fb{gl::FrameBufferBuilder()
             .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_aoBuffer)
             .build("hbao-fb")}
{
  m_renderMesh->bind("u_normals",
                     [buffer = geometryPass.getNormalBuffer()](
                       const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                     { uniform.set(buffer); });
  m_renderMesh->bind("u_position",
                     [buffer = geometryPass.getPositionBuffer()](
                       const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                     { uniform.set(buffer); });
  m_renderMesh->getRenderState().merge(m_fb->getRenderState());

  m_blur.setInput(gsl::not_null{m_aoBufferHandle});
}

void HBAOPass::updateCamera(const gsl::not_null<std::shared_ptr<scene::Camera>>& camera)
{
  m_material->getUniformBlock("Camera")->bindCameraBuffer(camera);
}

void HBAOPass::render()
{
  SOGLB_DEBUGGROUP("hbao-pass");
  m_fb->bind();

  scene::RenderContext context{scene::RenderMode::Full, std::nullopt};
  m_renderMesh->render(context);
  m_blur.render();

  if constexpr(FlushPasses)
    GL_ASSERT(gl::api::finish());
}
} // namespace render::pass
