#pragma once

#include "rendertarget.h"
#include "gl/pixel.h"

#include <glm/glm.hpp>

#include <vector>


namespace gameplay
{
    namespace gl
    {
        class Texture : public RenderTarget
        {
        public:
            explicit Texture(GLenum type)
                : RenderTarget(glGenTextures, [type](GLuint handle) { glBindTexture(type, handle); }, glDeleteTextures)
                , m_type(type)
            {
            }


            // ReSharper disable once CppMemberFunctionMayBeConst
            void set(GLenum param, GLint value)
            {
                glTextureParameteri(getHandle(), param, value);
                checkGlError();
            }


            // ReSharper disable once CppMemberFunctionMayBeConst
            void set(GLenum param, GLfloat value)
            {
                glTextureParameterf(getHandle(), param, value);
                checkGlError();
            }


            GLint getWidth() const noexcept override
            {
                return m_width;
            }


            GLint getHeight() const noexcept override
            {
                return m_height;
            }


            GLenum getType() const noexcept
            {
                return m_type;
            }


            // ReSharper disable once CppMemberFunctionMayBeConst
            template<typename T>
            void set2D(const std::vector<gl::RGBA<T>>& data)
            {
                BOOST_ASSERT(m_width > 0 && m_height > 0);
                BOOST_ASSERT(data.empty() || static_cast<size_t>(m_width) * static_cast<size_t>(m_height) == data.size());

                bind();

                glTexImage2D(m_type, 0, gl::RGBA<T>::InternalFormat, m_width, m_height, 0, gl::RGBA<T>::Format, gl::RGBA<T>::TypeId, data.empty() ? nullptr : data.data());
                checkGlError();

                if( m_mipmap )
                {
                    glGenerateMipmap(m_type);
                    checkGlError();
                }
            }


            // ReSharper disable once CppMemberFunctionMayBeConst
            template<typename T>
            void update2D(const std::vector<gl::RGBA<T>>& data)
            {
                BOOST_ASSERT(m_width > 0 && m_height > 0);
                BOOST_ASSERT(static_cast<size_t>(m_width) * static_cast<size_t>(m_height) == data.size());

                bind();

                glTexSubImage2D(m_type, 0, 0, 0, m_width, m_height, gl::RGBA<T>::Format, gl::RGBA<T>::TypeId, data.data());
                checkGlError();

                if( m_mipmap )
                {
                    glGenerateMipmap(m_type);
                    checkGlError();
                }
            }


            template<typename T>
            void set2D(GLint width, GLint height, const std::vector<gl::RGBA<T>>& data, bool generateMipmaps, GLint multisample = 0)
            {
                BOOST_ASSERT(width > 0 && height > 0);
                BOOST_ASSERT(data.empty() || static_cast<size_t>(width) * static_cast<size_t>(height) == data.size());

                bind();

                if( multisample > 0 )
                    glTexImage2DMultisample(m_type, multisample, gl::RGBA<T>::InternalFormat, width, height, GL_TRUE);
                else
                    glTexImage2D(m_type, 0, gl::RGBA<T>::InternalFormat, width, height, 0, gl::RGBA<T>::Format, gl::RGBA<T>::TypeId, data.empty() ? nullptr : data.data());
                checkGlError();

                m_width = width;
                m_height = height;

                // Set initial minification filter based on whether or not mipmaping was enabled.
                set(GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR);
                checkGlError();

                m_mipmap = generateMipmaps;

                if( m_mipmap )
                {
                    glGenerateMipmap(m_type);
                    checkGlError();
                }
            }


            template<typename PixelType>
            void reserve2D(GLint width, GLint height, bool generateMipmaps, GLint multisample = 0)
            {
                BOOST_ASSERT(width > 0 && height > 0);

                // Create the texture.
                bind();

                // Texture 2D
                if( multisample > 0 )
                    glTexImage2DMultisample(m_type, multisample, PixelType::InternalFormat, width, height, GL_TRUE);
                else
                    glTexImage2D(m_type, 0, PixelType::InternalFormat, width, height, 0, PixelType::Format, PixelType::TypeId, nullptr);
                checkGlError();

                m_width = width;
                m_height = height;

                // Set initial minification filter based on whether or not mipmaping was enabled.
                set(GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR);
                checkGlError();

                m_mipmap = generateMipmaps;

                if( m_mipmap )
                {
                    glGenerateMipmap(m_type);
                    checkGlError();
                }
            }


            void set2DDepth(GLint width, GLint height, GLint multisample = 0)
            {
                BOOST_ASSERT(width > 0 && height > 0);

                // Create the texture.
                bind();

                // Texture 2D
                if( multisample > 0 )
                glTexImage2DMultisample(m_type, multisample, GL_DEPTH_COMPONENT, width, height, GL_TRUE);
                else
                    glTexImage2D(m_type, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
                checkGlError();

                m_width = width;
                m_height = height;

                set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                checkGlError();

                m_mipmap = false;
            }


        private:
            const GLenum m_type;
            GLint m_width = -1;
            GLint m_height = -1;
            bool m_mipmap = false;
        };
    }
}
