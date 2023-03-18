#include "hbaopass.h"

#include "config.h"
#include "geometrypass.h"
#include "render/material/material.h"
#include "render/material/materialmanager.h"
#include "render/material/rendermode.h"
#include "render/material/uniformparameter.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"
#include "render/scene/translucency.h"

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
#include <string>

namespace render::scene
{
class Node;
}

namespace render::pass
{
HBAOPass::HBAOPass(material::MaterialManager& materialManager,
                   const glm::ivec2& viewport,
                   const gslu::nn_shared<GeometryPass>& geometryPass)
    : m_geometryPass{geometryPass}
    , m_material{materialManager.getHBAO()}
    , m_renderMesh{scene::createScreenQuad(m_material, scene::Translucency::Opaque, "hbao")}
    , m_aoBuffer{std::make_shared<gl::Texture2D<gl::ScalarByte>>(viewport, "hbao-ao")}
    , m_aoBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::ScalarByte>>>(
        m_aoBuffer,
        gsl::make_unique<gl::Sampler>("hbao-ao" + gl::SamplerSuffix)
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Linear) | set(gl::api::TextureMagFilter::Linear))}
    , m_blur{"hbao", materialManager, 2, false}
    , m_fb{gl::FrameBufferBuilder()
             .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_aoBuffer)
             .build("hbao-fb")}
{
  m_renderMesh->bind("u_normals",
                     [buffer = m_geometryPass->getNormalBuffer()](
                       const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                     {
                       uniform.set(buffer);
                     });
  m_renderMesh->bind("u_position",
                     [buffer = m_geometryPass->getPositionBuffer()](
                       const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                     {
                       uniform.set(buffer);
                     });
  m_renderMesh->getRenderState().merge(m_fb->getRenderState());

  m_blur.setInput(gsl::not_null{m_aoBufferHandle});
}

void HBAOPass::updateCamera(const gslu::nn_shared<scene::Camera>& camera)
{
  m_material->getUniformBlock("Camera")->bindCameraBuffer(camera);
}

void HBAOPass::render()
{
  SOGLB_DEBUGGROUP("hbao-pass");
  m_geometryPass->wait();

  m_fb->bind();
  scene::RenderContext context{material::RenderMode::FullOpaque, std::nullopt, scene::Translucency::Opaque};
  m_renderMesh->render(nullptr, context);
  m_fb->unbind();

  gl::FenceSync::sync();
  m_blur.render();

  if constexpr(FlushPasses)
    GL_ASSERT(gl::api::finish());
}
} // namespace render::pass
