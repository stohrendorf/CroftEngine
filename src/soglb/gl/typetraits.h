#pragma once

#include "api/gl.hpp" // IWYU pragma: export

#include <glm/fwd.hpp>

#if GLM_VERSION < 98 && !defined(__cppcheck__)
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
inline constexpr auto DrawElementsType = detail::InvalidValue{};
template<>
inline constexpr auto DrawElementsType<uint8_t> = api::DrawElementsType::UnsignedByte;
template<>
inline constexpr auto DrawElementsType<uint16_t> = api::DrawElementsType::UnsignedShort;
template<>
inline constexpr auto DrawElementsType<uint32_t> = api::DrawElementsType::UnsignedInt;

template<typename>
inline constexpr auto VertexAttribType = detail::InvalidValue{};
template<>
inline constexpr auto VertexAttribType<uint8_t> = api::VertexAttribType::UnsignedByte;
template<>
inline constexpr auto VertexAttribType<uint16_t> = api::VertexAttribType::UnsignedShort;
template<>
inline constexpr auto VertexAttribType<uint32_t> = api::VertexAttribType::UnsignedInt;
template<>
inline constexpr auto VertexAttribType<int8_t> = api::VertexAttribType::Byte;
template<>
inline constexpr auto VertexAttribType<int16_t> = api::VertexAttribType::Short;
template<>
inline constexpr auto VertexAttribType<int32_t> = api::VertexAttribType::Int;
template<>
inline constexpr auto VertexAttribType<float> = api::VertexAttribType::Float;
template<>
inline constexpr auto VertexAttribType<api::core::Half> = api::VertexAttribType::HalfFloat;
template<int N>
inline constexpr auto VertexAttribType<glm::vec<N, float>> = api::VertexAttribType::Float;
template<int C, int R>
inline constexpr auto VertexAttribType<glm::mat<C, R, float>> = api::VertexAttribType::Float;

template<typename>
inline constexpr auto PixelType = detail::InvalidValue{};
template<>
inline constexpr auto PixelType<uint8_t> = api::PixelType::UnsignedByte;
template<>
inline constexpr auto PixelType<uint16_t> = api::PixelType::UnsignedShort;
template<>
inline constexpr auto PixelType<uint32_t> = api::PixelType::UnsignedInt;
template<>
inline constexpr auto PixelType<int8_t> = api::PixelType::Byte;
template<>
inline constexpr auto PixelType<int16_t> = api::PixelType::Short;
template<>
inline constexpr auto PixelType<int32_t> = api::PixelType::Int;
template<>
inline constexpr auto PixelType<float> = api::PixelType::Float;
template<>
inline constexpr auto PixelType<api::core::Half> = api::PixelType::HalfFloat;
template<int N>
inline constexpr auto PixelType<glm::vec<N, float>> = api::PixelType::Float;

template<typename>
inline constexpr auto ElementCount = detail::InvalidValue{};
template<>
inline constexpr api::core::SizeType ElementCount<uint8_t> = 1;
template<>
inline constexpr api::core::SizeType ElementCount<uint16_t> = 1;
template<>
inline constexpr api::core::SizeType ElementCount<uint32_t> = 1;
template<>
inline constexpr api::core::SizeType ElementCount<int8_t> = 1;
template<>
inline constexpr api::core::SizeType ElementCount<int16_t> = 1;
template<>
inline constexpr api::core::SizeType ElementCount<int32_t> = 1;
template<>
inline constexpr api::core::SizeType ElementCount<float> = 1;
template<>
inline constexpr api::core::SizeType ElementCount<api::core::Half> = 1;
template<int N>
inline constexpr api::core::SizeType ElementCount<glm::vec<N, float>> = N;
template<int C, int R>
inline constexpr api::core::SizeType ElementCount<glm::mat<C, R, float>> = C * R;

template<typename>
inline constexpr auto SrgbaSizedInternalFormat = detail::InvalidValue{};
template<typename>
inline constexpr auto SrgbSizedInternalFormat = detail::InvalidValue{};
template<typename>
inline constexpr auto RgbaSizedInternalFormat = detail::InvalidValue{};
template<typename>
inline constexpr auto RgbSizedInternalFormat = detail::InvalidValue{};
template<typename>
inline constexpr auto RgSizedInternalFormat = detail::InvalidValue{};
template<typename>
inline constexpr auto RSizedInternalFormat = detail::InvalidValue{};

template<>
inline constexpr auto SrgbaSizedInternalFormat<uint8_t> = api::SizedInternalFormat::Srgb8Alpha8;
template<>
inline constexpr auto SrgbSizedInternalFormat<uint8_t> = api::SizedInternalFormat::Srgb8;
template<>
inline constexpr auto RgbaSizedInternalFormat<uint8_t> = api::SizedInternalFormat::Rgba8;
template<>
inline constexpr auto RgbSizedInternalFormat<uint8_t> = api::SizedInternalFormat::Rgb8;
template<>
inline constexpr auto RgSizedInternalFormat<uint8_t> = api::SizedInternalFormat::Rg8;
template<>
inline constexpr auto RSizedInternalFormat<uint8_t> = api::SizedInternalFormat::R8;

template<>
inline constexpr auto RgbaSizedInternalFormat<uint16_t> = api::SizedInternalFormat::Rgba16ui;
template<>
inline constexpr auto RgbSizedInternalFormat<uint16_t> = api::SizedInternalFormat::Rgb16ui;
template<>
inline constexpr auto RgSizedInternalFormat<uint16_t> = api::SizedInternalFormat::Rg16ui;
template<>
inline constexpr auto RSizedInternalFormat<uint16_t> = api::SizedInternalFormat::R16ui;

template<>
inline constexpr auto RgbaSizedInternalFormat<uint32_t> = api::SizedInternalFormat::Rgba32ui;
template<>
inline constexpr auto RgbSizedInternalFormat<uint32_t> = api::SizedInternalFormat::Rgb32ui;
template<>
inline constexpr auto RgSizedInternalFormat<uint32_t> = api::SizedInternalFormat::Rg32ui;
template<>
inline constexpr auto RSizedInternalFormat<uint32_t> = api::SizedInternalFormat::R32ui;

template<>
inline constexpr auto RgbaSizedInternalFormat<int8_t> = api::SizedInternalFormat::Rgba8i;
template<>
inline constexpr auto RgbSizedInternalFormat<int8_t> = api::SizedInternalFormat::Rgb8i;
template<>
inline constexpr auto RgSizedInternalFormat<int8_t> = api::SizedInternalFormat::Rg8i;
template<>
inline constexpr auto RSizedInternalFormat<int8_t> = api::SizedInternalFormat::R8i;

template<>
inline constexpr auto RgbaSizedInternalFormat<int16_t> = api::SizedInternalFormat::Rgba16i;
template<>
inline constexpr auto RgbSizedInternalFormat<int16_t> = api::SizedInternalFormat::Rgb16i;
template<>
inline constexpr auto RgSizedInternalFormat<int16_t> = api::SizedInternalFormat::Rg16i;
template<>
inline constexpr auto RSizedInternalFormat<int16_t> = api::SizedInternalFormat::R16i;

template<>
inline constexpr auto RgbaSizedInternalFormat<int32_t> = api::SizedInternalFormat::Rgba32i;
template<>
inline constexpr auto RgbSizedInternalFormat<int32_t> = api::SizedInternalFormat::Rgb32i;
template<>
inline constexpr auto RgSizedInternalFormat<int32_t> = api::SizedInternalFormat::Rg32i;
template<>
inline constexpr auto RSizedInternalFormat<int32_t> = api::SizedInternalFormat::R32i;

template<>
inline constexpr auto RgbaSizedInternalFormat<float> = api::SizedInternalFormat::Rgba32f;
template<>
inline constexpr auto RgbSizedInternalFormat<float> = api::SizedInternalFormat::Rgb32f;
template<>
inline constexpr auto RgSizedInternalFormat<float> = api::SizedInternalFormat::Rg32f;
template<>
inline constexpr auto RSizedInternalFormat<float> = api::SizedInternalFormat::R32f;

template<>
inline constexpr auto RgbaSizedInternalFormat<api::core::Half> = api::SizedInternalFormat::Rgba16f;
template<>
inline constexpr auto RgbSizedInternalFormat<api::core::Half> = api::SizedInternalFormat::Rgb16f;
template<>
inline constexpr auto RgSizedInternalFormat<api::core::Half> = api::SizedInternalFormat::Rg16f;
template<>
inline constexpr auto RSizedInternalFormat<api::core::Half> = api::SizedInternalFormat::R16f;

template<typename>
inline constexpr auto DepthInternalFormat = detail::InvalidValue{};
template<>
inline constexpr auto DepthInternalFormat<int16_t> = api::SizedInternalFormat::DepthComponent16;
template<>
inline constexpr auto DepthInternalFormat<int32_t> = api::SizedInternalFormat::DepthComponent32;
template<>
inline constexpr auto DepthInternalFormat<float> = api::SizedInternalFormat::DepthComponent32f;
} // namespace gl
