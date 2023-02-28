#include "renderer.h"

#include "camera.h"
#include "node.h"
#include "render/material/rendermode.h"
#include "rendercontext.h"
#include "translucency.h"
#include "visitor.h"

#include <array>
#include <boost/log/trivial.hpp>
#include <gl/api/gl.hpp>
#include <gl/api/gl_api_provider.hpp>
#include <gl/glassert.h>
#include <gl/pixel.h>
#include <gl/renderstate.h>
#include <glm/mat4x4.hpp>
#include <optional>
#include <utility>

namespace render::scene
{
Renderer::Renderer(gslu::nn_shared<Camera> camera)
    : m_rootNode{std::make_shared<Node>("<rootnode>")}
    , m_camera{std::move(camera)}
{
}

Renderer::~Renderer() = default;

void Renderer::render()
{
  for(const auto translucencySelector : {Translucency::Opaque, Translucency::NonOpaque})
  {
    RenderContext context{translucencySelector == Translucency::Opaque ? material::RenderMode::FullOpaque
                                                                       : material::RenderMode::FullNonOpaque,
                          std::nullopt,
                          translucencySelector};
    Visitor visitor{context};
    m_rootNode->accept(visitor);
    visitor.render(m_camera->getPosition());
  }
}

void Renderer::clear(const gl::api::core::Bitfield<gl::api::ClearBufferMask>& flags,
                     const gl::SRGBA8& clearColor,
                     const float clearDepth)
{
  gl::api::core::Bitfield<gl::api::ClearBufferMask> bits;
  if(flags.isSet(gl::api::ClearBufferMask::ColorBufferBit))
  {
    if(clearColor != m_clearColor)
    {
      GL_ASSERT(gl::api::clearColor(clearColor.channels[0] / 255.0f,
                                    clearColor.channels[1] / 255.0f,
                                    clearColor.channels[2] / 255.0f,
                                    clearColor.channels[3] / 255.0f));
      m_clearColor = clearColor;
    }
    bits |= gl::api::ClearBufferMask::ColorBufferBit;
  }

  if(flags.isSet(gl::api::ClearBufferMask::DepthBufferBit))
  {
    if(clearDepth != m_clearDepth)
    {
      GL_ASSERT(gl::api::clearDepth(clearDepth));
      m_clearDepth = clearDepth;
    }
    bits |= gl::api::ClearBufferMask::DepthBufferBit;

    gl::RenderState::getWantedState().setDepthWrite(true);
    gl::RenderState::getWantedState().setDepthTest(true);
    gl::RenderState::applyWantedState();
  }

  GL_ASSERT(gl::api::clear(bits));
}

void Renderer::resetRootNode()
{
  m_rootNode = gsl::make_shared<Node>("<rootnode>");
}
} // namespace render::scene
