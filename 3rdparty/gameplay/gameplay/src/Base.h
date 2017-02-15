#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>

#include <functional>
#include <vector>

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
    namespace gl
    {
        inline void checkGlError()
        {
#ifndef NDEBUG
            const auto error = glGetError();
            if(error == GL_NO_ERROR)
                return;

            BOOST_LOG_TRIVIAL(error) << "OpenGL error " << error << ": " << gluErrorString(error);
            BOOST_ASSERT_MSG(false, "OpenGL error check failed");
#endif
        }
    }

    class BindableResource
    {
    public:
        void bind() const
        {
            m_binder(m_handle);
            gl::checkGlError();
        }

        void unbind() const
        {
            m_binder(0);
            gl::checkGlError();
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
            gl::checkGlError();
        }

        virtual ~BindableResource()
        {
            m_deleter(1, &m_handle);
            gl::checkGlError();
        }

    private:
        GLuint m_handle = 0;
        std::function<Allocator> m_allocator;
        std::function<Binder> m_binder;
        std::function<Deleter> m_deleter;

        BindableResource(const BindableResource&) = delete;
        BindableResource& operator=(const BindableResource&) = delete;
    };

    class VertexBufferHandle : public BindableResource
    {
    public:
        explicit VertexBufferHandle()
            : BindableResource(glGenBuffers, [](GLuint handle) { glBindBuffer(GL_ARRAY_BUFFER, handle); }, glDeleteBuffers)
        {
        }

        const void* map()
        {
            bind();
            auto data = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
            gl::checkGlError();
            return data;
        }

        void* mapRw()
        {
            bind();
            auto data = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
            gl::checkGlError();
            return data;
        }

        void unmap()
        {
            glUnmapBuffer(GL_ARRAY_BUFFER);
            gl::checkGlError();
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
        explicit TextureHandle(GLenum type)
            : BindableResource(glGenTextures, [type](GLuint handle) { glBindTexture(type, handle); }, glDeleteTextures)
            , m_type{type}
        {
        }

        void set(GLenum param, GLint value)
        {
            glTextureParameteri(getHandle(), param, value);
            gl::checkGlError();
        }

        void set(GLenum param, GLfloat value)
        {
            glTextureParameterf(getHandle(), param, value);
            gl::checkGlError();
        }

        GLint getWidth() const noexcept
        {
            return m_width;
        }

        GLint getHeight() const noexcept
        {
            return m_height;
        }

        void set2D(const std::vector<glm::vec4>& data)
        {
            BOOST_ASSERT(m_width > 0 && m_height > 0);
            BOOST_ASSERT(static_cast<size_t>(m_width) * static_cast<size_t>(m_height) == data.size());

            glTexImage2D(m_type, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, data.empty() ? nullptr : data.data());
            gl::checkGlError();

            if( m_mipmap )
            {
                glGenerateMipmap(m_type);
                gl::checkGlError();
            }
        }

        void set2D(GLint width, GLint height, const std::vector<glm::vec4>& data, bool generateMipmaps)
        {
            BOOST_ASSERT(width > 0 && height > 0);
            BOOST_ASSERT(static_cast<size_t>(width) * static_cast<size_t>(height) == data.size());

            // Create the texture.
            bind();
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            gl::checkGlError();

            // Texture 2D
            glTexImage2D(m_type, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data.empty() ? nullptr : data.data());
            gl::checkGlError();

            m_width = width;
            m_width = height;

            // Set initial minification filter based on whether or not mipmaping was enabled.
            set(GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR);
            gl::checkGlError();

            m_mipmap = generateMipmaps;

            if( m_mipmap )
            {
                glGenerateMipmap(m_type);
                gl::checkGlError();
            }
        }

        void set2DDepth(GLint width, GLint height)
        {
            BOOST_ASSERT(width > 0 && height > 0);

            // Create the texture.
            bind();
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            gl::checkGlError();

            // Texture 2D
            glTexImage2D(m_type, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            gl::checkGlError();

            m_width = width;
            m_height = height;
            m_mipmap = false;

            set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }

    private:
        const GLenum m_type;
        GLint m_width = -1;
        GLint m_height = -1;
        bool m_mipmap = false;
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

    class ShaderHandle
    {
        ShaderHandle(const ShaderHandle&) = delete;
        ShaderHandle& operator=(const ShaderHandle&) = delete;

    public:
        explicit ShaderHandle(GLenum type)
            : m_handle{glCreateShader(type)}
            , m_type{type}
        {
            BOOST_ASSERT(type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER);
            gl::checkGlError();
            BOOST_ASSERT(m_handle != 0);
        }

        ~ShaderHandle()
        {
            glDeleteShader(m_handle);
            gl::checkGlError();
        }


        GLenum getType() const noexcept
        {
            return m_type;
        }


        void setSource(const std::string& src)
        {
            const GLchar* data[1]{ src.c_str() };
            glShaderSource(m_handle, 1, data, nullptr);
            gl::checkGlError();
        }

        void setSource(const GLchar* src[], GLsizei n)
        {
            glShaderSource(m_handle, n, src, nullptr);
            gl::checkGlError();
        }

        void compile()
        {
            glCompileShader(m_handle);
            gl::checkGlError();
        }


        bool getCompileStatus() const
        {
            GLint success = GL_FALSE;
            glGetShaderiv(m_handle, GL_COMPILE_STATUS, &success);
            gl::checkGlError();
            return success == GL_TRUE;
        }


        std::string getInfoLog() const
        {
            GLint length = 0;
            glGetShaderiv(m_handle, GL_INFO_LOG_LENGTH, &length);
            gl::checkGlError();
            if( length == 0 )
            {
                length = 4096;
            }
            if( length > 0 )
            {
                char* infoLog = new char[length];
                glGetShaderInfoLog(m_handle, length, nullptr, infoLog);
                gl::checkGlError();
                infoLog[length - 1] = '\0';
                std::string result = infoLog;
                delete[] infoLog;
                return result;
            }

            return {};
        }

        GLuint getHandle() const noexcept
        {
            return m_handle;
        }

    private:
        const GLuint m_handle;
        const GLenum m_type;
    };


    class ProgramHandle : public BindableResource
    {
    public:
        explicit ProgramHandle()
            : BindableResource{
                  [](GLsizei n, GLuint* handle){ BOOST_ASSERT(n == 1 && handle != nullptr); *handle = glCreateProgram(); },
                  glUseProgram,
                  [](GLsizei n, GLuint* handle){ BOOST_ASSERT(n == 1 && handle != nullptr); glDeleteProgram(*handle); }
            }
        {
        }

        void attach(const ShaderHandle& shader)
        {
            BOOST_ASSERT(shader.getCompileStatus());
            glAttachShader(getHandle(), shader.getHandle());
            gl::checkGlError();
        }

        void link()
        {
            glLinkProgram(getHandle());
            gl::checkGlError();
        }

        bool getLinkStatus() const
        {
            GLint success = GL_FALSE;
            glGetProgramiv(getHandle(), GL_LINK_STATUS, &success);
            gl::checkGlError();
            return success == GL_TRUE;
        }


        std::string getInfoLog() const
        {
            GLint length = 0;
            glGetProgramiv(getHandle(), GL_INFO_LOG_LENGTH, &length);
            gl::checkGlError();
            if( length == 0 )
            {
                length = 4096;
            }
            if( length > 0 )
            {
                char* infoLog = new char[length];
                glGetProgramInfoLog(getHandle(), length, nullptr, infoLog);
                gl::checkGlError();
                infoLog[length - 1] = '\0';
                std::string result = infoLog;
                delete[] infoLog;
                return result;
            }

            return {};
        }


        GLint getActiveAttributeCount() const
        {
            GLint activeAttributes = 0;
            glGetProgramiv(getHandle(), GL_ACTIVE_ATTRIBUTES, &activeAttributes);
            gl::checkGlError();
            return activeAttributes;
        }

        GLint getActiveAttributeMaxLength() const
        {
            GLint length = 0;
            glGetProgramiv(getHandle(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &length);
            gl::checkGlError();
            return length;
        }

        GLint getActiveUniformCount() const
        {
            GLint activeAttributes = 0;
            glGetProgramiv(getHandle(), GL_ACTIVE_UNIFORMS, &activeAttributes);
            gl::checkGlError();
            return activeAttributes;
        }

        GLint getActiveUniformMaxLength() const
        {
            GLint length = 0;
            glGetProgramiv(getHandle(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &length);
            gl::checkGlError();
            return length;
        }

        class ActiveAttribute
        {
        public:
            explicit ActiveAttribute(GLuint program, GLint index, GLint maxLength)
            {
                GLchar* attribName = new GLchar[maxLength + 1];
                glGetActiveAttrib(program, index, maxLength, nullptr, &m_size, &m_type, attribName);
                attribName[maxLength] = '\0';
                m_name = attribName;
                delete[] attribName;
                gl::checkGlError();

                m_location = glGetAttribLocation(program, m_name.c_str());
                gl::checkGlError();
            }

            const std::string& getName() const noexcept
            {
                return m_name;
            }

            GLint getLocation() const noexcept
            {
                return m_location;
            }

        private:
            GLint m_size = 0;
            GLenum m_type = 0;
            std::string m_name{};
            GLint m_location = -1;
        };

        class ActiveUniform
        {
        public:
            explicit ActiveUniform(GLuint program, GLint index, GLint maxLength, GLint& samplerIndex)
            {
                GLchar* uniformName = new GLchar[maxLength + 1];
                glGetActiveUniform(program, index, maxLength, nullptr, &m_size, &m_type, uniformName);
                uniformName[maxLength] = '\0';
                if(auto chr = std::strrchr(uniformName, '['))
                    *chr = '\0';

                m_name = uniformName;
                delete[] uniformName;
                gl::checkGlError();

                m_location = glGetUniformLocation(program, m_name.c_str());
                gl::checkGlError();

                switch(m_type)
                {
                    case GL_SAMPLER_1D:
                    case GL_SAMPLER_1D_SHADOW:
                    case GL_SAMPLER_1D_ARRAY:
                    case GL_SAMPLER_1D_ARRAY_SHADOW:
                    case GL_SAMPLER_2D:
                    case GL_SAMPLER_2D_SHADOW:
                    case GL_SAMPLER_2D_ARRAY:
                    case GL_SAMPLER_2D_ARRAY_SHADOW:
                    case GL_SAMPLER_2D_RECT:
                    case GL_SAMPLER_2D_RECT_SHADOW:
                    case GL_SAMPLER_2D_MULTISAMPLE:
                    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
                    case GL_SAMPLER_3D:
                    case GL_SAMPLER_CUBE:
                    case GL_SAMPLER_CUBE_MAP_ARRAY:
                    case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
                    case GL_SAMPLER_CUBE_SHADOW:
                    case GL_SAMPLER_BUFFER:
                        m_samplerIndex = samplerIndex;
                        samplerIndex += m_size;
                    default:
                        break;
                }

                if(m_type == GL_SAMPLER_1D || m_type == GL_SAMPLER_2D || m_type == GL_SAMPLER_3D || m_type == GL_SAMPLER_CUBE || m_type == GL_SAMPLER_1D_SHADOW || m_type == GL_SAMPLER_2D_SHADOW)
                {
                    m_samplerIndex = samplerIndex;
                    samplerIndex += m_size;
                }
            }

            const std::string& getName() const noexcept
            {
                return m_name;
            }

            void set(GLfloat value)
            {
                glUniform1f(m_location, value);
                gl::checkGlError();
            }


            void set(const GLfloat* values, size_t count)
            {
                BOOST_ASSERT(values != nullptr);
                glUniform1fv(m_location, count, values);
                gl::checkGlError();
            }


            void set(GLint value)
            {
                glUniform1i(m_location, value);
            }


            void set(const GLint* values, size_t count)
            {
                BOOST_ASSERT(values != nullptr);
                glUniform1iv(m_location, count, values);
                gl::checkGlError();
            }


            void set(const glm::mat4& value)
            {
                glUniformMatrix4fv(m_location, 1, GL_FALSE, glm::value_ptr(value));
                gl::checkGlError();
            }


            void set(const glm::mat4* values, size_t count)
            {
                BOOST_ASSERT(values != nullptr);
                glUniformMatrix4fv(m_location, count, GL_FALSE, (GLfloat*)values);
                gl::checkGlError();
            }


            void set(const glm::vec2& value)
            {
                glUniform2f(m_location, value.x, value.y);
                gl::checkGlError();
            }


            void set(const glm::vec2* values, size_t count)
            {
                BOOST_ASSERT(values != nullptr);
                glUniform2fv(m_location, count, (GLfloat*)values);
                gl::checkGlError();
            }


            void set(const glm::vec3& value)
            {
                glUniform3f(m_location, value.x, value.y, value.z);
                gl::checkGlError();
            }


            void set(const glm::vec3* values, size_t count)
            {
                BOOST_ASSERT(values != nullptr);
                glUniform3fv(m_location, count, (GLfloat*)values);
                gl::checkGlError();
            }


            void set(const glm::vec4& value)
            {
                glUniform4f(m_location, value.x, value.y, value.z, value.w);
                gl::checkGlError();
            }


            void set(const glm::vec4* values, size_t count)
            {
                BOOST_ASSERT(values != nullptr);
                glUniform4fv(m_location, count, (GLfloat*)values);
                gl::checkGlError();
            }


            void set(const TextureHandle& texture)
            {
                BOOST_ASSERT(m_samplerIndex >= 0);

                glActiveTexture(GL_TEXTURE0 + m_samplerIndex);
                gl::checkGlError();

                // Bind the sampler - this binds the texture and applies sampler state
                texture.bind();

                glUniform1i(m_location, m_samplerIndex);
                gl::checkGlError();
            }


            void set(const std::vector<std::shared_ptr<TextureHandle>>& values)
            {
                BOOST_ASSERT(m_samplerIndex >= 0);

                // Set samplers as active and load texture unit array
                GLint units[32];
                for( size_t i = 0; i < values.size(); ++i )
                {
                    glActiveTexture(GL_TEXTURE0 + m_samplerIndex + i);
                    gl::checkGlError();

                    // Bind the sampler - this binds the texture and applies sampler state
                    values[i]->bind();

                    units[i] = m_samplerIndex + i;
                }

                // Pass texture unit array to GL
                glUniform1iv(m_location, values.size(), units);
                gl::checkGlError();
            }

            GLenum getType() const noexcept
            {
                return m_type;
            }

        private:
            GLint m_size = 0;
            GLenum m_type = 0;
            std::string m_name{};
            GLint m_location = -1;
            GLint m_samplerIndex = -1;
        };

        ActiveAttribute getActiveAttribute(GLint index) const
        {
            return ActiveAttribute{getHandle(), index, getActiveAttributeMaxLength()};
        }

        std::vector<ActiveAttribute> getActiveAttributes() const
        {
            std::vector<ActiveAttribute> attribs;
            auto count = getActiveAttributeCount();
            const auto maxLength = getActiveAttributeMaxLength();
            for(decltype(count) i=0; i<count; ++i)
                attribs.emplace_back(getHandle(), i, maxLength);
            return attribs;
        }

        ActiveUniform getActiveUniform(GLint index, GLint& samplerIndex) const
        {
            return ActiveUniform{getHandle(), index, getActiveAttributeMaxLength(), samplerIndex};
        }

        std::vector<ActiveUniform> getActiveUniforms() const
        {
            std::vector<ActiveUniform> attribs;
            auto count = getActiveAttributeCount();
            const auto maxLength = getActiveAttributeMaxLength();
            GLint samplerIndex = 0;
            for(decltype(count) i=0; i<count; ++i)
                attribs.emplace_back(getHandle(), i, maxLength, samplerIndex);
            return attribs;
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
#ifdef NDEBUG
#define GL_ASSERT( gl_code ) gl_code
#else
#define GL_ASSERT( gl_code ) \
    { \
        gl_code; \
        __gl_error_code = glGetError(); \
        BOOST_ASSERT(__gl_error_code == GL_NO_ERROR); \
    }
#endif

/** Global variable to hold GL errors
 * @script{ignore} */
extern GLenum __gl_error_code;
