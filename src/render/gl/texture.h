#pragma once

#include "rendertarget.h"

#include <vector>

namespace render
{
namespace gl
{
enum class TextureWrapAxis : RawGlEnum
{
    WrapR = (RawGlEnum)::gl::GL_TEXTURE_WRAP_R,
    WrapS = (RawGlEnum)::gl::GL_TEXTURE_WRAP_S,
    WrapT = (RawGlEnum)::gl::GL_TEXTURE_WRAP_T,
};

enum class TextureWrapMode : RawGlEnum
{
    Clamp = (RawGlEnum)::gl::GL_CLAMP,
    ClampToBorder = (RawGlEnum)::gl::GL_CLAMP_TO_BORDER,
    ClampToEdge = (RawGlEnum)::gl::GL_CLAMP_TO_EDGE,
    Repeat = (RawGlEnum)::gl::GL_REPEAT,
};

enum class TextureMinFilter : RawGlEnum
{
    Linear = (RawGlEnum)::gl::GL_LINEAR,
    LinearMipmapLinear = (RawGlEnum)::gl::GL_LINEAR_MIPMAP_LINEAR,
    LinearMipmapNearest = (RawGlEnum)::gl::GL_LINEAR_MIPMAP_NEAREST,
    Nearest = (RawGlEnum)::gl::GL_NEAREST,
    NearestMipmapLinear = (RawGlEnum)::gl::GL_NEAREST_MIPMAP_LINEAR,
    NearestMipmapNearest = (RawGlEnum)::gl::GL_NEAREST_MIPMAP_NEAREST,
};

enum class TextureMagFilter : RawGlEnum
{
    Linear = (RawGlEnum)::gl::GL_LINEAR,
    Nearest = (RawGlEnum)::gl::GL_NEAREST,
};

enum class TextureTarget : RawGlEnum
{
    Proxy1D = (RawGlEnum)::gl::GL_PROXY_TEXTURE_1D,
    Proxy1DArray = (RawGlEnum)::gl::GL_PROXY_TEXTURE_1D_ARRAY,
    Proxy2D = (RawGlEnum)::gl::GL_PROXY_TEXTURE_2D,
    Proxy2DArray = (RawGlEnum)::gl::GL_PROXY_TEXTURE_2D_ARRAY,
    Proxy2DMultisample = (RawGlEnum)::gl::GL_PROXY_TEXTURE_2D_MULTISAMPLE,
    Proxy2DMultisampleArray = (RawGlEnum)::gl::GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY,
    Proxy3D = (RawGlEnum)::gl::GL_PROXY_TEXTURE_3D,
    ProxyCubeMap = (RawGlEnum)::gl::GL_PROXY_TEXTURE_CUBE_MAP,
    ProxyCubeMapArray = (RawGlEnum)::gl::GL_PROXY_TEXTURE_CUBE_MAP_ARRAY,
    ProxyRectangle = (RawGlEnum)::gl::GL_PROXY_TEXTURE_RECTANGLE,
    _1D = (RawGlEnum)::gl::GL_TEXTURE_1D,
    _2D = (RawGlEnum)::gl::GL_TEXTURE_2D,
    _3D = (RawGlEnum)::gl::GL_TEXTURE_3D,
    Rectangle = (RawGlEnum)::gl::GL_TEXTURE_RECTANGLE,
    CubeMap = (RawGlEnum)::gl::GL_TEXTURE_CUBE_MAP,
    CubeMapPositiveX = (RawGlEnum)::gl::GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    CubeMapNegativeX = (RawGlEnum)::gl::GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    CubeMapPositiveY = (RawGlEnum)::gl::GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    CubeMapNegativeY = (RawGlEnum)::gl::GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    CubeMapPositiveZ = (RawGlEnum)::gl::GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    CubeMapNegativeZ = (RawGlEnum)::gl::GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
    CubeMapArray = (RawGlEnum)::gl::GL_TEXTURE_CUBE_MAP_ARRAY,
    _1DArray = (RawGlEnum)::gl::GL_TEXTURE_1D_ARRAY,
    _2DArray = (RawGlEnum)::gl::GL_TEXTURE_2D_ARRAY,
    _2DMultisample = (RawGlEnum)::gl::GL_TEXTURE_2D_MULTISAMPLE,
    _2DMultisampleArray = (RawGlEnum)::gl::GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
};

enum class TextureParameter : RawGlEnum
{
    GenerateMipmap = (RawGlEnum)::gl::GL_GENERATE_MIPMAP,
    BorderColor = (RawGlEnum)::gl::GL_TEXTURE_BORDER_COLOR,
    Priority = (RawGlEnum)::gl::GL_TEXTURE_PRIORITY,
    BaseLevel = (RawGlEnum)::gl::GL_TEXTURE_BASE_LEVEL,
    CompareMode = (RawGlEnum)::gl::GL_TEXTURE_COMPARE_MODE,
    CompareFunc = (RawGlEnum)::gl::GL_TEXTURE_COMPARE_FUNC,
    LodBias = (RawGlEnum)::gl::GL_TEXTURE_LOD_BIAS,
    MinLod = (RawGlEnum)::gl::GL_TEXTURE_MIN_LOD,
    MaxLod = (RawGlEnum)::gl::GL_TEXTURE_MAX_LOD,
    MaxLevel = (RawGlEnum)::gl::GL_TEXTURE_MAX_LEVEL,
    SwizzleR = (RawGlEnum)::gl::GL_TEXTURE_SWIZZLE_R,
    SwizzleG = (RawGlEnum)::gl::GL_TEXTURE_SWIZZLE_G,
    SwizzleB = (RawGlEnum)::gl::GL_TEXTURE_SWIZZLE_B,
    SwizzleA = (RawGlEnum)::gl::GL_TEXTURE_SWIZZLE_A,
    SwizzleRGBA = (RawGlEnum)::gl::GL_TEXTURE_SWIZZLE_RGBA,
    DepthStencilMode = (RawGlEnum)::gl::GL_DEPTH_STENCIL_TEXTURE_MODE,
    AlphaSize = (RawGlEnum)::gl::GL_TEXTURE_ALPHA_SIZE,
    BlueSize = (RawGlEnum)::gl::GL_TEXTURE_BLUE_SIZE,
    Border = (RawGlEnum)::gl::GL_TEXTURE_BORDER,
    Components = (RawGlEnum)::gl::GL_TEXTURE_COMPONENTS,
    GreenSize = (RawGlEnum)::gl::GL_TEXTURE_GREEN_SIZE,
    Height = (RawGlEnum)::gl::GL_TEXTURE_HEIGHT,
    IntensitySize = (RawGlEnum)::gl::GL_TEXTURE_INTENSITY_SIZE,
    Format = (RawGlEnum)::gl::GL_TEXTURE_INTERNAL_FORMAT,
    LuminanceSize = (RawGlEnum)::gl::GL_TEXTURE_LUMINANCE_SIZE,
    RedSize = (RawGlEnum)::gl::GL_TEXTURE_RED_SIZE,
    Resident = (RawGlEnum)::gl::GL_TEXTURE_RESIDENT,
    Width = (RawGlEnum)::gl::GL_TEXTURE_WIDTH,
};

class Texture : public RenderTarget
{
public:
    explicit Texture(TextureTarget type, const std::string& label = {})
        : RenderTarget{ ::gl::glGenTextures,
                        [type](const ::gl::GLuint handle) { glBindTexture( (::gl::GLenum)type, handle ); },
                        ::gl::glDeleteTextures, ObjectIdentifier::Texture, label }
        , m_type{ type }
    {
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

    TextureTarget getType() const noexcept
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

        GL_ASSERT( glTexImage2D( (::gl::GLenum)m_type, 0, T::InternalFormat, m_width, m_height, 0,
                                 (::gl::GLenum)T::PixelFormat, (::gl::GLenum)T::PixelType,
                                 data.empty() ? nullptr : data.data() ) );

        if( m_mipmap )
        {
            GL_ASSERT( glGenerateMipmap( (::gl::GLenum)m_type ) );
        }
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    template<typename T>
    void subImage2D(const std::vector<T>& data)
    {
        BOOST_ASSERT( m_width > 0 && m_height > 0 );
        BOOST_ASSERT( static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height) == data.size() );

        bind();

        GL_ASSERT( glTexSubImage2D( (::gl::GLenum)m_type,
                                    0, 0, 0, m_width, m_height,
                                    (::gl::GLenum)T::PixelFormat, (::gl::GLenum)T::PixelType, data.data() ) );

        if( m_mipmap )
        {
            GL_ASSERT( glGenerateMipmap( (::gl::GLenum)m_type ) );
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

        GL_ASSERT( glTexImage2D( (::gl::GLenum)m_type, 0, (::gl::GLenum)T::InternalFormat, width, height, 0,
                                 (::gl::GLenum)T::PixelFormat, (::gl::GLenum)T::PixelType,
                                 data.empty() ? nullptr : data.data() ) );

        m_width = width;
        m_height = height;
        m_mipmap = generateMipmaps;

        if( m_mipmap )
        {
            GL_ASSERT( glGenerateMipmap( (::gl::GLenum)m_type ) );
        }

        return *this;
    }

    void depthImage2D(const ::gl::GLint width, const ::gl::GLint height)
    {
        BOOST_ASSERT( width > 0 && height > 0 );

        bind();

        GL_ASSERT( glTexImage2D( (::gl::GLenum)m_type,
                                 0,
                                 ::gl::GL_DEPTH_COMPONENT24,
                                 width,
                                 height,
                                 0,
                                 ::gl::GL_DEPTH_COMPONENT,
                                 ::gl::GL_UNSIGNED_INT,
                                 nullptr ) );

        m_width = width;
        m_height = height;
        m_mipmap = false;
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void copyImageSubData(const Texture& src)
    {
        if( m_type != src.m_type )
            BOOST_THROW_EXCEPTION( std::runtime_error( "Refusing to copy image data with different types" ) );

        GL_ASSERT( glCopyImageSubData( src.getHandle(), (::gl::GLenum)src.m_type, 0, 0, 0, 0,
                                       getHandle(), (::gl::GLenum)m_type, 0, 0, 0, 0,
                                       src.m_width, src.m_height, 1 ) );
        m_width = src.m_width;
        m_height = src.m_height;
    }

private:
    const TextureTarget m_type;

    ::gl::GLint m_width = -1;

    ::gl::GLint m_height = -1;

    bool m_mipmap = false;
};
}
}
