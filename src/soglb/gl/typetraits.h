#pragma once

#include "glassert.h"

#include <cstdint>
#include <glm/gtc/packing.hpp>

namespace gl
{
template<typename T>
struct TypeTraits;

template<>
struct TypeTraits<uint8_t>
{
  static constexpr api::DrawElementsType DrawElementsType = api::DrawElementsType::UnsignedByte;
  static constexpr api::VertexAttribType VertexAttribType = api::VertexAttribType::UnsignedByte;
  static constexpr api::PixelType PixelType = api::PixelType::UnsignedByte;

  static constexpr api::core::SizeType ElementCount = 1;

  static constexpr api::InternalFormat SrgbaSizedInternalFormat = api::InternalFormat::Srgb8Alpha8;
  static constexpr api::InternalFormat SrgbSizedInternalFormat = api::InternalFormat::Srgb8;

  static constexpr api::InternalFormat RgbaSizedInternalFormat = api::InternalFormat::Rgba8;
  static constexpr api::InternalFormat RgbSizedInternalFormat = api::InternalFormat::Rgb8;
  static constexpr api::InternalFormat RgSizedInternalFormat = api::InternalFormat::Rg8;
  static constexpr api::InternalFormat RSizedInternalFormat = api::InternalFormat::R8;
};

template<>
struct TypeTraits<uint16_t>
{
  static constexpr api::DrawElementsType DrawElementsType = api::DrawElementsType::UnsignedShort;
  static constexpr api::VertexAttribType VertexAttribType = api::VertexAttribType::UnsignedShort;
  static constexpr api::PixelType PixelType = api::PixelType::UnsignedShort;

  static constexpr api::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<uint32_t>
{
  static constexpr api::DrawElementsType DrawElementsType = api::DrawElementsType::UnsignedInt;
  static constexpr api::VertexAttribType VertexAttribType = api::VertexAttribType::UnsignedInt;
  static constexpr api::PixelType PixelType = api::PixelType::UnsignedInt;

  static constexpr api::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<int8_t>
{
  static constexpr api::VertexAttribType VertexAttribType = api::VertexAttribType::Byte;
  static constexpr api::PixelType PixelType = api::PixelType::Byte;
  static constexpr api::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<int16_t>
{
  static constexpr api::VertexAttribType VertexAttribType = api::VertexAttribType::Short;
  static constexpr api::PixelType PixelType = api::PixelType::Short;
  static constexpr api::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<int32_t>
{
  static constexpr api::VertexAttribType VertexAttribType = api::VertexAttribType::Int;
  static constexpr api::PixelType PixelType = api::PixelType::Int;
  static constexpr api::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<float>
{
  static constexpr api::VertexAttribType VertexAttribType = api::VertexAttribType::Float;
  static constexpr api::PixelType PixelType = api::PixelType::Float;
  static constexpr api::core::SizeType ElementCount = 1;

  static constexpr api::InternalFormat SrgbaSizedInternalFormat = api::InternalFormat::Srgb8Alpha8;
  static constexpr api::InternalFormat SrgbSizedInternalFormat = api::InternalFormat::Srgb8;

  static constexpr api::InternalFormat RgbaSizedInternalFormat = api::InternalFormat::Rgba32f;
  static constexpr api::InternalFormat RgbSizedInternalFormat = api::InternalFormat::Rgb32f;
  static constexpr api::InternalFormat RgSizedInternalFormat = api::InternalFormat::Rg32f;
  static constexpr api::InternalFormat RSizedInternalFormat = api::InternalFormat::R32f;
  static constexpr api::InternalFormat DepthInternalFormat = api::InternalFormat::DepthComponent32f;
};

#if GLM_VERSION < 98
template<>
struct TypeTraits<glm::vec1>
{
  static constexpr api::VertexAttribType VertexAttribType = api::VertexAttribType::Float;
  static constexpr api::PixelType PixelType = api::PixelType::Float;
  static constexpr api::core::SizeType ElementCount = 1;
};
template<>
struct TypeTraits<glm::vec2>
{
  static constexpr api::VertexAttribType VertexAttribType = api::VertexAttribType::Float;
  static constexpr api::PixelType PixelType = api::PixelType::Float;
  static constexpr api::core::SizeType ElementCount = 2;
};
template<>
struct TypeTraits<glm::vec3>
{
  static constexpr api::VertexAttribType VertexAttribType = api::VertexAttribType::Float;
  static constexpr api::PixelType PixelType = api::PixelType::Float;
  static constexpr api::core::SizeType ElementCount = 3;
};
template<>
struct TypeTraits<glm::vec4>
{
  static constexpr api::VertexAttribType VertexAttribType = api::VertexAttribType::Float;
  static constexpr api::PixelType PixelType = api::PixelType::Float;
  static constexpr api::core::SizeType ElementCount = 4;
};
#else
template<int N>
struct TypeTraits<glm::vec<N, float, glm::defaultp>>
{
  static constexpr api::VertexAttribType VertexAttribType = api::VertexAttribType::Float;
  static constexpr api::PixelType PixelType = api::PixelType::Float;
  static constexpr api::core::SizeType ElementCount = N;
};
#endif

template<>
struct TypeTraits<api::core::Half>
{
  static constexpr api::VertexAttribType VertexAttribType = api::VertexAttribType::HalfFloat;
  static constexpr api::PixelType PixelType = api::PixelType::Float;
  static constexpr api::core::SizeType ElementCount = 1;

  static constexpr api::InternalFormat RgbaSizedInternalFormat = api::InternalFormat::Rgba16f;
  static constexpr api::InternalFormat RgbSizedInternalFormat = api::InternalFormat::Rgb16f;
  static constexpr api::InternalFormat RgSizedInternalFormat = api::InternalFormat::Rg16f;
  static constexpr api::InternalFormat RSizedInternalFormat = api::InternalFormat::R16f;
};

template<typename T>
struct TypeTraits<const T> : TypeTraits<T>
{
};

template<typename T>
struct TypeTraits<T&> : TypeTraits<T>
{
};
} // namespace gl
