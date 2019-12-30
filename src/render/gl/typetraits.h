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
  static constexpr ::gl::VertexAttribType VertexAttribType = ::gl::VertexAttribType::UnsignedByte;
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
  static constexpr ::gl::VertexAttribType VertexAttribType = ::gl::VertexAttribType::UnsignedShort;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::UnsignedShort;

  static constexpr ::gl::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<uint32_t>
{
  static constexpr ::gl::DrawElementsType DrawElementsType = ::gl::DrawElementsType::UnsignedInt;
  static constexpr ::gl::VertexAttribType VertexAttribType = ::gl::VertexAttribType::UnsignedInt;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::UnsignedInt;

  static constexpr ::gl::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<int8_t>
{
  static constexpr ::gl::VertexAttribType VertexAttribType = ::gl::VertexAttribType::Byte;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::Byte;
  static constexpr ::gl::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<int16_t>
{
  static constexpr ::gl::VertexAttribType VertexAttribType = ::gl::VertexAttribType::Short;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::Short;
  static constexpr ::gl::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<int32_t>
{
  static constexpr ::gl::VertexAttribType VertexAttribType = ::gl::VertexAttribType::Int;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::Int;
  static constexpr ::gl::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<float>
{
  static constexpr ::gl::VertexAttribType VertexAttribType = ::gl::VertexAttribType::Float;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::Float;
  static constexpr ::gl::core::SizeType ElementCount = 1;

  static constexpr ::gl::InternalFormat RgbaSizedInternalFormat = ::gl::InternalFormat::Rgba32f;
  static constexpr ::gl::InternalFormat RgbSizedInternalFormat = ::gl::InternalFormat::Rgb32f;
  static constexpr ::gl::InternalFormat RgSizedInternalFormat = ::gl::InternalFormat::Rg32f;
  static constexpr ::gl::InternalFormat RSizedInternalFormat = ::gl::InternalFormat::R32f;
};

#if GLM_VERSION < 98
template<>
struct TypeTraits<glm::vec1>
{
  static constexpr ::gl::VertexAttribType VertexAttribType = ::gl::VertexAttribType::Float;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::Float;
  static constexpr ::gl::core::SizeType ElementCount = 1;
};
template<>
struct TypeTraits<glm::vec2>
{
  static constexpr ::gl::VertexAttribType VertexAttribType = ::gl::VertexAttribType::Float;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::Float;
  static constexpr ::gl::core::SizeType ElementCount = 2;
};
template<>
struct TypeTraits<glm::vec3>
{
  static constexpr ::gl::VertexAttribType VertexAttribType = ::gl::VertexAttribType::Float;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::Float;
  static constexpr ::gl::core::SizeType ElementCount = 3;
};
template<>
struct TypeTraits<glm::vec4>
{
  static constexpr ::gl::VertexAttribType VertexAttribType = ::gl::VertexAttribType::Float;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::Float;
  static constexpr ::gl::core::SizeType ElementCount = 4;
};
#else
template<int N>
struct TypeTraits<glm::vec<N, float, glm::defaultp>>
{
  static constexpr ::gl::VertexAttribType VertexAttribType = ::gl::VertexAttribType::Float;
  static constexpr ::gl::PixelType PixelType = ::gl::PixelType::Float;
  static constexpr ::gl::core::SizeType ElementCount = N;
};
#endif

template<>
struct TypeTraits<::gl::core::Half>
{
  static constexpr ::gl::VertexAttribType VertexAttribType = ::gl::VertexAttribType::HalfFloat;
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
} // namespace render::gl
