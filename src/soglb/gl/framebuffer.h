#pragma once

#include "texture.h"

#include <boost/log/trivial.hpp>
#include <utility>

namespace gl
{
class Framebuffer;

class TextureAttachment
{
  friend Framebuffer;

private:
  const gsl::not_null<std::shared_ptr<Texture>> m_texture;
  const int32_t m_level;
  const bool m_alphaBlend;

  void attach(const Framebuffer& framebuffer, api::FramebufferAttachment attachment) const;

public:
  explicit TextureAttachment(gsl::not_null<std::shared_ptr<Texture>> texture,
                             const int32_t level = 0,
                             bool alphaBlend = true)
      : m_texture{std::move(texture)}
      , m_level{level}
      , m_alphaBlend{alphaBlend}
  {
  }

  ~TextureAttachment() = default;

  void setAlphaBlend(const uint32_t slot) const
  {
    if(m_alphaBlend)
      GL_ASSERT(api::enable(api::EnableCap::Blend, slot));
    else
      GL_ASSERT(api::disable(api::EnableCap::Blend, slot));
  }
};

class Framebuffer final : public BindableResource
{
public:
  using Attachment = std::pair<gsl::not_null<std::shared_ptr<TextureAttachment>>, api::FramebufferAttachment>;
  using Attachments = std::vector<Attachment>;

private:
  Attachments m_attachments;

public:
  explicit Framebuffer(Attachments attachments, const std::string& label = {})
      : BindableResource{api::createFramebuffers,
                         [](const uint32_t handle)
                         { bindFramebuffer(api::FramebufferTarget::DrawFramebuffer, handle); },
                         api::deleteFramebuffers,
                         api::ObjectIdentifier::Framebuffer,
                         label}
      , m_attachments{std::move(attachments)}
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

    Expects(isComplete());
  }

  [[nodiscard]] const Attachments& getAttachments() const
  {
    return m_attachments;
  }

  [[nodiscard]] bool isComplete() const
  {
    const auto result
      = GL_ASSERT_FN(api::checkNamedFramebufferStatus(getHandle(), api::FramebufferTarget::Framebuffer));

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
    case api::FramebufferStatus::FramebufferComplete: break;
    default:
      BOOST_LOG_TRIVIAL(error) << "Framebuffer #" << getHandle() << " incomplete: unknown code #"
                               << static_cast<api::core::EnumType>(result);
    }
#endif

    return result == api::FramebufferStatus::FramebufferComplete;
  }

  void bindWithAttachments() const
  {
    bind();

    for(const auto& [attachment, slot] : m_attachments)
    {
      if(slot >= api::FramebufferAttachment::ColorAttachment0 && slot <= api::FramebufferAttachment::ColorAttachment31)
        attachment->setAlphaBlend(static_cast<uint32_t>(slot)
                                  - static_cast<uint32_t>(api::FramebufferAttachment::ColorAttachment0));
    }
  }

  static void unbindAll()
  {
    GL_ASSERT(api::bindFramebuffer(api::FramebufferTarget::Framebuffer, 0));
  }

  void invalidate()
  {
    std::vector<api::FramebufferAttachment> attachments;
    attachments.reserve(m_attachments.size());
    std::transform(m_attachments.begin(),
                   m_attachments.end(),
                   std::back_inserter(attachments),
                   [](const auto& src) { return src.second; });
    invalidateNamedFramebufferData(
      getHandle(), gsl::narrow<api::core::SizeType>(attachments.size()), attachments.data());
  }
};

inline void TextureAttachment::attach(const Framebuffer& framebuffer, const api::FramebufferAttachment attachment) const
{
  GL_ASSERT(api::namedFramebufferTexture(framebuffer.getHandle(), attachment, m_texture->getHandle(), m_level));
}

class FrameBufferBuilder
{
private:
  Framebuffer::Attachments m_attachments;

public:
  std::shared_ptr<Framebuffer> build(const std::string& label = {})
  {
    return std::make_shared<Framebuffer>(std::move(m_attachments), label);
  }

  FrameBufferBuilder& texture(api::FramebufferAttachment attachment,
                              const gsl::not_null<std::shared_ptr<Texture>>& texture,
                              const int32_t level = 0,
                              bool alphaBlend = true)
  {
    m_attachments.emplace_back(std::make_shared<TextureAttachment>(texture, level, alphaBlend), attachment);
    return *this;
  }

  FrameBufferBuilder& textureNoBlend(api::FramebufferAttachment attachment,
                                     const gsl::not_null<std::shared_ptr<Texture>>& texture,
                                     const int32_t level = 0)
  {
    m_attachments.emplace_back(std::make_shared<TextureAttachment>(texture, level, false), attachment);
    return *this;
  }
};
} // namespace gl
