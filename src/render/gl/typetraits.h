#pragma once

#include "glassert.h"

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/packing.hpp>

namespace render
{
namespace gl
{
enum class DrawElementsType : RawGlEnum
{
    UByte = (RawGlEnum)::gl::GL_UNSIGNED_BYTE,
    UShort = (RawGlEnum)::gl::GL_UNSIGNED_SHORT,
    UInt = (RawGlEnum)::gl::GL_UNSIGNED_INT,
};

#define _GL_MAKE_CHANNEL_ENUMERATORS(CHN_ENUM, CHN)                                                        \
    CHN_ENUM##8 = (RawGlEnum)::gl::GL_R8, CHN_ENUM##8Snorm = (RawGlEnum)::gl::GL_##CHN##8_SNORM,           \
    CHN_ENUM##16 = (RawGlEnum)::gl::GL_##CHN##16, CHN_ENUM##16Snorm = (RawGlEnum)::gl::GL_##CHN##16_SNORM, \
    CHN_ENUM##16f = (RawGlEnum)::gl::GL_##CHN##16F, CHN_ENUM##32f = (RawGlEnum)::gl::GL_##CHN##32F,        \
    CHN_ENUM##8i = (RawGlEnum)::gl::GL_##CHN##8I, CHN_ENUM##16i = (RawGlEnum)::gl::GL_##CHN##16I,          \
    CHN_ENUM##32i = (RawGlEnum)::gl::GL_##CHN##32I, CHN_ENUM##8ui = (RawGlEnum)::gl::GL_##CHN##8UI,        \
    CHN_ENUM##16ui = (RawGlEnum)::gl::GL_##CHN##16UI, CHN_ENUM##32ui = (RawGlEnum)::gl::GL_##CHN##32UI

enum class InternalFormat : RawGlEnum
{
    Alpha12 = (RawGlEnum)::gl::GL_ALPHA12,
    Alpha16 = (RawGlEnum)::gl::GL_ALPHA16,
    Alpha4 = (RawGlEnum)::gl::GL_ALPHA4,
    Alpha8 = (RawGlEnum)::gl::GL_ALPHA8,
    Intensity = (RawGlEnum)::gl::GL_INTENSITY,
    Intensity12 = (RawGlEnum)::gl::GL_INTENSITY12,
    Intensity16 = (RawGlEnum)::gl::GL_INTENSITY16,
    Intensity4 = (RawGlEnum)::gl::GL_INTENSITY4,
    Intensity8 = (RawGlEnum)::gl::GL_INTENSITY8,
    Luminance12 = (RawGlEnum)::gl::GL_LUMINANCE12,
    Luminance12Alpha2 = (RawGlEnum)::gl::GL_LUMINANCE12_ALPHA12,
    Luminance12Alpha4 = (RawGlEnum)::gl::GL_LUMINANCE12_ALPHA4,
    Luminance16 = (RawGlEnum)::gl::GL_LUMINANCE16,
    Luminance16Alpha16 = (RawGlEnum)::gl::GL_LUMINANCE16_ALPHA16,
    Luminance4 = (RawGlEnum)::gl::GL_LUMINANCE4,
    Luminance4Alpha4 = (RawGlEnum)::gl::GL_LUMINANCE4_ALPHA4,
    Luminance6Alpha2 = (RawGlEnum)::gl::GL_LUMINANCE6_ALPHA2,
    Luminance8 = (RawGlEnum)::gl::GL_LUMINANCE8,
    Luminance8Alpha8 = (RawGlEnum)::gl::GL_LUMINANCE8_ALPHA8,

    // Base internal format: GL_RED
    Red = (RawGlEnum)::gl::GL_RED,
    _GL_MAKE_CHANNEL_ENUMERATORS(R, R),

    // Base internal format: GL_RG
    Rg = (RawGlEnum)::gl::GL_RG,
    _GL_MAKE_CHANNEL_ENUMERATORS(Rg, RG),

    // Base internal format: GL_RGB
    Rgb = (RawGlEnum)::gl::GL_RGB,
    Rgb4 = (RawGlEnum)::gl::GL_RGB4,
    Rgb5 = (RawGlEnum)::gl::GL_RGB5,
    Rgb12 = (RawGlEnum)::gl::GL_RGB12,
    R3G3B2 = (RawGlEnum)::gl::GL_R3_G3_B2,
    R11fG11fB10f = (RawGlEnum)::gl::GL_R11F_G11F_B10F,
    RGB9E5 = (RawGlEnum)::gl::GL_RGB9_E5,
    _GL_MAKE_CHANNEL_ENUMERATORS(Rgb, RGB),
    Srgb = (RawGlEnum)::gl::GL_SRGB,
    SrgbAlpha = (RawGlEnum)::gl::GL_SRGB_ALPHA,
    Srgb8 = (RawGlEnum)::gl::GL_SRGB8,
    Srgb8Alpha8 = (RawGlEnum)::gl::GL_SRGB8_ALPHA8,

    // Base internal format: GL_RGBA
    Rgba = (RawGlEnum)::gl::GL_RGBA,
    Rgba4 = (RawGlEnum)::gl::GL_RGBA4,
    Rgb5A1 = (RawGlEnum)::gl::GL_RGB5_A1,
    Rgb10A2 = (RawGlEnum)::gl::GL_RGB10_A2,
    Rgba12 = (RawGlEnum)::gl::GL_RGBA12,
    Rgb10A2ui = (RawGlEnum)::gl::GL_RGB10_A2UI,
    _GL_MAKE_CHANNEL_ENUMERATORS(Rgba, RGBA),

    // Base internal format: GL_DEPTH_COMPONENT
    DepthComponent = (RawGlEnum)::gl::GL_DEPTH_COMPONENT,
    DepthComponent16 = (RawGlEnum)::gl::GL_DEPTH_COMPONENT16,
    DepthComponent32f = (RawGlEnum)::gl::GL_DEPTH_COMPONENT32F,

    // Base internal format: GL_DEPTH_STENCIL
    DepthStencil = (RawGlEnum)::gl::GL_DEPTH_STENCIL,
    Depth24Stencil8 = (RawGlEnum)::gl::GL_DEPTH24_STENCIL8,
    Depth32fStencil8 = (RawGlEnum)::gl::GL_DEPTH32F_STENCIL8,

    // TODO: Compressed base internal formats
};

#undef _GL_MAKE_CHANNEL_ENUMERATORS

enum class VertexAttribPointerType : RawGlEnum
{
    Byte = (RawGlEnum)::gl::GL_BYTE,
    UByte = (RawGlEnum)::gl::GL_UNSIGNED_BYTE,
    Short = (RawGlEnum)::gl::GL_SHORT,
    UShort = (RawGlEnum)::gl::GL_UNSIGNED_SHORT,
    Int = (RawGlEnum)::gl::GL_INT,
    UInt = (RawGlEnum)::gl::GL_UNSIGNED_INT,
    Float = (RawGlEnum)::gl::GL_FLOAT,
    Double = (RawGlEnum)::gl::GL_DOUBLE,
    HalfFloat = (RawGlEnum)::gl::GL_HALF_FLOAT,
    Fixed = (RawGlEnum)::gl::GL_FIXED,
};

enum class PixelType : RawGlEnum
{
    Bitmap = (RawGlEnum)::gl::GL_BITMAP,
    Byte = (RawGlEnum)::gl::GL_BYTE,
    Float = (RawGlEnum)::gl::GL_FLOAT,
    Int = (RawGlEnum)::gl::GL_INT,
    Short = (RawGlEnum)::gl::GL_SHORT,
    UByte = (RawGlEnum)::gl::GL_UNSIGNED_BYTE,
    UByte332 = (RawGlEnum)::gl::GL_UNSIGNED_BYTE_3_3_2,
    UInt = (RawGlEnum)::gl::GL_UNSIGNED_INT,
    UInt1010102 = (RawGlEnum)::gl::GL_UNSIGNED_INT_10_10_10_2,
    UInt8888 = (RawGlEnum)::gl::GL_UNSIGNED_INT_8_8_8_8,
    UShort = (RawGlEnum)::gl::GL_UNSIGNED_SHORT,
    UShort4444 = (RawGlEnum)::gl::GL_UNSIGNED_SHORT_4_4_4_4,
    UShort5551 = (RawGlEnum)::gl::GL_UNSIGNED_SHORT_5_5_5_1,
};

template<typename T>
struct TypeTraits;

template<>
struct TypeTraits<uint8_t>
{
    static const constexpr DrawElementsType DrawElementsType = DrawElementsType::UByte;
    static const constexpr VertexAttribPointerType VertexAttribPointerType = VertexAttribPointerType::UByte;
    static const constexpr PixelType PixelType = PixelType::UByte;

    static const constexpr ::gl::GLsizei ElementCount = 1;

    static const constexpr InternalFormat SrgbaSizedInternalFormat = InternalFormat::Srgb8Alpha8;
    static const constexpr InternalFormat SrgbSizedInternalFormat = InternalFormat::Srgb8;

    static const constexpr InternalFormat RgbaSizedInternalFormat = InternalFormat::Rgba8;
    static const constexpr InternalFormat RgbSizedInternalFormat = InternalFormat::Rgb8;
};

template<>
struct TypeTraits<uint16_t>
{
    static const constexpr DrawElementsType DrawElementsType = DrawElementsType::UShort;
    static const constexpr VertexAttribPointerType VertexAttribPointerType = VertexAttribPointerType::UShort;
    static const constexpr PixelType PixelType = PixelType::UShort;

    static const constexpr ::gl::GLsizei ElementCount = 1;
};

template<>
struct TypeTraits<uint32_t>
{
    static const constexpr DrawElementsType DrawElementsType = DrawElementsType::UInt;
    static const constexpr VertexAttribPointerType VertexAttribPointerType = VertexAttribPointerType::UInt;
    static const constexpr PixelType PixelType = PixelType::UInt;

    static const constexpr ::gl::GLsizei ElementCount = 1;
};

template<>
struct TypeTraits<int8_t>
{
    static const constexpr VertexAttribPointerType VertexAttribPointerType = VertexAttribPointerType::Byte;
    static const constexpr PixelType PixelType = PixelType::Byte;
    static const constexpr ::gl::GLsizei ElementCount = 1;
};

template<>
struct TypeTraits<int16_t>
{
    static const constexpr VertexAttribPointerType VertexAttribPointerType = VertexAttribPointerType::Short;
    static const constexpr PixelType PixelType = PixelType::Short;
    static const constexpr ::gl::GLsizei ElementCount = 1;
};

template<>
struct TypeTraits<int32_t>
{
    static const constexpr VertexAttribPointerType VertexAttribPointerType = VertexAttribPointerType::Int;
    static const constexpr PixelType PixelType = PixelType::Int;
    static const constexpr ::gl::GLsizei ElementCount = 1;
};

template<>
struct TypeTraits<float>
{
    static const constexpr VertexAttribPointerType VertexAttribPointerType = VertexAttribPointerType::Float;
    static const constexpr PixelType PixelType = PixelType::Float;
    static const constexpr ::gl::GLsizei ElementCount = 1;

    static const constexpr InternalFormat RgbSizedInternalFormat = InternalFormat::Rgb32f;
    static const constexpr InternalFormat RSizedInternalFormat = InternalFormat::R32f;
};

template<int N>
struct TypeTraits<glm::vec<N, float, glm::defaultp>>
{
    static const constexpr VertexAttribPointerType VertexAttribPointerType = VertexAttribPointerType::Float;
    static const constexpr PixelType PixelType = PixelType::Float;
    static const constexpr ::gl::GLsizei ElementCount = N;
};

struct Half final
{
private:
#ifdef __clang__
#    pragma clang diagnostic push
#    pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#endif
    const ::gl::GLhalf __placeholder = 0;
#ifdef __clang__
#    pragma clang diagnostic pop
#endif
};

template<>
struct TypeTraits<Half>
{
    static const constexpr VertexAttribPointerType VertexAttribPointerType = VertexAttribPointerType::HalfFloat;
    static const constexpr PixelType PixelType = PixelType::Float;
    static const constexpr ::gl::GLsizei ElementCount = 1;

    static const constexpr InternalFormat RgbSizedInternalFormat = InternalFormat::Rgb16f;
    static const constexpr InternalFormat RSizedInternalFormat = InternalFormat::R16f;
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
