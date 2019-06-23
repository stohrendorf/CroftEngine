#pragma once

#include "typetraits.h"

namespace render
{
namespace gl
{
template<typename T>
struct SRGBA final
{
    static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value,
                  "Pixel may only have channels of integral types");

    using Type = T;
    using Traits = TypeTraits<T>;

    static constexpr const ::gl::PixelFormat PixelFormat = ::gl::PixelFormat::Rgba;
    static constexpr const ::gl::PixelType PixelType = Traits::PixelType;
    static constexpr const auto InternalFormat = Traits::SrgbaSizedInternalFormat;

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

using SRGBA8 = SRGBA<uint8_t>;

template<typename T>
struct SRGB final
{
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<::gl::core::Half, T>,
                  "Pixel may only have channels of integral types");

    using Type = T;
    using Traits = TypeTraits<T>;

    static constexpr const ::gl::PixelFormat PixelFormat = ::gl::PixelFormat::Rgb;
    static constexpr const ::gl::PixelType PixelType = Traits::PixelType;
    static constexpr const auto InternalFormat = Traits::SrgbSizedInternalFormat;

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

using SRGB8 = SRGB<uint8_t>;
using SRGB16F = SRGB<::gl::core::Half>;
using SRGB32F = SRGB<float>;

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
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<::gl::core::Half, T>,
                  "Pixel may only have channels of integral types");

    using Type = T;
    using Traits = TypeTraits<T>;

    static constexpr const ::gl::PixelFormat PixelFormat = ::gl::PixelFormat::Rgb;
    static constexpr const ::gl::PixelType PixelType = Traits::PixelType;
    static constexpr const auto InternalFormat = Traits::RgbSizedInternalFormat;

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

using RGB8 = RGB<uint8_t>;
using RGB16F = RGB<::gl::core::Half>;
using RGB32F = RGB<float>;

template<typename T>
struct RG final
{
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<::gl::core::Half, T>,
                  "Pixel may only have channels of integral types");

    using Type = T;
    using Traits = TypeTraits<T>;

    static constexpr const ::gl::PixelFormat PixelFormat = ::gl::PixelFormat::Rg;
    static constexpr const ::gl::PixelType PixelType = Traits::PixelType;
    static constexpr const auto InternalFormat = Traits::RgSizedInternalFormat;

    explicit RG()
        : RG{0}
    {
    }

    explicit constexpr RG(Type value) noexcept
        : r{value}
        , g{value}
    {
    }

    constexpr RG(Type r_, Type g_, Type b_) noexcept
        : r{r_}
        , g{g_}
    {
    }

    Type r, g;
};

template<typename T>
constexpr bool operator==(const RG<T>& lhs, const RG<T>& rhs)
{
    return lhs.r == rhs.r && lhs.g == rhs.g;
}

template<typename T>
constexpr bool operator!=(const RG<T>& lhs, const RG<T>& rhs)
{
    return !(lhs == rhs);
}

using RG8 = RG<uint8_t>;
using RG16F = RG<::gl::core::Half>;
using RG32F = RG<float>;

template<typename T>
struct Scalar final
{
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<::gl::core::Half, T>,
                  "Pixel may only have channels of integral types");

    using Type = T;
    using Traits = TypeTraits<T>;

    static constexpr const ::gl::PixelFormat PixelFormat = ::gl::PixelFormat::Red;
    static constexpr const ::gl::PixelType PixelType = Traits::PixelType;
    static constexpr const auto InternalFormat = Traits::RSizedInternalFormat;

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

using ScalarByte = Scalar<uint8_t>;
using Scalar32F = Scalar<float>;
using Scalar16F = Scalar<::gl::core::Half>;
} // namespace gl
} // namespace render
