#include "framebuffer.h"

#include "glassert.h"
#include "renderstate.h"
#include "texture.h"

#include <boost/log/trivial.hpp>
#include <cstdint>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <string_view>
#include <utility>
#include <vector>

namespace gl
{
void TextureAttachment::attach(const Framebuffer& framebuffer, const api::FramebufferAttachment attachment) const
{
  GL_ASSERT(api::namedFramebufferTexture(framebuffer.getHandle(), attachment, m_texture->getHandle(), m_level));
}

Framebuffer::Framebuffer(Framebuffer::Attachments attachments,
                         const std::string_view& label,
                         RenderState&& renderState,
                         const glm::ivec2& size)
    : BindableResource{api::createFramebuffers,
                       [](const uint32_t handle)
                       {
                         GL_ASSERT(memoryBarrier(api::MemoryBarrierMask::FramebufferBarrierBit));
                         GL_ASSERT(bindFramebuffer(api::FramebufferTarget::DrawFramebuffer, handle));
                       },
                       api::deleteFramebuffers,
                       label}
    , m_attachments{std::move(attachments)}
    , m_renderState{std::move(renderState)}
    , m_size{size}
{
  std::vector<api::ColorBuffer> colorAttachments;
  for(const auto& [attachment, slot] : m_attachments)
  {
    attachment->attach(*this, slot);

    if(slot >= api::FramebufferAttachment::ColorAttachment0 && slot <= api::FramebufferAttachment::ColorAttachment31)
      colorAttachments.emplace_back(static_cast<api::ColorBuffer>(slot));
  }
  if(!colorAttachments.empty())
  {
    GL_ASSERT(api::namedFramebufferDrawBuffers(
      getHandle(), gsl::narrow<api::core::SizeType>(colorAttachments.size()), colorAttachments.data()));
  }
  else
  {
    GL_ASSERT(api::namedFramebufferDrawBuffer(getHandle(), api::ColorBuffer::None));
    GL_ASSERT(api::namedFramebufferReadBuffer(getHandle(), api::ColorBuffer::None));
  }

  gsl_Expects(isComplete());
}

bool Framebuffer::isComplete() const
{
  const auto result = GL_ASSERT_FN(api::checkNamedFramebufferStatus(getHandle(), api::FramebufferTarget::Framebuffer));

#ifndef NDEBUG
  switch(result)
  {
  case api::FramebufferStatus::FramebufferIncompleteAttachment:
    BOOST_LOG_TRIVIAL(warning) << "Framebuffer #" << getHandle() << " incomplete: incomplete attachment";
    break;
  case api::FramebufferStatus::FramebufferUndefined:
    BOOST_LOG_TRIVIAL(warning) << "Framebuffer #" << getHandle() << " incomplete: default is undefined";
    break;
  case api::FramebufferStatus::FramebufferIncompleteMissingAttachment:
    BOOST_LOG_TRIVIAL(warning) << "Framebuffer #" << getHandle() << " incomplete: missing attachment";
    break;
  case api::FramebufferStatus::FramebufferIncompleteDrawBuffer:
    BOOST_LOG_TRIVIAL(warning) << "Framebuffer #" << getHandle() << " incomplete: incomplete draw buffer";
    break;
  case api::FramebufferStatus::FramebufferIncompleteReadBuffer:
    BOOST_LOG_TRIVIAL(warning) << "Framebuffer #" << getHandle() << " incomplete: incomplete read buffer";
    break;
  case api::FramebufferStatus::FramebufferUnsupported:
    BOOST_LOG_TRIVIAL(warning) << "Framebuffer #" << getHandle() << " incomplete: unsupported attachment type";
    break;
  case api::FramebufferStatus::FramebufferIncompleteMultisample:
    BOOST_LOG_TRIVIAL(warning) << "Framebuffer #" << getHandle() << " incomplete: incomplete multisample";
    break;
  case api::FramebufferStatus::FramebufferIncompleteLayerTargets:
    BOOST_LOG_TRIVIAL(warning) << "Framebuffer #" << getHandle() << " incomplete: incomplete layer targets";
    break;
  case api::FramebufferStatus::FramebufferComplete:
    break;
  default:
    BOOST_LOG_TRIVIAL(error) << "Framebuffer #" << getHandle() << " incomplete: unknown code #"
                             << static_cast<api::core::EnumType>(result);
  }
#endif

  return result == api::FramebufferStatus::FramebufferComplete;
}

void Framebuffer::blit(const Framebuffer& target, api::BlitFramebufferFilter filter)
{
  GL_ASSERT(api::memoryBarrier(api::MemoryBarrierMask::FramebufferBarrierBit));
  GL_ASSERT(api::blitNamedFramebuffer(getHandle(),
                                      target.getHandle(),
                                      0,
                                      0,
                                      m_size.x - 1,
                                      m_size.y - 1,
                                      0,
                                      0,
                                      target.m_size.x - 1,
                                      target.m_size.y - 1,
                                      api::ClearBufferMask::ColorBufferBit,
                                      filter));
}

void Framebuffer::blit(const glm::ivec2& backbufferSize, api::BlitFramebufferFilter filter)
{
  GL_ASSERT(api::blitNamedFramebuffer(getHandle(),
                                      0,
                                      0,
                                      0,
                                      m_size.x - 1,
                                      m_size.y - 1,
                                      0,
                                      0,
                                      backbufferSize.x - 1,
                                      backbufferSize.y - 1,
                                      api::ClearBufferMask::ColorBufferBit,
                                      filter));
}

gslu::nn_shared<Framebuffer> FrameBufferBuilder::build(const std::string_view& label)
{
  gsl_Expects(!m_attachments.empty());

  glm::ivec2 size{-1, -1};
  for(const auto& [attachment, _] : m_attachments)
  {
    const auto attachmentSize = attachment->getTextureSize();
    if(size == glm::ivec2{-1, -1})
      size = attachmentSize;
    else
      gsl_Assert(size == attachmentSize);
  }

  m_renderState.setViewport(size);
  return gsl::make_shared<Framebuffer>(std::move(m_attachments), label, std::move(m_renderState), size);
}
} // namespace gl
