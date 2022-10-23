#include "edgedetectionpass.h"

#include "config.h"
#include "geometrypass.h"
#include "render/material/material.h"
#include "render/material/materialmanager.h"
#include "render/material/rendermode.h"
#include "render/material/uniformparameter.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"

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
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <optional>

namespace render::scene
{
class Node;
}

namespace render::pass
{
EdgeDetectionPass::EdgeDetectionPass(material::MaterialManager& materialManager,
                                     const glm::ivec2& viewport,
                                     const GeometryPass& geometryPass)
    : m_edgeRenderMesh{scene::createScreenQuad(materialManager.getEdgeDetection(), "edge")}
    , m_edgeBuffer{std::make_shared<gl::Texture2D<gl::ScalarByte>>(viewport, "edge")}
    , m_edgeBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::ScalarByte>>>(
        m_edgeBuffer,
        gsl::make_unique<gl::Sampler>("edge-sampler")
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Nearest) | set(gl::api::TextureMagFilter::Nearest))}
    , m_edgeFb{gl::FrameBufferBuilder()
                 .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_edgeBuffer)
                 .build("edge-fb")}
    , m_dilationRenderMesh{scene::createScreenQuad(materialManager.getEdgeDilation(), "dilation")}
    , m_dilationBuffer{std::make_shared<gl::Texture2D<gl::ScalarByte>>(viewport, "dilation")}
    , m_dilationBufferHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::ScalarByte>>>(
        m_dilationBuffer,
        gsl::make_unique<gl::Sampler>("dilation-sampler")
          | set(gl::api::SamplerParameterI::TextureWrapS, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::SamplerParameterI::TextureWrapT, gl::api::TextureWrapMode::ClampToEdge)
          | set(gl::api::TextureMinFilter::Nearest) | set(gl::api::TextureMagFilter::Nearest))}
    , m_dilationFb{gl::FrameBufferBuilder()
                     .textureNoBlend(gl::api::FramebufferAttachment::ColorAttachment0, m_dilationBuffer)
                     .build("dilation-fb")}
{
  m_edgeRenderMesh->bind("u_normals",
                         [buffer = geometryPass.getNormalBuffer()](const render::scene::Node* /*node*/,
                                                                   const render::scene::Mesh& /*mesh*/,
                                                                   gl::Uniform& uniform)
                         {
                           uniform.set(buffer);
                         });
  m_edgeRenderMesh->bind("u_depth",
                         [buffer = geometryPass.getDepthBufferHandle()](const render::scene::Node* /*node*/,
                                                                        const render::scene::Mesh& /*mesh*/,
                                                                        gl::Uniform& uniform)
                         {
                           uniform.set(buffer);
                         });
  m_edgeRenderMesh->getRenderState().merge(m_edgeFb->getRenderState());

  m_dilationRenderMesh->bind("u_edges",
                             [buffer = m_edgeBufferHandle](const render::scene::Node* /*node*/,
                                                           const render::scene::Mesh& /*mesh*/,
                                                           gl::Uniform& uniform)
                             {
                               uniform.set(buffer);
                             });
  m_dilationRenderMesh->getRenderState().merge(m_dilationFb->getRenderState());
}

void EdgeDetectionPass::render()
{
  SOGLB_DEBUGGROUP("edge-pass");

  {
    SOGLB_DEBUGGROUP("edge-detection");
    m_edgeFb->bind();

    scene::RenderContext context{material::RenderMode::Full, std::nullopt};
    m_edgeRenderMesh->render(nullptr, context);
  }
  {
    SOGLB_DEBUGGROUP("edge-dilation");
    m_dilationFb->bind();

    scene::RenderContext context{material::RenderMode::Full, std::nullopt};
    m_dilationRenderMesh->render(nullptr, context);
  }

  if constexpr(FlushPasses)
    GL_ASSERT(gl::api::finish());
}
} // namespace render::pass
