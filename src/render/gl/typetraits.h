#pragma once

#include "glassert.h"

#include <glm/glm.hpp>
#include <glm/gtc/packing.hpp>

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

    static const constexpr GLint SrgbaInternalFormat = GL_SRGB8_ALPHA8;
    static const constexpr GLint SrgbInternalFormat = GL_SRGB8;

    static const constexpr GLint RgbaInternalFormat = GL_RGBA8;
    static const constexpr GLint RgbInternalFormat = GL_RGB8;
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
struct TypeTraits<GLfloat>
{
    static const constexpr GLenum TypeId = GL_FLOAT;

    static const constexpr GLsizei ElementCount = 1;

    static const constexpr GLint RgbInternalFormat = GL_RGB32F;
    static const constexpr GLint RInternalFormat = GL_R32F;
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

struct Half final
{
private:
#ifdef __clang__
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#endif
    const uint16_t __placeholder = 0;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
};

template<>
struct TypeTraits<Half>
{
    static const constexpr GLenum TypeId = GL_FLOAT;

    static const constexpr GLsizei ElementCount = 1;

    static const constexpr GLint RgbInternalFormat = GL_RGB16F;
    static const constexpr GLint RInternalFormat = GL_R16F;
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
