#pragma once

#include "glassert.h"

#include <glm/glm.hpp>

#include <cstdint>

namespace render
{
namespace gl
{
template<typename T>
struct TypeTraits;

template<>
struct TypeTraits<uint8_t>
{
    static const constexpr GLenum TypeId = GL_UNSIGNED_BYTE;

    static const constexpr GLsizei ElementCount = 1;

    static const constexpr GLint RgbaInternalFormat = GL_SRGB_ALPHA;
    static const constexpr GLint RgbInternalFormat = GL_SRGB;

    static const constexpr GLint RgbaFormat = GL_RGBA;
    static const constexpr GLint RgbFormat = GL_RGB;
};

template<>
struct TypeTraits<uint16_t>
{
    static const constexpr GLenum TypeId = GL_UNSIGNED_SHORT;

    static const constexpr GLsizei ElementCount = 1;
};

template<>
struct TypeTraits<uint32_t>
{
    static const constexpr GLenum TypeId = GL_UNSIGNED_INT;

    static const constexpr GLsizei ElementCount = 1;
};

template<>
struct TypeTraits<int8_t>
{
    static const constexpr GLenum TypeId = GL_BYTE;

    static const constexpr GLsizei ElementCount = 1;
};

template<>
struct TypeTraits<int16_t>
{
    static const constexpr GLenum TypeId = GL_SHORT;

    static const constexpr GLsizei ElementCount = 1;
};

template<>
struct TypeTraits<int32_t>
{
    static const constexpr GLenum TypeId = GL_INT;

    static const constexpr GLsizei ElementCount = 1;
};

template<>
struct TypeTraits<float>
{
    static const constexpr GLenum TypeId = GL_FLOAT;

    static const constexpr GLsizei ElementCount = 1;
};

template<>
struct TypeTraits<glm::vec2>
{
    static const constexpr GLenum TypeId = GL_FLOAT;

    static const constexpr GLsizei ElementCount = 2;
};

template<>
struct TypeTraits<glm::vec3>
{
    static const constexpr GLenum TypeId = GL_FLOAT;

    static const constexpr GLsizei ElementCount = 3;
};

template<>
struct TypeTraits<glm::vec4>
{
    static const constexpr GLenum TypeId = GL_FLOAT;

    static const constexpr GLsizei ElementCount = 4;
};

template<typename T>
struct TypeTraits<const T> : TypeTraits<T>
{
};

template<typename T>
struct TypeTraits<T&> : TypeTraits<T>
{
};
}
}
