#include "renderer.h"

#include "camera.h"
#include "rendercontext.h"

#include <utility>

namespace render::scene
{
Renderer::Renderer(gsl::not_null<std::shared_ptr<Camera>> camera)
    : m_rootNode{std::make_shared<Node>("<rootnode>")}
    , m_camera{std::move(camera)}
{
}

Renderer::~Renderer() = default;

void Renderer::render()
{
  RenderContext context{RenderMode::Full, std::nullopt};
  Visitor visitor{context};
  m_rootNode->accept(visitor);

  // Update FPS.
  ++m_frameCount;
  const auto t = getGameTime();
  const auto dt = t - m_frameLastFPS;
  if(dt >= std::chrono::seconds(1))
  {
    m_frameRate
      = std::exchange(m_frameCount, 0) * 1000.0f / std::chrono::duration_cast<std::chrono::milliseconds>(dt).count();
    m_frameLastFPS = t;
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

    // We need to explicitly call the static enableDepthWrite() method on StateBlock
    // to ensure depth writing is enabled before clearing the depth buffer (and to
    // update the global StateBlock render state to reflect this).
    gl::RenderState::enableDepthWrite();
  }

  GL_ASSERT(gl::api::clear(bits));
}

void Renderer::resetRootNode()
{
  m_rootNode = std::make_shared<Node>("<rootnode>");
}
} // namespace render::scene
