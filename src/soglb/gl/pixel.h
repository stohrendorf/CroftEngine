#pragma once

#include "typetraits.h"

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename T, size_t _Channels, api::PixelFormat _PixelFormat, api::InternalFormat _InternalFormat>
struct Pixel
{
  static_assert(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<T, api::core::Half>,
                "Pixel may only have channels of integral types");

  static_assert(_Channels > 0, "Pixel must contain at least one channel");

  using Self = Pixel<T, _Channels, _PixelFormat, _InternalFormat>;

  using Type = T;
  using Traits = TypeTraits<T>;

  static constexpr auto Channels = _Channels;
  static constexpr api::PixelFormat PixelFormat = _PixelFormat;
  static constexpr api::InternalFormat InternalFormat = _InternalFormat;
  static constexpr api::PixelType PixelType = Traits::PixelType;
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
    std::transform(channels.begin(), channels.end(), tmp.channels.begin(), [rhs](Type v) { return v * rhs; });
    return tmp;
  }
};

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename T, api::PixelFormat _PixelFormat, api::InternalFormat _InternalFormat>
Pixel<T, 4, _PixelFormat, _InternalFormat> mixAlpha(const Pixel<T, 4, _PixelFormat, _InternalFormat>& lhs,
                                                    const Pixel<T, 4, _PixelFormat, _InternalFormat>& rhs)
{
  const float bias = static_cast<float>(rhs.channels[3]) / std::numeric_limits<T>::max();
  return Pixel<T, 4, _PixelFormat, _InternalFormat>{glm::mix(lhs.channels, rhs.channels, bias)};
}

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename T, size_t _Channels, api::PixelFormat _PixelFormat, api::InternalFormat _InternalFormat>
Pixel<T, _Channels, _PixelFormat, _InternalFormat> mix(const Pixel<T, _Channels, _PixelFormat, _InternalFormat>& lhs,
                                                       const Pixel<T, _Channels, _PixelFormat, _InternalFormat>& rhs,
                                                       float bias)
{
  auto tmp = lhs;
  for(size_t i = 0; i < _Channels; ++i)
    tmp.channels[i] = static_cast<T>(lhs.channels[i] * (1 - bias) + rhs.channels[i] * bias);
  return tmp;
}

template<typename T>
using SRGBA = Pixel<T, 4, api::PixelFormat::Rgba, TypeTraits<T>::SrgbaSizedInternalFormat>;
using SRGBA8 = SRGBA<uint8_t>;

template<typename T>
using SRGB = Pixel<T, 3, api::PixelFormat::Rgb, TypeTraits<T>::SrgbSizedInternalFormat>;
using SRGB8 = SRGB<uint8_t>;
// using SRGB16F = SRGB<api::core::Half>;
// using SRGB32F = SRGB<float>;

template<typename T>
using RGB = Pixel<T, 3, api::PixelFormat::Rgb, TypeTraits<T>::RgbSizedInternalFormat>;
using RGB8 = RGB<uint8_t>;
using RGB16F = RGB<api::core::Half>;
using RGB32 = RGB<int32_t>;
using RGB32F = RGB<float>;

template<typename T>
using RG = Pixel<T, 2, api::PixelFormat::Rg, TypeTraits<T>::RgSizedInternalFormat>;
using RG8 = RG<uint8_t>;
using RG16F = RG<api::core::Half>;
using RG32F = RG<float>;

template<typename T>
using Scalar = Pixel<T, 1, api::PixelFormat::Red, TypeTraits<T>::RSizedInternalFormat>;
using ScalarByte = Scalar<uint8_t>;
using Scalar32F = Scalar<float>;
using Scalar16F = Scalar<api::core::Half>;

template<typename T>
struct ScalarDepth final
{
  static_assert(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<api::core::Half, T>,
                "Pixel may only have channels of integral types");

  using Type = T;
  using Traits = TypeTraits<T>;

  static constexpr api::PixelFormat PixelFormat = api::PixelFormat::DepthComponent;
  static constexpr api::PixelType PixelType = Traits::PixelType;
  static constexpr auto InternalFormat = Traits::DepthInternalFormat;

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
