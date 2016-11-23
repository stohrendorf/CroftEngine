#pragma once

#include <functional>

#include <glm/glm.hpp>

#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>

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
            checkGlError();
        }

        void unbind() const
        {
            m_binder(0);
            checkGlError();
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
            checkGlError();
        }

        virtual ~BindableResource()
        {
            m_deleter(1, &m_handle);
            checkGlError();
        }

    private:
        GLuint m_handle = 0;
        std::function<Allocator> m_allocator;
        std::function<Binder> m_binder;
        std::function<Deleter> m_deleter;

        BindableResource(const BindableResource&) = delete;
        BindableResource& operator=(const BindableResource&) = delete;

        static void checkGlError()
        {
#ifndef NDEBUG
            const auto error = glGetError();
            if(error == GL_NO_ERROR)
                return;

            BOOST_LOG_TRIVIAL(error) << "OpenGL error " << error << ": " << gluErrorString(error);
            BOOST_ASSERT_MSG(false, "OpenGL error check failed");
#endif
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

        static void unbindAll()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
