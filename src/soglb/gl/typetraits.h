#pragma once

#include "api/gl.hpp"

#include <cstdint>
#include <glm/fwd.hpp>

#if GLM_VERSION < 98
#  error "Your GLM version is too old, upgrade to 0.9.8 or higher"
#endif

namespace gl
{
namespace detail
{
struct InvalidValue
{
};
} // namespace detail

template<typename>
constexpr auto DrawElementsType = detail::InvalidValue{};
template<>
constexpr api::DrawElementsType DrawElementsType<uint8_t> = api::DrawElementsType::UnsignedByte;
template<>
constexpr api::DrawElementsType DrawElementsType<uint16_t> = api::DrawElementsType::UnsignedShort;
template<>
constexpr api::DrawElementsType DrawElementsType<uint32_t> = api::DrawElementsType::UnsignedInt;

template<typename>
constexpr auto VertexAttribType = detail::InvalidValue{};
template<>
constexpr api::VertexAttribType VertexAttribType<uint8_t> = api::VertexAttribType::UnsignedByte;
template<>
constexpr api::VertexAttribType VertexAttribType<uint16_t> = api::VertexAttribType::UnsignedShort;
template<>
constexpr api::VertexAttribType VertexAttribType<uint32_t> = api::VertexAttribType::UnsignedInt;
template<>
constexpr api::VertexAttribType VertexAttribType<int8_t> = api::VertexAttribType::Byte;
template<>
constexpr api::VertexAttribType VertexAttribType<int16_t> = api::VertexAttribType::Short;
template<>
constexpr api::VertexAttribType VertexAttribType<int32_t> = api::VertexAttribType::Int;
template<>
constexpr api::VertexAttribType VertexAttribType<float> = api::VertexAttribType::Float;
template<>
constexpr api::VertexAttribType VertexAttribType<api::core::Half> = api::VertexAttribType::HalfFloat;
template<int N>
constexpr api::VertexAttribType VertexAttribType<glm::vec<N, float, glm::defaultp>> = api::VertexAttribType::Float;

template<typename>
constexpr auto PixelType = detail::InvalidValue{};
template<>
constexpr api::PixelType PixelType<uint8_t> = api::PixelType::UnsignedByte;
template<>
constexpr api::PixelType PixelType<uint16_t> = api::PixelType::UnsignedShort;
template<>
constexpr api::PixelType PixelType<uint32_t> = api::PixelType::UnsignedInt;
template<>
constexpr api::PixelType PixelType<int8_t> = api::PixelType::Byte;
template<>
constexpr api::PixelType PixelType<int16_t> = api::PixelType::Short;
template<>
constexpr api::PixelType PixelType<int32_t> = api::PixelType::Int;
template<>
constexpr api::PixelType PixelType<float> = api::PixelType::Float;
template<>
constexpr api::PixelType PixelType<api::core::Half> = api::PixelType::Float;
template<int N>
constexpr api::PixelType PixelType<glm::vec<N, float, glm::defaultp>> = api::PixelType::Float;

template<typename>
constexpr auto ElementCount = detail::InvalidValue{};
template<>
constexpr api::core::SizeType ElementCount<uint8_t> = 1;
template<>
constexpr api::core::SizeType ElementCount<uint16_t> = 1;
template<>
constexpr api::core::SizeType ElementCount<uint32_t> = 1;
template<>
constexpr api::core::SizeType ElementCount<int8_t> = 1;
template<>
constexpr api::core::SizeType ElementCount<int16_t> = 1;
template<>
constexpr api::core::SizeType ElementCount<int32_t> = 1;
template<>
constexpr api::core::SizeType ElementCount<float> = 1;
template<>
constexpr api::core::SizeType ElementCount<api::core::Half> = 1;
template<int N>
constexpr api::core::SizeType ElementCount<glm::vec<N, float, glm::defaultp>> = N;

template<typename>
constexpr auto SrgbaSizedInternalFormat = detail::InvalidValue{};
template<typename>
constexpr auto SrgbSizedInternalFormat = detail::InvalidValue{};
template<typename>
constexpr auto RgbaSizedInternalFormat = detail::InvalidValue{};
template<typename>
constexpr auto RgbSizedInternalFormat = detail::InvalidValue{};
template<typename>
constexpr auto RgSizedInternalFormat = detail::InvalidValue{};
template<typename>
constexpr auto RSizedInternalFormat = detail::InvalidValue{};

template<>
constexpr api::SizedInternalFormat SrgbaSizedInternalFormat<uint8_t> = api::SizedInternalFormat::Srgb8Alpha8;
template<>
constexpr api::SizedInternalFormat SrgbSizedInternalFormat<uint8_t> = api::SizedInternalFormat::Srgb8;
template<>
constexpr api::SizedInternalFormat RgbaSizedInternalFormat<uint8_t> = api::SizedInternalFormat::Rgba8;
template<>
constexpr api::SizedInternalFormat RgbSizedInternalFormat<uint8_t> = api::SizedInternalFormat::Rgb8;
template<>
constexpr api::SizedInternalFormat RgSizedInternalFormat<uint8_t> = api::SizedInternalFormat::Rg8;
template<>
constexpr api::SizedInternalFormat RSizedInternalFormat<uint8_t> = api::SizedInternalFormat::R8;

template<>
constexpr api::SizedInternalFormat RgbaSizedInternalFormat<uint16_t> = api::SizedInternalFormat::Rgba16ui;
template<>
constexpr api::SizedInternalFormat RgbSizedInternalFormat<uint16_t> = api::SizedInternalFormat::Rgb16ui;
template<>
constexpr api::SizedInternalFormat RgSizedInternalFormat<uint16_t> = api::SizedInternalFormat::Rg16ui;
template<>
constexpr api::SizedInternalFormat RSizedInternalFormat<uint16_t> = api::SizedInternalFormat::R16ui;

template<>
constexpr api::SizedInternalFormat RgbaSizedInternalFormat<uint32_t> = api::SizedInternalFormat::Rgba32ui;
template<>
constexpr api::SizedInternalFormat RgbSizedInternalFormat<uint32_t> = api::SizedInternalFormat::Rgb32ui;
template<>
constexpr api::SizedInternalFormat RgSizedInternalFormat<uint32_t> = api::SizedInternalFormat::Rg32ui;
template<>
constexpr api::SizedInternalFormat RSizedInternalFormat<uint32_t> = api::SizedInternalFormat::R32ui;

template<>
constexpr api::SizedInternalFormat RgbaSizedInternalFormat<int8_t> = api::SizedInternalFormat::Rgba8i;
template<>
constexpr api::SizedInternalFormat RgbSizedInternalFormat<int8_t> = api::SizedInternalFormat::Rgb8i;
template<>
constexpr api::SizedInternalFormat RgSizedInternalFormat<int8_t> = api::SizedInternalFormat::Rg8i;
template<>
constexpr api::SizedInternalFormat RSizedInternalFormat<int8_t> = api::SizedInternalFormat::R8i;

template<>
constexpr api::SizedInternalFormat RgbaSizedInternalFormat<int16_t> = api::SizedInternalFormat::Rgba16i;
template<>
constexpr api::SizedInternalFormat RgbSizedInternalFormat<int16_t> = api::SizedInternalFormat::Rgb16i;
template<>
constexpr api::SizedInternalFormat RgSizedInternalFormat<int16_t> = api::SizedInternalFormat::Rg16i;
template<>
constexpr api::SizedInternalFormat RSizedInternalFormat<int16_t> = api::SizedInternalFormat::R16i;

template<>
constexpr api::SizedInternalFormat RgbaSizedInternalFormat<int32_t> = api::SizedInternalFormat::Rgba32i;
template<>
constexpr api::SizedInternalFormat RgbSizedInternalFormat<int32_t> = api::SizedInternalFormat::Rgb32i;
template<>
constexpr api::SizedInternalFormat RgSizedInternalFormat<int32_t> = api::SizedInternalFormat::Rg32i;
template<>
constexpr api::SizedInternalFormat RSizedInternalFormat<int32_t> = api::SizedInternalFormat::R32i;

template<>
constexpr api::SizedInternalFormat RgbaSizedInternalFormat<float> = api::SizedInternalFormat::Rgba32f;
template<>
constexpr api::SizedInternalFormat RgbSizedInternalFormat<float> = api::SizedInternalFormat::Rgb32f;
template<>
constexpr api::SizedInternalFormat RgSizedInternalFormat<float> = api::SizedInternalFormat::Rg32f;
template<>
constexpr api::SizedInternalFormat RSizedInternalFormat<float> = api::SizedInternalFormat::R32f;

template<>
constexpr api::SizedInternalFormat RgbaSizedInternalFormat<api::core::Half> = api::SizedInternalFormat::Rgba16f;
template<>
constexpr api::SizedInternalFormat RgbSizedInternalFormat<api::core::Half> = api::SizedInternalFormat::Rgb16f;
template<>
constexpr api::SizedInternalFormat RgSizedInternalFormat<api::core::Half> = api::SizedInternalFormat::Rg16f;
template<>
constexpr api::SizedInternalFormat RSizedInternalFormat<api::core::Half> = api::SizedInternalFormat::R16f;

template<typename>
constexpr auto DepthInternalFormat = detail::InvalidValue{};
template<>
constexpr api::SizedInternalFormat DepthInternalFormat<int16_t> = api::SizedInternalFormat::DepthComponent16Arb;
template<>
constexpr api::SizedInternalFormat DepthInternalFormat<int32_t> = api::SizedInternalFormat::DepthComponent32Arb;
template<>
constexpr api::SizedInternalFormat DepthInternalFormat<float> = api::SizedInternalFormat::DepthComponent32f;
} // namespace gl
