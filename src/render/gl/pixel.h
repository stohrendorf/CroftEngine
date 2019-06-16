#pragma once

#include "typetraits.h"

namespace render
{
namespace gl
{
enum class PixelFormat : RawGlEnum
{
    Alpha = (RawGlEnum)::gl::GL_ALPHA,
    Bgr = (RawGlEnum)::gl::GL_BGR,
    BgrInteger = (RawGlEnum)::gl::GL_BGR_INTEGER,
    Bgra = (RawGlEnum)::gl::GL_BGRA,
    BgraInteger = (RawGlEnum)::gl::GL_BGRA_INTEGER,
    Blue = (RawGlEnum)::gl::GL_BLUE,
    BlueInteger = (RawGlEnum)::gl::GL_BLUE_INTEGER,
    ColorIndex = (RawGlEnum)::gl::GL_COLOR_INDEX,
    DepthComponent = (RawGlEnum)::gl::GL_DEPTH_COMPONENT,
    DepthStencil = (RawGlEnum)::gl::GL_DEPTH_STENCIL,
    Green = (RawGlEnum)::gl::GL_GREEN,
    GreenInteger = (RawGlEnum)::gl::GL_GREEN_INTEGER,
    Luminance = (RawGlEnum)::gl::GL_LUMINANCE,
    LuminanceAlpha = (RawGlEnum)::gl::GL_LUMINANCE_ALPHA,
    Red = (RawGlEnum)::gl::GL_RED,
    RedInteger = (RawGlEnum)::gl::GL_RED_INTEGER,
    Rg = (RawGlEnum)::gl::GL_RG,
    RgInteger = (RawGlEnum)::gl::GL_RG_INTEGER,
    Rgb = (RawGlEnum)::gl::GL_RGB,
    RgbInteger = (RawGlEnum)::gl::GL_RGB_INTEGER,
    Rgba = (RawGlEnum)::gl::GL_RGBA,
    RgbaInteger = (RawGlEnum)::gl::GL_RGBA_INTEGER,
    StencilIndex = (RawGlEnum)::gl::GL_STENCIL_INDEX,
    UInt = (RawGlEnum)::gl::GL_UNSIGNED_INT,
    UShort = (RawGlEnum)::gl::GL_UNSIGNED_SHORT,
};

template<typename T>
struct SRGBA final
{
    static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value,
                  "Pixel may only have channels of integral types");

    using Type = T;
    using Traits = TypeTraits<T>;

    static constexpr const PixelFormat PixelFormat = PixelFormat::Rgba;
    static constexpr const PixelType PixelType = Traits::PixelType;
    static constexpr const auto InternalFormat = Traits::SrgbaSizedInternalFormat;
    static constexpr const ::gl::GLenum Format = ::gl::GL_RGBA;

    explicit SRGBA()
        : SRGBA{0}
    {
    }

    explicit constexpr SRGBA(Type value) noexcept
        : r{value}
        , g{value}
        , b{value}
        , a{value}
    {
    }

    constexpr SRGBA(Type r_, Type g_, Type b_, Type a_) noexcept
        : r{r_}
        , g{g_}
        , b{b_}
        , a{a_}
    {
    }

    Type r, g, b, a;
};

template<typename T>
constexpr bool operator==(const SRGBA<T>& lhs, const SRGBA<T>& rhs)
{
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

template<typename T>
constexpr bool operator!=(const SRGBA<T>& lhs, const SRGBA<T>& rhs)
{
    return !(lhs == rhs);
}

template<typename T>
inline SRGBA<T> mixAlpha(const SRGBA<T>& lhs, const SRGBA<T>& rhs)
{
    const float bias = float(rhs.a) / std::numeric_limits<T>::max();
    return {static_cast<T>(lhs.r * (1 - bias) + rhs.r * bias),
            static_cast<T>(lhs.g * (1 - bias) + rhs.g * bias),
            static_cast<T>(lhs.b * (1 - bias) + rhs.b * bias),
            static_cast<T>(lhs.a * (1 - bias) + rhs.a * bias)};
}

using SRGBA8 = SRGBA<::gl::GLubyte>;

template<typename T>
struct SRGB final
{
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<Half, T>,
                  "Pixel may only have channels of integral types");

    using Type = T;
    using Traits = TypeTraits<T>;

    static constexpr const PixelFormat PixelFormat = PixelFormat::Rgb;
    static constexpr const PixelType PixelType = Traits::PixelType;
    static constexpr const auto InternalFormat = Traits::SrgbSizedInternalFormat;
    static constexpr const ::gl::GLenum Format = ::gl::GL_RGB;

    explicit SRGB()
        : RGB{0}
    {
    }

    explicit constexpr SRGB(Type value) noexcept
        : r{value}
        , g{value}
        , b{value}
    {
    }

    constexpr SRGB(Type r_, Type g_, Type b_) noexcept
        : r{r_}
        , g{g_}
        , b{b_}
    {
    }

    Type r, g, b;
};

using SRGB8 = SRGB<::gl::GLubyte>;
using SRGB16F = SRGB<Half>;
using SRGB32F = SRGB<::gl::GLfloat>;

template<typename T>
constexpr bool operator==(const SRGB<T>& lhs, const SRGB<T>& rhs)
{
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
}

template<typename T>
constexpr bool operator!=(const SRGB<T>& lhs, const SRGB<T>& rhs)
{
    return !(lhs == rhs);
}

template<typename T>
struct RGB final
{
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<Half, T>,
                  "Pixel may only have channels of integral types");

    using Type = T;
    using Traits = TypeTraits<T>;

    static constexpr const PixelFormat PixelFormat = PixelFormat::Rgb;
    static constexpr const PixelType PixelType = Traits::PixelType;
    static constexpr const auto InternalFormat = Traits::RgbSizedInternalFormat;
    static constexpr const ::gl::GLenum Format = ::gl::GL_RGB;

    explicit RGB()
        : RGB{0}
    {
    }

    explicit constexpr RGB(Type value) noexcept
        : r{value}
        , g{value}
        , b{value}
    {
    }

    constexpr RGB(Type r_, Type g_, Type b_) noexcept
        : r{r_}
        , g{g_}
        , b{b_}
    {
    }

    Type r, g, b;
};

template<typename T>
constexpr bool operator==(const RGB<T>& lhs, const RGB<T>& rhs)
{
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
}

template<typename T>
constexpr bool operator!=(const RGB<T>& lhs, const RGB<T>& rhs)
{
    return !(lhs == rhs);
}

using RGB8 = RGB<::gl::GLubyte>;
using RGB16F = RGB<Half>;
using RGB32F = RGB<::gl::GLfloat>;

template<typename T>
struct Scalar final
{
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<Half, T>,
                  "Pixel may only have channels of integral types");

    using Type = T;
    using Traits = TypeTraits<T>;

    static constexpr const PixelFormat PixelFormat = PixelFormat::Red;
    static constexpr const PixelType PixelType = Traits::PixelType;
    static constexpr const auto InternalFormat = Traits::RSizedInternalFormat;
    static constexpr const ::gl::GLenum Format = ::gl::GL_RED;

    explicit Scalar()
        : Scalar{0}
    {
    }

    explicit constexpr Scalar(Type value) noexcept
        : value{value}
    {
    }

    Type value;
};

template<typename T>
constexpr bool operator==(const Scalar<T>& lhs, const Scalar<T>& rhs)
{
    return lhs.value == rhs.value;
}

template<typename T>
constexpr bool operator!=(const Scalar<T>& lhs, const Scalar<T>& rhs)
{
    return !(lhs == rhs);
}

using ScalarByte = Scalar<::gl::GLubyte>;
using Scalar32F = Scalar<::gl::GLfloat>;
using Scalar16F = Scalar<Half>;
} // namespace gl
} // namespace render
