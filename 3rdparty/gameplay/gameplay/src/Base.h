#pragma once

#include <new>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cwchar>
#include <cwctype>
#include <cctype>
#include <cmath>
#include <cstdarg>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <list>
#include <set>
#include <stack>
#include <map>
#include <queue>
#include <algorithm>
#include <limits>
#include <functional>
#include <bitset>
#include <typeinfo>
#include <thread>
#include <mutex>
#include <chrono>
#include <boost/assert.hpp>
#include <boost/current_function.hpp>

#if defined(WIN32) && defined(_MSC_VER)
#define DEBUG_BREAK() __debugbreak()
#else
#define DEBUG_BREAK()
#endif

// Object deletion macro
#define SAFE_DELETE(x) \
    { \
        delete x; \
        x = nullptr; \
    }

// Array deletion macro
#define SAFE_DELETE_ARRAY(x) \
    { \
        delete[] x; \
        x = nullptr; \
    }

#define WINDOW_VSYNC        true

#define WIN32_LEAN_AND_MEAN
#define GLEW_STATIC
#include <GL/glew.h>

// Graphics (GLSL)
#define VERTEX_ATTRIBUTE_POSITION_NAME              "a_position"
#define VERTEX_ATTRIBUTE_NORMAL_NAME                "a_normal"
#define VERTEX_ATTRIBUTE_COLOR_NAME                 "a_color"
#define VERTEX_ATTRIBUTE_TANGENT_NAME               "a_tangent"
#define VERTEX_ATTRIBUTE_BINORMAL_NAME              "a_binormal"
#define VERTEX_ATTRIBUTE_BLENDWEIGHTS_NAME          "a_blendWeights"
#define VERTEX_ATTRIBUTE_BLENDINDICES_NAME          "a_blendIndices"
#define VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME       "a_texCoord"


// Hardware buffer
namespace gameplay
{
    /** Vertex attribute. */
    typedef GLint VertexAttribute;
    /** Vertex buffer handle. */
    typedef GLuint VertexBufferHandle;
    /** Index buffer handle. */
    typedef GLuint IndexBufferHandle;
    /** Texture handle. */
    typedef GLuint TextureHandle;
    /** Frame buffer handle. */
    typedef GLuint FrameBufferHandle;
    /** Render buffer handle. */
    typedef GLuint RenderBufferHandle;
}


/**
 * GL assertion that can be used for any OpenGL function call.
 *
 * This macro will assert if an error is detected when executing
 * the specified GL code. This macro will do nothing in release
 * mode and is therefore safe to use for realtime/per-frame GL
 * function calls.
 */
#ifndef NDEBUG
#define GL_ASSERT( gl_code ) gl_code
#else
#define GL_ASSERT( gl_code ) \
    { \
        gl_code; \
        __gl_error_code = glGetError(); \
        GP_ASSERT(__gl_error_code == GL_NO_ERROR); \
    }
#endif

/** Global variable to hold GL errors
 * @script{ignore} */
extern GLenum __gl_error_code;
