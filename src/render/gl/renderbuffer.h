#pragma once

#include "rendertarget.h"

namespace render
{
namespace gl
{
class RenderBuffer : public RenderTarget
{
public:
    explicit RenderBuffer(const int32_t width,
                          const int32_t height,
                          const ::gl::InternalFormat format,
                          const std::string& label = {})
        : RenderTarget{::gl::genRenderbuffers,
                       [](const uint32_t handle) {
                           ::gl::bindRenderbuffer(::gl::RenderbufferTarget::Renderbuffer, handle);
                       },
                       ::gl::deleteRenderbuffers,
                       ::gl::ObjectIdentifier::Renderbuffer,
                       label}
        , m_width{width}
        , m_height{height}
        , m_format{format}
    {
        GL_ASSERT(::gl::renderbufferStorage(::gl::RenderbufferTarget::Renderbuffer, format, width, height));
    }

    int32_t getWidth() const noexcept override
    {
        return m_width;
    }

    int32_t getHeight() const noexcept override
    {
        return m_height;
    }

    auto getFormat() const noexcept
    {
        return m_format;
    }

private:
    const int32_t m_width;

    const int32_t m_height;

    const ::gl::InternalFormat m_format;
};
} // namespace gl
} // namespace render
