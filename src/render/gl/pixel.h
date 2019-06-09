#pragma once

#include "typetraits.h"

namespace render
{
namespace gl
{
template<typename T>
struct SRGBA final
{
    static_assert( std::is_integral<T>::value || std::is_floating_point<T>::value,
                   "Pixel may only have channels of integral types" );

    using Type = T;
    using Traits = TypeTraits<T>;

    static constexpr const auto TypeId = Traits::TypeId;
    static constexpr const auto InternalFormat = Traits::SrgbaInternalFormat;
    static constexpr const GLint Format = GL_RGBA;

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
    const float bias = float( rhs.a ) / std::numeric_limits<T>::max();
    return {
            static_cast<T>(lhs.r * (1 - bias) + rhs.r * bias),
            static_cast<T>(lhs.g * (1 - bias) + rhs.g * bias),
            static_cast<T>(lhs.b * (1 - bias) + rhs.b * bias),
            static_cast<T>(lhs.a * (1 - bias) + rhs.a * bias)
    };
}

using SRGBA8 = SRGBA<GLubyte>;


template<typename T>
struct SRGB final
{
    static_assert( std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<Half, T>,
                   "Pixel may only have channels of integral types" );

    using Type = T;
    using Traits = TypeTraits<T>;

    static constexpr const auto TypeId = Traits::TypeId;
    static constexpr const auto InternalFormat = Traits::SrgbInternalFormat;
    static constexpr const GLint Format = GL_RGB;

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


using SRGB8 = SRGB<GLubyte>;
using SRGB16F = SRGB<Half>;
using SRGB32F = SRGB<GLfloat>;

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
    static_assert( std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<Half, T>,
                   "Pixel may only have channels of integral types" );

    using Type = T;
    using Traits = TypeTraits<T>;

    static constexpr const auto TypeId = Traits::TypeId;
    static constexpr const auto InternalFormat = Traits::RgbInternalFormat;
    static constexpr const GLint Format = GL_RGB;

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

using RGB8 = RGB<GLubyte>;
using RGB16F = RGB<Half>;
using RGB32F = RGB<GLfloat>;


template<typename T>
struct Single final
{
    static_assert( std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<Half, T>,
                   "Pixel may only have channels of integral types" );

    using Type = T;
    using Traits = TypeTraits<T>;

    static constexpr const auto TypeId = Traits::TypeId;
    static constexpr const auto InternalFormat = Traits::RInternalFormat;
    static constexpr const GLint Format = GL_RED;

    explicit Single()
            : Single{0}
    {
    }

    explicit constexpr Single(Type value) noexcept
            : value{value}
    {
    }

    Type value;
};


template<typename T>
constexpr bool operator==(const Single<T>& lhs, const Single<T>& rhs)
{
    return lhs.value == rhs.value;
}

template<typename T>
constexpr bool operator!=(const Single<T>& lhs, const Single<T>& rhs)
{
    return !(lhs == rhs);
}

using SingleByte = Single<GLubyte>;
using SingleF = Single<GLfloat>;
using SingleH = Single<Half>;
}
}
