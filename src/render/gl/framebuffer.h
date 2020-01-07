#pragma once

#include "texture.h"

#include <boost/log/trivial.hpp>
#include <utility>

namespace render::gl
{
class Framebuffer;

class TextureAttachment
{
  friend Framebuffer;

private:
  const std::shared_ptr<Texture> m_texture;
  const int32_t m_level;
  const ::gl::BlendingFactor m_srcBlend;
  const ::gl::BlendingFactor m_dstBlend;

  void attach(const Framebuffer& framebuffer, ::gl::FramebufferAttachment attachment) const;

public:
  explicit TextureAttachment(std::shared_ptr<Texture> texture,
                             const int32_t level = 0,
                             ::gl::BlendingFactor src = ::gl::BlendingFactor::SrcAlpha,
                             ::gl::BlendingFactor dst = ::gl::BlendingFactor::OneMinusSrcAlpha)
      : m_texture{std::move(texture)}
      , m_level{level}
      , m_srcBlend{src}
      , m_dstBlend{dst}
  {
  }

  ~TextureAttachment() = default;

  void bind(uint32_t buffer) const
  {
    GL_ASSERT(::gl::blendFunc(buffer, m_srcBlend, m_dstBlend));
  }
};

class Framebuffer : public BindableResource
{
public:
  using Attachment = std::pair<std::shared_ptr<render::gl::TextureAttachment>, ::gl::FramebufferAttachment>;
  using Attachments = std::vector<Attachment>;

private:
  Attachments m_attachments;

public:
  explicit Framebuffer(Attachments attachments, const std::string& label = {})
      : BindableResource{::gl::createFramebuffers,
                         [](const uint32_t handle) { bindFramebuffer(::gl::FramebufferTarget::Framebuffer, handle); },
                         ::gl::deleteFramebuffers,
                         ::gl::ObjectIdentifier::Framebuffer,
                         label}
      , m_attachments{std::move(attachments)}
  {
    std::vector<::gl::ColorBuffer> colorAttachments;
    for(const auto& attachment : m_attachments)
    {
      attachment.first->attach(*this, attachment.second);

      if(attachment.second >= ::gl::FramebufferAttachment::ColorAttachment0
         && attachment.second <= ::gl::FramebufferAttachment::ColorAttachment31)
        colorAttachments.emplace_back(static_cast<::gl::ColorBuffer>(attachment.second));
    }
    if(!colorAttachments.empty())
    {
      GL_ASSERT(::gl::namedFramebufferDrawBuffers(
        getHandle(), gsl::narrow<::gl::core::SizeType>(colorAttachments.size()), colorAttachments.data()));
    }
    else
    {
      GL_ASSERT(::gl::namedFramebufferDrawBuffer(getHandle(), ::gl::ColorBuffer::None));
      GL_ASSERT(::gl::namedFramebufferReadBuffer(getHandle(), ::gl::ColorBuffer::None));
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
      = GL_ASSERT_FN(::gl::checkNamedFramebufferStatus(getHandle(), ::gl::FramebufferTarget::Framebuffer));

#ifndef NDEBUG
    switch(result)
    {
    case ::gl::FramebufferStatus::FramebufferIncompleteAttachment:
      BOOST_LOG_TRIVIAL(warning) << "Framebuffer #" << getHandle() << " incomplete: incomplete attachment";
      break;
    case ::gl::FramebufferStatus::FramebufferUndefined:
      BOOST_LOG_TRIVIAL(warning) << "Framebuffer #" << getHandle() << " incomplete: default is undefined";
      break;
    case ::gl::FramebufferStatus::FramebufferIncompleteMissingAttachment:
      BOOST_LOG_TRIVIAL(warning) << "Framebuffer #" << getHandle() << " incomplete: missing attachment";
      break;
    case ::gl::FramebufferStatus::FramebufferIncompleteDrawBuffer:
      BOOST_LOG_TRIVIAL(warning) << "Framebuffer #" << getHandle() << " incomplete: incomplete draw buffer";
      break;
    case ::gl::FramebufferStatus::FramebufferIncompleteReadBuffer:
      BOOST_LOG_TRIVIAL(warning) << "Framebuffer #" << getHandle() << " incomplete: incomplete read buffer";
      break;
    case ::gl::FramebufferStatus::FramebufferUnsupported:
      BOOST_LOG_TRIVIAL(warning) << "Framebuffer #" << getHandle() << " incomplete: unsupported attachment type";
      break;
    case ::gl::FramebufferStatus::FramebufferIncompleteMultisample:
      BOOST_LOG_TRIVIAL(warning) << "Framebuffer #" << getHandle() << " incomplete: incomplete multisample";
      break;
    case ::gl::FramebufferStatus::FramebufferIncompleteLayerTargets:
      BOOST_LOG_TRIVIAL(warning) << "Framebuffer #" << getHandle() << " incomplete: incomplete layer targets";
      break;
    case ::gl::FramebufferStatus::FramebufferComplete: break;
    default:
      BOOST_LOG_TRIVIAL(error) << "Framebuffer #" << getHandle() << " incomplete: unknown code #"
                               << static_cast<::gl::core::EnumType>(result);
    }
#endif

    return result == ::gl::FramebufferStatus::FramebufferComplete;
  }

  void bindWithAttachments() const
  {
    BindableResource::bind();

    for(size_t i = 0; i < m_attachments.size(); ++i)
      m_attachments[i].first->bind(gsl::narrow_cast<uint32_t>(i));
  }

  static void unbindAll()
  {
    GL_ASSERT(::gl::bindFramebuffer(::gl::FramebufferTarget::Framebuffer, 0));
  }
};

inline void TextureAttachment::attach(const Framebuffer& framebuffer, ::gl::FramebufferAttachment attachment) const
{
  GL_ASSERT(::gl::namedFramebufferTexture(framebuffer.getHandle(), attachment, m_texture->getHandle(), m_level));
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

  FrameBufferBuilder& texture(::gl::FramebufferAttachment attachment,
                              const std::shared_ptr<Texture>& texture,
                              const int32_t level = 0,
                              ::gl::BlendingFactor src = ::gl::BlendingFactor::SrcAlpha,
                              ::gl::BlendingFactor dst = ::gl::BlendingFactor::OneMinusSrcAlpha)
  {
    m_attachments.emplace_back(std::make_shared<TextureAttachment>(texture, level, src, dst), attachment);
    return *this;
  }

  FrameBufferBuilder& textureNoBlend(::gl::FramebufferAttachment attachment,
                                     const std::shared_ptr<Texture>& texture,
                                     const int32_t level = 0)
  {
    m_attachments.emplace_back(
      std::make_shared<TextureAttachment>(texture, level, ::gl::BlendingFactor::One, ::gl::BlendingFactor::Zero),
      attachment);
    return *this;
  }
};
} // namespace render::gl
