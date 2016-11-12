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

    class BindableResource
    {
    public:
        void bind() const
        {
            m_binder(m_handle);
            BOOST_ASSERT(glGetError() == GL_NO_ERROR);

            postBind();
        }

        void unbind() const
        {
            m_binder(0);
            BOOST_ASSERT(glGetError() == GL_NO_ERROR);
        }

        GLuint getHandle() const
        {
            BOOST_ASSERT(m_handle != 0);
            return m_handle;
        }

    protected:
        using Allocator = void(GLsizei, GLuint*);
        using Binder = void(GLuint);
        using Deleter = void(GLsizei, GLuint*);

        explicit BindableResource(const std::function<Allocator>& allocator, const std::function<Binder>& binder, const std::function<Deleter>& deleter)
            : m_allocator{ allocator }
            , m_binder{ binder }
            , m_deleter{ deleter }
        {
            BOOST_ASSERT(allocator != nullptr);
            BOOST_ASSERT(binder != nullptr);
            BOOST_ASSERT(deleter != nullptr);

            m_allocator(1, &m_handle);
            BOOST_ASSERT(glGetError() == GL_NO_ERROR);
        }

        virtual ~BindableResource()
        {
            m_deleter(1, &m_handle);
            BOOST_ASSERT(glGetError() == GL_NO_ERROR);
        }

    private:
        GLuint m_handle = 0;
        std::function<Allocator> m_allocator;
        std::function<Binder> m_binder;
        std::function<Deleter> m_deleter;

        BindableResource(const BindableResource&) = delete;
        BindableResource& operator=(const BindableResource&) = delete;

        virtual void postBind() const
        {
        }
    };

    class VertexBufferHandle : public BindableResource
    {
    public:
        explicit VertexBufferHandle()
            : BindableResource(glGenBuffers, [](GLuint handle) { glBindBuffer(GL_ARRAY_BUFFER, handle); }, glDeleteBuffers)
        {
        }
    };

    class VertexArrayHandle : public BindableResource
    {
    public:
        explicit VertexArrayHandle()
            : BindableResource(glGenVertexArrays, glBindVertexArray, glDeleteVertexArrays)
        {
        }
    };

    class IndexBufferHandle : public BindableResource
    {
    public:
        explicit IndexBufferHandle()
            : BindableResource(glGenBuffers, [](GLuint handle) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle); }, glDeleteBuffers)
        {
        }
    };

    class TextureHandle : public BindableResource
    {
    public:
        explicit TextureHandle()
            : BindableResource(glGenTextures, [](GLuint handle) { glBindTexture(GL_TEXTURE_2D, handle); }, glDeleteTextures)
        {
        }
    };

    class FrameBufferHandle : public BindableResource
    {
    public:
        explicit FrameBufferHandle()
            : BindableResource(glGenFramebuffers, [](GLuint handle) { glBindFramebuffer(GL_FRAMEBUFFER, handle); }, glDeleteFramebuffers)
        {
        }

        static const std::shared_ptr<FrameBufferHandle>& getDefault()
        {
            static std::shared_ptr<FrameBufferHandle> instance;
            if(instance == nullptr)
            {
                GLint fbo;
                glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
                instance.reset(new FrameBufferHandle(static_cast<GLuint>(fbo)));
            }

            return instance;
        }

    private:
        explicit FrameBufferHandle(GLuint handle)
            : BindableResource([handle](GLsizei, GLuint* ptr) { *ptr = handle; }, [](GLuint handle) { glBindFramebuffer(GL_FRAMEBUFFER, handle); }, [](GLsizei, GLuint*) {})
        {
        }
    };

    class RenderBufferHandle : public BindableResource
    {
    public:
        explicit RenderBufferHandle()
            : BindableResource(glGenRenderbuffers, [](GLuint handle) { glBindRenderbuffer(GL_RENDERBUFFER, handle); }, glDeleteRenderbuffers)
        {
        }
    };
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
