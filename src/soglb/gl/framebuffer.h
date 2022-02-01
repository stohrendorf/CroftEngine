#pragma once

#include "bindableresource.h"
#include "renderstate.h"
#include "soglb_fwd.h"

#include <cstdint>
#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <string_view>
#include <tuple>
#include <vector>
#include <gslu.h>

namespace gl
{
class Framebuffer;

class TextureAttachment final
{
  friend Framebuffer;

private:
  const gslu::nn_shared<Texture> m_texture;
  const int32_t m_level;
  const glm::ivec2 m_size;

  void attach(const Framebuffer& framebuffer, api::FramebufferAttachment attachment) const;

public:
  template<typename PixelT>
  explicit TextureAttachment(const gslu::nn_shared<Texture2D<PixelT>>& texture, int32_t level = 0)
      : m_texture{texture}
      , m_level{level}
      , m_size{texture->size()}
  {
  }

  template<typename PixelT>
  explicit TextureAttachment(const gslu::nn_shared<TextureDepth<PixelT>>& texture, int32_t level = 0)
      : m_texture{texture}
      , m_level{level}
      , m_size{texture->size()}
  {
  }

  ~TextureAttachment() = default;

  [[nodiscard]] const glm::ivec2& getTextureSize() const
  {
    return m_size;
  }
};

class Framebuffer final : public BindableResource<api::ObjectIdentifier::Framebuffer>
{
public:
  using Attachment = std::tuple<gslu::nn_shared<TextureAttachment>, api::FramebufferAttachment>;
  using Attachments = std::vector<Attachment>;

private:
  Attachments m_attachments;
  RenderState m_renderState;
  const glm::ivec2 m_size;

public:
  explicit Framebuffer(Attachments attachments,
                       const std::string_view& label,
                       RenderState&& renderState,
                       const glm::ivec2& size);

  [[nodiscard]] const Attachments& getAttachments() const
  {
    return m_attachments;
  }

  [[nodiscard]] bool isComplete() const;

  static void unbindAll();

  void invalidate();

  [[nodiscard]] const auto& getRenderState() const
  {
    return m_renderState;
  }

  void blit(const Framebuffer& target, gl::api::BlitFramebufferFilter filter = gl::api::BlitFramebufferFilter::Nearest);

  void blit(const glm::ivec2& backbufferSize,
            gl::api::BlitFramebufferFilter filter = gl::api::BlitFramebufferFilter::Nearest);
};

class FrameBufferBuilder
{
private:
  Framebuffer::Attachments m_attachments;
  RenderState m_renderState{};

public:
  gslu::nn_shared<Framebuffer> build(const std::string_view& label);

  template<typename TPixel>
  FrameBufferBuilder& texture(api::FramebufferAttachment attachment,
                              const gslu::nn_shared<Texture2D<TPixel>>& texture,
                              int32_t level = 0,
                              bool alphaBlend = true)
  {
    m_attachments.emplace_back(std::make_shared<TextureAttachment>(texture, level), attachment);
    if(attachment >= api::FramebufferAttachment::ColorAttachment0
       && attachment <= api::FramebufferAttachment::ColorAttachment31)
      m_renderState.setBlend(static_cast<uint32_t>(attachment)
                               - static_cast<uint32_t>(api::FramebufferAttachment::ColorAttachment0),
                             alphaBlend);
    return *this;
  }

  template<typename TPixel>
  FrameBufferBuilder& textureNoBlend(api::FramebufferAttachment attachment,
                                     const gslu::nn_shared<Texture2D<TPixel>>& texture,
                                     int32_t level = 0)
  {
    m_attachments.emplace_back(std::make_shared<TextureAttachment>(texture, level), attachment);
    if(attachment >= api::FramebufferAttachment::ColorAttachment0
       && attachment <= api::FramebufferAttachment::ColorAttachment31)
      m_renderState.setBlend(
        static_cast<uint32_t>(attachment) - static_cast<uint32_t>(api::FramebufferAttachment::ColorAttachment0), false);
    return *this;
  }

  template<typename TPixel>
  FrameBufferBuilder& textureNoBlend(api::FramebufferAttachment attachment,
                                     const gslu::nn_shared<TextureDepth<TPixel>>& texture,
                                     int32_t level = 0)
  {
    m_attachments.emplace_back(std::make_shared<TextureAttachment>(texture, level), attachment);
    if(attachment >= api::FramebufferAttachment::ColorAttachment0
       && attachment <= api::FramebufferAttachment::ColorAttachment31)
      m_renderState.setBlend(
        static_cast<uint32_t>(attachment) - static_cast<uint32_t>(api::FramebufferAttachment::ColorAttachment0), false);
    return *this;
  }
};
} // namespace gl
