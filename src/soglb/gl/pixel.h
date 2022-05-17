#pragma once

#include "typetraits.h"

#include <algorithm>
#include <glm/glm.hpp>

namespace gl
{
template<typename T,
         // NOLINTNEXTLINE(bugprone-reserved-identifier)
         glm::length_t _Channels,
         // NOLINTNEXTLINE(bugprone-reserved-identifier)
         api::PixelFormat _PixelFormat,
         // NOLINTNEXTLINE(bugprone-reserved-identifier)
         api::SizedInternalFormat _SizedInternalFormat,
         // NOLINTNEXTLINE(bugprone-reserved-identifier)
         bool _Premultiplied>
struct Pixel
{
  static_assert(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<T, api::core::Half>,
                "Pixel may only have channels of integral types");

  static_assert(_Channels > 0, "Pixel must contain at least one channel");

  using Self = Pixel<T, _Channels, _PixelFormat, _SizedInternalFormat, _Premultiplied>;

  using Type = T;

  static constexpr auto Channels = _Channels;
  static constexpr api::PixelFormat PixelFormat = _PixelFormat;
  static constexpr api::SizedInternalFormat SizedInternalFormat = _SizedInternalFormat;
  static constexpr api::PixelType PixelType = ::gl::PixelType<T>;
  static constexpr bool Premultiplied = _Premultiplied;
  using Vec = glm::vec<Channels, Type, glm::qualifier::defaultp>;

  Vec channels;

  explicit constexpr Pixel() noexcept
      : Pixel{Type(0)}
  {
  }

  explicit constexpr Pixel(Type scalar) noexcept
      : channels{scalar}
  {
  }

  template<typename... U>
  constexpr Pixel(Type value0, Type value1, U... tail) noexcept
      : channels{value0, value1, static_cast<Type>(tail)...}
  {
    static_assert(sizeof...(U) + 2 == _Channels, "Invalid constructor call");
  }

  explicit constexpr Pixel(Vec channels) noexcept
      : channels{std::move(channels)}
  {
  }

  constexpr bool operator==(const Self& rhs) const
  {
    return channels == rhs.channels;
  }

  constexpr bool operator!=(const Self& rhs) const
  {
    return !(*this == rhs);
  }

  template<typename U>
  Self operator*(U rhs) const
  {
    Self tmp = *this;
    std::transform(&channels[0],
                   &channels[0] + _Channels,
                   &tmp.channels[0],
                   [rhs](Type v)
                   {
                     return v * rhs;
                   });
    return tmp;
  }
};

template<typename T,
         typename U,
         // NOLINTNEXTLINE(bugprone-reserved-identifier)
         glm::length_t _Channels,
         // NOLINTNEXTLINE(bugprone-reserved-identifier)
         api::PixelFormat _PixelFormat,
         // NOLINTNEXTLINE(bugprone-reserved-identifier)
         api::SizedInternalFormat _SizedInternalFormat,
         // NOLINTNEXTLINE(bugprone-reserved-identifier)
         bool _Premultiplied>
auto imix(const Pixel<T, _Channels, _PixelFormat, _SizedInternalFormat, _Premultiplied>& lhs,
          const Pixel<T, _Channels, _PixelFormat, _SizedInternalFormat, _Premultiplied>& rhs,
          U bias,
          U biasMax = std::numeric_limits<U>::max())
  -> std::enable_if_t<std::is_unsigned_v<T> == std::is_unsigned_v<U>, // lgtm [cpp/comparison-of-identical-expressions]
                      Pixel<T, _Channels, _PixelFormat, _SizedInternalFormat, _Premultiplied>>
{
  if(bias >= biasMax)
    return rhs;
  else if(bias <= 0)
    return lhs;

  const auto invBias = biasMax - bias;
  auto tmp = lhs;
  for(glm::length_t i = 0; i < _Channels; ++i)
  {
    tmp.channels[i] = static_cast<T>(lhs.channels[i] * invBias / biasMax + rhs.channels[i] * bias / biasMax);
  }
  return tmp;
}

template<typename T,
         // NOLINTNEXTLINE(bugprone-reserved-identifier)
         api::PixelFormat _PixelFormat,
         // NOLINTNEXTLINE(bugprone-reserved-identifier)
         api::SizedInternalFormat _SizedInternalFormat,
         // NOLINTNEXTLINE(bugprone-reserved-identifier)
         bool _Premultiplied>
Pixel<T, 4, _PixelFormat, _SizedInternalFormat, _Premultiplied>
  mixAlpha(const Pixel<T, 4, _PixelFormat, _SizedInternalFormat, _Premultiplied>& lhs,
           const Pixel<T, 4, _PixelFormat, _SizedInternalFormat, _Premultiplied>& rhs)
{
  return imix(lhs, rhs, rhs.channels[3]);
}

template<typename T>
using SRGBA = Pixel<T, 4, api::PixelFormat::Rgba, SrgbaSizedInternalFormat<T>, false>;
using SRGBA8 = SRGBA<uint8_t>;

template<typename T>
using PremultipliedSRGBA = Pixel<T, 4, api::PixelFormat::Rgba, SrgbaSizedInternalFormat<T>, true>;
using PremultipliedSRGBA8 = PremultipliedSRGBA<uint8_t>;

template<typename T>
using SRGB = Pixel<T, 3, api::PixelFormat::Rgb, SrgbSizedInternalFormat<T>, false>;
using SRGB8 = SRGB<uint8_t>;
// using SRGB16F = SRGB<api::core::Half>;
// using SRGB32F = SRGB<float>;

template<typename T>
using RGB = Pixel<T, 3, api::PixelFormat::Rgb, RgbSizedInternalFormat<T>, false>;
using RGB8 = RGB<uint8_t>;
using RGB16F = RGB<api::core::Half>;
using RGB32 = RGB<int32_t>;
using RGB32F = RGB<float>;

template<typename T>
using RG = Pixel<T, 2, api::PixelFormat::Rg, RgSizedInternalFormat<T>, false>;
using RG8 = RG<uint8_t>;
using RG16F = RG<api::core::Half>;
using RG32F = RG<float>;

template<typename T>
using Scalar = Pixel<T, 1, api::PixelFormat::Red, RSizedInternalFormat<T>, false>;
using ScalarByte = Scalar<uint8_t>;
using Scalar32F = Scalar<float>;
using Scalar16F = Scalar<api::core::Half>;

namespace detail
{
template<typename T>
constexpr T premultiply(T value, T alpha) noexcept
{
  return value * alpha / std::numeric_limits<T>::max();
}

constexpr float premultiply(float value, float alpha) noexcept
{
  return value * alpha;
}
} // namespace detail

constexpr PremultipliedSRGBA8 premultiply(const SRGBA8& color)
{
  return PremultipliedSRGBA8{
    detail::premultiply(color.channels[0], color.channels[3]),
    detail::premultiply(color.channels[1], color.channels[3]),
    detail::premultiply(color.channels[2], color.channels[3]),
    color.channels[3],
  };
}

constexpr glm::vec4 premultiply(const glm::vec4& color)
{
  return glm::vec4{
    detail::premultiply(color[0], color[3]),
    detail::premultiply(color[1], color[3]),
    detail::premultiply(color[2], color[3]),
    color[3],
  };
}

template<typename T>
struct ScalarDepth final
{
  static_assert(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<api::core::Half, T>,
                "Pixel may only have channels of integral types");

  using Type = T;

  static constexpr api::PixelFormat PixelFormat = api::PixelFormat::DepthComponent;
  static constexpr api::PixelType PixelType = ::gl::PixelType<T>;
  static constexpr auto InternalFormat = DepthInternalFormat<T>;

  explicit ScalarDepth()
      : ScalarDepth{0}
  {
  }

  explicit constexpr ScalarDepth(Type value) noexcept
      : value{value}
  {
  }

  Type value;
};

using ScalarDepth32F = ScalarDepth<float>;
using ScalarDepth16F = ScalarDepth<api::core::Half>;

template<typename T>
constexpr bool operator==(const ScalarDepth<T>& lhs, const ScalarDepth<T>& rhs)
{
  return lhs.value == rhs.value;
}

template<typename T>
constexpr bool operator!=(const ScalarDepth<T>& lhs, const ScalarDepth<T>& rhs)
{
  return !(lhs == rhs);
}
} // namespace gl
