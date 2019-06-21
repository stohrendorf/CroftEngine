#pragma once

#include "rendertarget.h"

#include <vector>

namespace render
{
namespace gl
{
class Texture : public RenderTarget
{
public:
    explicit Texture(::gl::TextureTarget type, const std::string& label = {})
        : RenderTarget{::gl::genTextures,
                       [type](const uint32_t handle) { ::gl::bindTexture(type, handle); },
                       ::gl::deleteTextures,
                       ::gl::ObjectIdentifier::Texture,
                       label}
        , m_type{type}
    {
    }

    Texture& set(const ::gl::TextureMinFilter value)
    {
        bind();
        GL_ASSERT(::gl::texParameter(m_type, ::gl::TextureParameterName::TextureMinFilter, (int32_t)value));
        return *this;
    }

    Texture& set(const ::gl::TextureMagFilter value)
    {
        bind();
        GL_ASSERT(::gl::texParameter(m_type, ::gl::TextureParameterName::TextureMagFilter, (int32_t)value));
        return *this;
    }

    Texture& set(const ::gl::TextureParameterName param, const ::gl::TextureWrapMode value)
    {
        bind();
        GL_ASSERT(::gl::texParameter(m_type, param, (int32_t)value));
        return *this;
    }

    int32_t getWidth() const noexcept override
    {
        return m_width;
    }

    int32_t getHeight() const noexcept override
    {
        return m_height;
    }

    ::gl::TextureTarget getType() const noexcept
    {
        return m_type;
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    template<typename T>
    void image2D(const std::vector<T>& data)
    {
        BOOST_ASSERT(m_width > 0 && m_height > 0);
        BOOST_ASSERT(data.empty()
                     || static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height) == data.size());

        bind();

        GL_ASSERT(::gl::texImage2D(m_type,
                                   0,
                                   T::InternalFormat,
                                   m_width,
                                   m_height,
                                   0,
                                   T::PixelFormat,
                                   T::PixelType,
                                   data.empty() ? nullptr : data.data()));

        if(m_mipmap)
        {
            GL_ASSERT(::gl::generateMipmap(m_type));
        }
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    template<typename T>
    void subImage2D(const std::vector<T>& data)
    {
        BOOST_ASSERT(m_width > 0 && m_height > 0);
        BOOST_ASSERT(static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height) == data.size());

        bind();

        GL_ASSERT(::gl::texSubImage2D(m_type, 0, 0, 0, m_width, m_height, T::PixelFormat, T::PixelType, data.data()));

        if(m_mipmap)
        {
            GL_ASSERT(::gl::generateMipmap(m_type));
        }
    }

    template<typename T>
    Texture& image2D(int32_t width, int32_t height, bool generateMipmaps)
    {
        return image2D(width, height, std::vector<T>{}, generateMipmaps);
    }

    template<typename T>
    Texture& image2D(const int32_t width, const int32_t height, const std::vector<T>& data, const bool generateMipmaps)
    {
        BOOST_ASSERT(width > 0 && height > 0);
        BOOST_ASSERT(data.empty() || static_cast<std::size_t>(width) * static_cast<std::size_t>(height) == data.size());

        bind();

        GL_ASSERT(::gl::texImage2D(m_type,
                                   0,
                                   T::InternalFormat,
                                   width,
                                   height,
                                   0,
                                   T::PixelFormat,
                                   T::PixelType,
                                   data.empty() ? nullptr : data.data()));

        m_width = width;
        m_height = height;
        m_mipmap = generateMipmaps;

        if(m_mipmap)
        {
            GL_ASSERT(::gl::generateMipmap(m_type));
        }

        return *this;
    }

    void depthImage2D(const int32_t width, const int32_t height)
    {
        BOOST_ASSERT(width > 0 && height > 0);

        bind();

        GL_ASSERT(::gl::texImage2D(m_type,
                                   0,
                                   ::gl::InternalFormat::DepthComponent32f,
                                   width,
                                   height,
                                   0,
                                   ::gl::PixelFormat::DepthComponent,
                                   ::gl::PixelType::UnsignedInt,
                                   nullptr));

        m_width = width;
        m_height = height;
        m_mipmap = false;
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void copyImageSubData(const Texture& src)
    {
        if(m_type != src.m_type)
            BOOST_THROW_EXCEPTION(std::runtime_error("Refusing to copy image data with different types"));

        GL_ASSERT(::gl::copyImageSubData(
            src.getHandle(), src.m_type, 0, 0, 0, 0, getHandle(), m_type, 0, 0, 0, 0, src.m_width, src.m_height, 1));
        m_width = src.m_width;
        m_height = src.m_height;
    }

private:
    const ::gl::TextureTarget m_type;

    int32_t m_width = -1;

    int32_t m_height = -1;

    bool m_mipmap = false;
};
} // namespace gl
} // namespace render
