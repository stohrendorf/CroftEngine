#pragma once

#include "typetraits.h"

#include <array>

namespace render::gl
{
template<typename T, size_t _Channels, ::gl::PixelFormat _PixelFormat, ::gl::InternalFormat _InternalFormat>
struct Pixel
{
  static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value
                  || std::is_same<T, ::gl::core::Half>::value,
                "Pixel may only have channels of integral types");

  static_assert(_Channels > 0, "Pixel must contain at least one channel");

  using Self = Pixel<T, _Channels, _PixelFormat, _InternalFormat>;

  using Type = T;
  using Traits = TypeTraits<T>;

  static constexpr auto Channels = _Channels;
  static constexpr ::gl::PixelFormat PixelFormat = _PixelFormat;
  static constexpr ::gl::InternalFormat InternalFormat = _InternalFormat;
  static constexpr ::gl::PixelType PixelType = Traits::PixelType;

  std::array<Type, Channels> channels;

  explicit Pixel()
      : Pixel{Type(0)}
  {
  }

  explicit constexpr Pixel(Type value) noexcept
      : channels{}
  {
    channels.fill(value);
  }

  template<typename... U>
  constexpr Pixel(Type value0, Type value1, U... tail) noexcept
      : channels{value0, value1, static_cast<Type>(tail)...}
  {
    static_assert(sizeof...(U) + 2 == _Channels, "Invalid constructor call");
  }

  constexpr bool operator==(const Self& rhs) const
  {
    return channels == rhs.channels;
  }

  constexpr bool operator!=(const Self& rhs) const
  {
    return !(*this == rhs);
  }
};

template<typename T, ::gl::PixelFormat _PixelFormat, ::gl::InternalFormat _InternalFormat>
inline Pixel<T, 4, _PixelFormat, _InternalFormat> mixAlpha(const Pixel<T, 4, _PixelFormat, _InternalFormat>& lhs,
                                                           const Pixel<T, 4, _PixelFormat, _InternalFormat>& rhs)
{
  const float bias = float(rhs.channels[3]) / std::numeric_limits<T>::max();
  return {static_cast<T>(lhs.channels[0] * (1 - bias) + rhs.channels[0] * bias),
          static_cast<T>(lhs.channels[1] * (1 - bias) + rhs.channels[1] * bias),
          static_cast<T>(lhs.channels[2] * (1 - bias) + rhs.channels[2] * bias),
          static_cast<T>(lhs.channels[3] * (1 - bias) + rhs.channels[3] * bias)};
}

template<typename T>
using SRGBA = Pixel<T, 4, ::gl::PixelFormat::Rgba, TypeTraits<T>::SrgbaSizedInternalFormat>;
using SRGBA8 = SRGBA<uint8_t>;

template<typename T>
using SRGB = Pixel<T, 3, ::gl::PixelFormat::Rgb, TypeTraits<T>::SrgbSizedInternalFormat>;
using SRGB8 = SRGB<uint8_t>;
// using SRGB16F = SRGB<::gl::core::Half>;
// using SRGB32F = SRGB<float>;

template<typename T>
using RGB = Pixel<T, 3, ::gl::PixelFormat::Rgb, TypeTraits<T>::RgbSizedInternalFormat>;
using RGB8 = RGB<uint8_t>;
using RGB16F = RGB<::gl::core::Half>;
using RGB32F = RGB<float>;

template<typename T>
using RG = Pixel<T, 3, ::gl::PixelFormat::Rg, TypeTraits<T>::RgSizedInternalFormat>;
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

  static constexpr ::gl::PixelFormat PixelFormat = ::gl::PixelFormat::Red;
  static constexpr ::gl::PixelType PixelType = Traits::PixelType;
  static constexpr auto InternalFormat = Traits::RSizedInternalFormat;

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
} // namespace render
