#pragma once

#include "rendertarget.h"

#include <vector>

namespace render
{
namespace gl
{
enum class TextureWrapAxis : std::underlying_type_t<::gl::GLenum>
{
    WrapR = (uint32_t)::gl::GL_TEXTURE_WRAP_R,
    WrapS = (uint32_t)::gl::GL_TEXTURE_WRAP_S,
    WrapT = (uint32_t)::gl::GL_TEXTURE_WRAP_T,
};

enum class TextureWrapMode : std::underlying_type_t<::gl::GLenum>
{
    Clamp = (uint32_t)::gl::GL_CLAMP,
    ClampToBorder = (uint32_t)::gl::GL_CLAMP_TO_BORDER,
    ClampToEdge = (uint32_t)::gl::GL_CLAMP_TO_EDGE,
    Repeat = (uint32_t)::gl::GL_REPEAT,
};

enum class TextureMinFilter : std::underlying_type_t<::gl::GLenum>
{
    Linear = (uint32_t)::gl::GL_LINEAR,
    LinearMipmapLinear = (uint32_t)::gl::GL_LINEAR_MIPMAP_LINEAR,
    LinearMipmapNearest = (uint32_t)::gl::GL_LINEAR_MIPMAP_NEAREST,
    Nearest = (uint32_t)::gl::GL_NEAREST,
    NearestMipmapLinear = (uint32_t)::gl::GL_NEAREST_MIPMAP_LINEAR,
    NearestMipmapNearest = (uint32_t)::gl::GL_NEAREST_MIPMAP_NEAREST,
};

enum class TextureMagFilter : std::underlying_type_t<::gl::GLenum>
{
    Linear = (uint32_t)::gl::GL_LINEAR,
    Nearest = (uint32_t)::gl::GL_NEAREST,
};

enum class TextureParameter : uint32_t
{
    GenerateMipmap = (uint32_t)::gl::GL_GENERATE_MIPMAP,
    BorderColor = (uint32_t)::gl::GL_TEXTURE_BORDER_COLOR,
    Priority = (uint32_t)::gl::GL_TEXTURE_PRIORITY,
    BaseLevel = (uint32_t)::gl::GL_TEXTURE_BASE_LEVEL,
    CompareMode = (uint32_t)::gl::GL_TEXTURE_COMPARE_MODE,
    CompareFunc = (uint32_t)::gl::GL_TEXTURE_COMPARE_FUNC,
    LodBias = (uint32_t)::gl::GL_TEXTURE_LOD_BIAS,
    MinLod = (uint32_t)::gl::GL_TEXTURE_MIN_LOD,
    MaxLod = (uint32_t)::gl::GL_TEXTURE_MAX_LOD,
    MaxLevel = (uint32_t)::gl::GL_TEXTURE_MAX_LEVEL,
    SwizzleR = (uint32_t)::gl::GL_TEXTURE_SWIZZLE_R,
    SwizzleG = (uint32_t)::gl::GL_TEXTURE_SWIZZLE_G,
    SwizzleB = (uint32_t)::gl::GL_TEXTURE_SWIZZLE_B,
    SwizzleA = (uint32_t)::gl::GL_TEXTURE_SWIZZLE_A,
    SwizzleRGBA = (uint32_t)::gl::GL_TEXTURE_SWIZZLE_RGBA,
    DepthStencilMode = (uint32_t)::gl::GL_DEPTH_STENCIL_TEXTURE_MODE,
    AlphaSize = (uint32_t)::gl::GL_TEXTURE_ALPHA_SIZE,
    BlueSize = (uint32_t)::gl::GL_TEXTURE_BLUE_SIZE,
    Border = (uint32_t)::gl::GL_TEXTURE_BORDER,
    Components = (uint32_t)::gl::GL_TEXTURE_COMPONENTS,
    GreenSize = (uint32_t)::gl::GL_TEXTURE_GREEN_SIZE,
    Height = (uint32_t)::gl::GL_TEXTURE_HEIGHT,
    IntensitySize = (uint32_t)::gl::GL_TEXTURE_INTENSITY_SIZE,
    Format = (uint32_t)::gl::GL_TEXTURE_INTERNAL_FORMAT,
    LuminanceSize = (uint32_t)::gl::GL_TEXTURE_LUMINANCE_SIZE,
    RedSize = (uint32_t)::gl::GL_TEXTURE_RED_SIZE,
    Resident = (uint32_t)::gl::GL_TEXTURE_RESIDENT,
    Width = (uint32_t)::gl::GL_TEXTURE_WIDTH,
};

class Texture : public RenderTarget
{
public:
    explicit Texture(::gl::GLenum type, const std::string& label = {})
        : RenderTarget{ ::gl::glGenTextures, [type](const ::gl::GLuint handle) { glBindTexture( type, handle ); },
                        ::gl::glDeleteTextures, ::gl::GL_TEXTURE, label
    }
          , m_type{ type }
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void setLabel(const std::string& lbl)
    {
        bind();
        GL_ASSERT(
            glObjectLabel( ::gl::GL_TEXTURE, getHandle(), static_cast<::gl::GLsizei>(lbl.length()), lbl.c_str() ) );
    }

    Texture& set(const TextureParameter param, const ::gl::GLint value)
    {
        GL_ASSERT( glTextureParameteri( getHandle(), (::gl::GLenum)param, value ) );
        return *this;
    }

    Texture& set(const TextureParameter param, const ::gl::GLenum value)
    {
        GL_ASSERT( glTextureParameteri( getHandle(), (::gl::GLenum)param, value ) );
        return *this;
    }

    Texture& set(const TextureParameter param, const ::gl::GLfloat value)
    {
        GL_ASSERT( glTextureParameterf( getHandle(), (::gl::GLenum)param, value ) );
        return *this;
    }

    Texture& set(const TextureMinFilter value)
    {
        GL_ASSERT( glTextureParameteri( getHandle(), ::gl::GL_TEXTURE_MIN_FILTER, (::gl::GLenum)value ) );
        return *this;
    }

    Texture& set(const TextureMagFilter value)
    {
        GL_ASSERT( glTextureParameteri( getHandle(), ::gl::GL_TEXTURE_MAG_FILTER, (::gl::GLenum)value ) );
        return *this;
    }

    Texture& set(const TextureWrapAxis param, const TextureWrapMode value)
    {
        GL_ASSERT( glTextureParameteri( getHandle(), (::gl::GLenum)param, (::gl::GLenum)value ) );
        return *this;
    }

    ::gl::GLint getWidth() const noexcept override
    {
        return m_width;
    }

    ::gl::GLint getHeight() const noexcept override
    {
        return m_height;
    }

    ::gl::GLenum getType() const noexcept
    {
        return m_type;
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    template<typename T>
    void image2D(const std::vector<T>& data)
    {
        BOOST_ASSERT( m_width > 0 && m_height > 0 );
        BOOST_ASSERT(
            data.empty() || static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height) == data.size() );

        bind();

        GL_ASSERT( glTexImage2D( m_type, 0, T::InternalFormat, m_width, m_height, 0, T::Format, T::TypeId,
                                 data.empty() ? nullptr : data.data() ) );

        if( m_mipmap )
        {
            GL_ASSERT( glGenerateMipmap( m_type ) );
        }
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    template<typename T>
    void subImage2D(const std::vector<T>& data)
    {
        BOOST_ASSERT( m_width > 0 && m_height > 0 );
        BOOST_ASSERT( static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height) == data.size() );

        bind();

        GL_ASSERT( glTexSubImage2D( m_type, 0, 0, 0, m_width, m_height, T::Format, T::TypeId, data.data() ) );

        if( m_mipmap )
        {
            GL_ASSERT( glGenerateMipmap( m_type ) );
        }
    }

    template<typename T>
    Texture& image2D(::gl::GLint width, ::gl::GLint height, bool generateMipmaps)
    {
        return image2D( width, height, std::vector<T>{}, generateMipmaps );
    }

    template<typename T>
    Texture&
    image2D(const ::gl::GLint width, const ::gl::GLint height, const std::vector<T>& data, const bool generateMipmaps)
    {
        BOOST_ASSERT( width > 0 && height > 0 );
        BOOST_ASSERT(
            data.empty() || static_cast<std::size_t>(width) * static_cast<std::size_t>(height) == data.size() );

        bind();

        GL_ASSERT( glTexImage2D( m_type, 0, T::InternalFormat, width, height, 0, T::Format, T::TypeId,
                                 data.empty() ? nullptr : data.data() ) );

        m_width = width;
        m_height = height;
        m_mipmap = generateMipmaps;

        if( m_mipmap )
        {
            GL_ASSERT( glGenerateMipmap( m_type ) );
        }

        return *this;
    }

    void depthImage2D(const ::gl::GLint width, const ::gl::GLint height)
    {
        BOOST_ASSERT( width > 0 && height > 0 );

        bind();

        GL_ASSERT( glTexImage2D( m_type, 0, ::gl::GL_DEPTH_COMPONENT24, width, height, 0, ::gl::GL_DEPTH_COMPONENT,
                                 ::gl::GL_UNSIGNED_INT, nullptr ) );

        m_width = width;
        m_height = height;
        m_mipmap = false;
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void copyImageSubData(const Texture& src)
    {
        if( m_type != src.m_type )
            BOOST_THROW_EXCEPTION( std::runtime_error( "Refusing to copy image data with different types" ) );

        GL_ASSERT( glCopyImageSubData( src.getHandle(), src.m_type, 0, 0, 0, 0, getHandle(), m_type, 0, 0, 0, 0,
                                       src.m_width,
                                       src.m_height, 1 ) );
        m_width = src.m_width;
        m_height = src.m_height;
    }

private:
    const ::gl::GLenum m_type;

    ::gl::GLint m_width = -1;

    ::gl::GLint m_height = -1;

    bool m_mipmap = false;
};
}
}
