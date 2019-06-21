#pragma once

#include "glassert.h"

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/packing.hpp>

namespace render
{
namespace gl
{
template<typename T>
struct TypeTraits;

template<>
struct TypeTraits<uint8_t>
{
    static const constexpr ::gl::DrawElementsType DrawElementsType = ::gl::DrawElementsType::UnsignedByte;
    static const constexpr ::gl::VertexAttribPointerType VertexAttribPointerType
        = ::gl::VertexAttribPointerType::UnsignedByte;
    static const constexpr ::gl::PixelType PixelType = ::gl::PixelType::UnsignedByte;

    static const constexpr ::gl::core::SizeType ElementCount = 1;

    static const constexpr ::gl::InternalFormat SrgbaSizedInternalFormat = ::gl::InternalFormat::Srgb8Alpha8;
    static const constexpr ::gl::InternalFormat SrgbSizedInternalFormat = ::gl::InternalFormat::Srgb8;

    static const constexpr ::gl::InternalFormat RgbaSizedInternalFormat = ::gl::InternalFormat::Rgba8;
    static const constexpr ::gl::InternalFormat RgbSizedInternalFormat = ::gl::InternalFormat::Rgb8;
};

template<>
struct TypeTraits<uint16_t>
{
    static const constexpr ::gl::DrawElementsType DrawElementsType = ::gl::DrawElementsType::UnsignedShort;
    static const constexpr ::gl::VertexAttribPointerType VertexAttribPointerType
        = ::gl::VertexAttribPointerType::UnsignedShort;
    static const constexpr ::gl::PixelType PixelType = ::gl::PixelType::UnsignedShort;

    static const constexpr ::gl::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<uint32_t>
{
    static const constexpr ::gl::DrawElementsType DrawElementsType = ::gl::DrawElementsType::UnsignedInt;
    static const constexpr ::gl::VertexAttribPointerType VertexAttribPointerType = ::gl::VertexAttribPointerType::UnsignedInt;
    static const constexpr ::gl::PixelType PixelType = ::gl::PixelType::UnsignedInt;

    static const constexpr ::gl::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<int8_t>
{
    static const constexpr ::gl::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<int16_t>
{
    static const constexpr ::gl::VertexAttribPointerType VertexAttribPointerType = ::gl::VertexAttribPointerType::Short;
    static const constexpr ::gl::PixelType PixelType = ::gl::PixelType::Short;
    static const constexpr ::gl::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<int32_t>
{
    static const constexpr ::gl::VertexAttribPointerType VertexAttribPointerType = ::gl::VertexAttribPointerType::Int;
    static const constexpr ::gl::PixelType PixelType = ::gl::PixelType::Int;
    static const constexpr ::gl::core::SizeType ElementCount = 1;
};

template<>
struct TypeTraits<float>
{
    static const constexpr ::gl::VertexAttribPointerType VertexAttribPointerType = ::gl::VertexAttribPointerType::Float;
    static const constexpr ::gl::PixelType PixelType = ::gl::PixelType::Float;
    static const constexpr ::gl::core::SizeType ElementCount = 1;

    static const constexpr ::gl::InternalFormat RgbSizedInternalFormat = ::gl::InternalFormat::Rgb32f;
    static const constexpr ::gl::InternalFormat RSizedInternalFormat = ::gl::InternalFormat::R32f;
};

template<int N>
struct TypeTraits<glm::vec<N, float, glm::defaultp>>
{
    static const constexpr ::gl::VertexAttribPointerType VertexAttribPointerType = ::gl::VertexAttribPointerType::Float;
    static const constexpr ::gl::PixelType PixelType = ::gl::PixelType::Float;
    static const constexpr ::gl::core::SizeType ElementCount = N;
};

template<>
struct TypeTraits<::gl::core::Half>
{
    static const constexpr ::gl::VertexAttribPointerType VertexAttribPointerType = ::gl::VertexAttribPointerType::HalfFloat;
    static const constexpr ::gl::PixelType PixelType = ::gl::PixelType::Float;
    static const constexpr ::gl::core::SizeType ElementCount = 1;

    static const constexpr ::gl::InternalFormat RgbSizedInternalFormat = ::gl::InternalFormat::Rgb16f;
    static const constexpr ::gl::InternalFormat RSizedInternalFormat = ::gl::InternalFormat::R16f;
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
} // namespace render
