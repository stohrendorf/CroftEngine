#pragma once

#include "glassert.h"

#include <cstdint>
#include <glm/gtc/packing.hpp>

namespace render::gl
{
template<typename T>
struct TypeTraits;

template<>
struct TypeTraits<uint8_t>
{
  static constexpr ::gl::DrawElementsType DrawElementsType = ::gl::DrawElementsType::UnsignedByte;
  static constexpr ::gl::VertexAttribPointerType VertexAttribPointerType = ::gl::VertexAttribPointerType::UnsignedByte;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::UnsignedByte;

  static constexpr ::gl::core::SizeType ElementCount = 1;

  static constexpr ::gl::InternalFormat SrgbaSizedInternalFormat = ::gl::InternalFormat::Srgb8Alpha8;
  static constexpr ::gl::InternalFormat SrgbSizedInternalFormat = ::gl::InternalFormat::Srgb8;

  static constexpr ::gl::InternalFormat RgbaSizedInternalFormat = ::gl::InternalFormat::Rgba8;
  static constexpr ::gl::InternalFormat RgbSizedInternalFormat = ::gl::InternalFormat::Rgb8;
  static constexpr ::gl::InternalFormat RgSizedInternalFormat = ::gl::InternalFormat::Rg8;
  static constexpr ::gl::InternalFormat RSizedInternalFormat = ::gl::InternalFormat::R8;
};

template<>
struct TypeTraits<uint16_t>
{
  static constexpr ::gl::DrawElementsType DrawElementsType = ::gl::DrawElementsType::UnsignedShort;
  static constexpr ::gl::VertexAttribPointerType VertexAttribPointerType = ::gl::VertexAttribPointerType::UnsignedShort;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::UnsignedShort;

  static constexpr ::gl::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<uint32_t>
{
  static constexpr ::gl::DrawElementsType DrawElementsType = ::gl::DrawElementsType::UnsignedInt;
  static constexpr ::gl::VertexAttribPointerType VertexAttribPointerType = ::gl::VertexAttribPointerType::UnsignedInt;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::UnsignedInt;

  static constexpr ::gl::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<int8_t>
{
  static constexpr ::gl::VertexAttribPointerType VertexAttribPointerType = ::gl::VertexAttribPointerType::Byte;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::Byte;
  static constexpr ::gl::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<int16_t>
{
  static constexpr ::gl::VertexAttribPointerType VertexAttribPointerType = ::gl::VertexAttribPointerType::Short;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::Short;
  static constexpr ::gl::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<int32_t>
{
  static constexpr ::gl::VertexAttribPointerType VertexAttribPointerType = ::gl::VertexAttribPointerType::Int;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::Int;
  static constexpr ::gl::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<float>
{
  static constexpr ::gl::VertexAttribPointerType VertexAttribPointerType = ::gl::VertexAttribPointerType::Float;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::Float;
  static constexpr ::gl::core::SizeType ElementCount = 1;

  static constexpr ::gl::InternalFormat RgbaSizedInternalFormat = ::gl::InternalFormat::Rgba32f;
  static constexpr ::gl::InternalFormat RgbSizedInternalFormat = ::gl::InternalFormat::Rgb32f;
  static constexpr ::gl::InternalFormat RgSizedInternalFormat = ::gl::InternalFormat::Rg32f;
  static constexpr ::gl::InternalFormat RSizedInternalFormat = ::gl::InternalFormat::R32f;
};

template<int N>
struct TypeTraits<glm::vec<N, float, glm::defaultp>>
{
  static constexpr ::gl::VertexAttribPointerType VertexAttribPointerType = ::gl::VertexAttribPointerType::Float;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::Float;
  static constexpr ::gl::core::SizeType ElementCount = N;
};

template<>
struct TypeTraits<::gl::core::Half>
{
  static constexpr ::gl::VertexAttribPointerType VertexAttribPointerType = ::gl::VertexAttribPointerType::HalfFloat;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::Float;
  static constexpr ::gl::core::SizeType ElementCount = 1;

  static constexpr ::gl::InternalFormat RgbaSizedInternalFormat = ::gl::InternalFormat::Rgba16f;
  static constexpr ::gl::InternalFormat RgbSizedInternalFormat = ::gl::InternalFormat::Rgb16f;
  static constexpr ::gl::InternalFormat RgSizedInternalFormat = ::gl::InternalFormat::Rg16f;
  static constexpr ::gl::InternalFormat RSizedInternalFormat = ::gl::InternalFormat::R16f;
};

template<typename T>
struct TypeTraits<const T> : TypeTraits<T>
{
};

template<typename T>
struct TypeTraits<T&> : TypeTraits<T>
{
};
} // namespace render
