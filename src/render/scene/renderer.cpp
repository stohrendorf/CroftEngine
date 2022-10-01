#include "renderer.h"

#include "camera.h"
#include "node.h"
#include "rendercontext.h"
#include "rendermode.h"
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
  {
    RenderContext context{RenderMode::Full, std::nullopt};
    Visitor visitor{context};
    m_rootNode->accept(visitor);
    visitor.render(m_camera->getPosition());
  }

  const auto t = std::chrono::high_resolution_clock::now();
  const auto dt = t - m_lastLogTime;
  if(dt >= std::chrono::seconds(5))
  {
    m_lastLogTime = t;

    if(GLAD_GL_ATI_meminfo)
    {
      std::array<GLint, 4> tmp{};
      auto logStats = [&tmp](const char* prefix)
      {
        BOOST_LOG_TRIVIAL(debug) << prefix << ": total free " << tmp[0] / 1024 << ", largest free block "
                                 << tmp[1] / 1024 << ", total auxiliary free " << tmp[2] / 1024
                                 << ", largest auxiliary free " << tmp[3] / 1024;
      };

      tmp.fill(0);
      GL_ASSERT(glGetIntegerv(GL_VBO_FREE_MEMORY_ATI, tmp.data()));
      logStats("VBO");
      tmp.fill(0);
      GL_ASSERT(glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, tmp.data()));
      logStats("Texture");
      tmp.fill(0);
      GL_ASSERT(glGetIntegerv(GL_RENDERBUFFER_FREE_MEMORY_ATI, tmp.data()));
      logStats("Renderbuffer");
    }
    else if(GLAD_GL_NVX_gpu_memory_info)
    {
      GLint dedicated;
      GL_ASSERT(glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &dedicated));
      GLint total;
      GL_ASSERT(glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &total));
      GLint available;
      GL_ASSERT(glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &available));
      GLint evictionCount;
      GL_ASSERT(glGetIntegerv(GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &evictionCount));
      GLint evicted;
      GL_ASSERT(glGetIntegerv(GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, &evicted));

      BOOST_LOG_TRIVIAL(debug) << "dedicated " << dedicated / 1024 << ", total " << total / 1024 << ", available "
                               << available / 1024 << ", eviction count " << evictionCount << ", evicted "
                               << evicted / 1024;
    }
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
