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
  const std::shared_ptr<Texture> m_texture;
  const int32_t m_level;
  const api::BlendingFactor m_srcBlend;
  const api::BlendingFactor m_dstBlend;

  void attach(const Framebuffer& framebuffer, api::FramebufferAttachment attachment) const;

public:
  explicit TextureAttachment(std::shared_ptr<Texture> texture,
                             const int32_t level = 0,
                             api::BlendingFactor src = api::BlendingFactor::SrcAlpha,
                             api::BlendingFactor dst = api::BlendingFactor::OneMinusSrcAlpha)
      : m_texture{std::move(texture)}
      , m_level{level}
      , m_srcBlend{src}
      , m_dstBlend{dst}
  {
  }

  ~TextureAttachment() = default;

  void bind(uint32_t buffer) const
  {
    GL_ASSERT(api::blendFunc(buffer, m_srcBlend, m_dstBlend));
  }
};

class Framebuffer : public BindableResource
{
public:
  using Attachment = std::pair<std::shared_ptr<TextureAttachment>, api::FramebufferAttachment>;
  using Attachments = std::vector<Attachment>;

private:
  Attachments m_attachments;

public:
  explicit Framebuffer(Attachments attachments, const std::string& label = {})
      : BindableResource{api::createFramebuffers,
                         [](const uint32_t handle) { bindFramebuffer(api::FramebufferTarget::Framebuffer, handle); },
                         api::deleteFramebuffers,
                         api::ObjectIdentifier::Framebuffer,
                         label}
      , m_attachments{std::move(attachments)}
  {
    std::vector<api::ColorBuffer> colorAttachments;
    for(const auto& attachment : m_attachments)
    {
      attachment.first->attach(*this, attachment.second);

      if(attachment.second >= api::FramebufferAttachment::ColorAttachment0
         && attachment.second <= api::FramebufferAttachment::ColorAttachment31)
        colorAttachments.emplace_back(static_cast<api::ColorBuffer>(attachment.second));
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
    BindableResource::bind();

    for(size_t i = 0; i < m_attachments.size(); ++i)
      m_attachments[i].first->bind(gsl::narrow_cast<uint32_t>(i));
  }

  static void unbindAll()
  {
    GL_ASSERT(api::bindFramebuffer(api::FramebufferTarget::Framebuffer, 0));
  }

  void invalidate()
  {
    std::vector<gl::api::FramebufferAttachment> attachments;
    attachments.reserve(m_attachments.size());
    std::transform(m_attachments.begin(), m_attachments.end(), std::back_inserter(attachments), [](const auto& src) {
      return src.second;
    });
    gl::api::invalidateNamedFramebufferData(
      getHandle(), gsl::narrow<gl::api::core::SizeType>(attachments.size()), attachments.data());
  }
};

inline void TextureAttachment::attach(const Framebuffer& framebuffer, api::FramebufferAttachment attachment) const
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
                              const std::shared_ptr<Texture>& texture,
                              const int32_t level = 0,
                              api::BlendingFactor src = api::BlendingFactor::SrcAlpha,
                              api::BlendingFactor dst = api::BlendingFactor::OneMinusSrcAlpha)
  {
    m_attachments.emplace_back(std::make_shared<TextureAttachment>(texture, level, src, dst), attachment);
    return *this;
  }

  FrameBufferBuilder& textureNoBlend(api::FramebufferAttachment attachment,
                                     const std::shared_ptr<Texture>& texture,
                                     const int32_t level = 0)
  {
    m_attachments.emplace_back(
      std::make_shared<TextureAttachment>(texture, level, api::BlendingFactor::One, api::BlendingFactor::Zero),
      attachment);
    return *this;
  }
};
} // namespace gl
