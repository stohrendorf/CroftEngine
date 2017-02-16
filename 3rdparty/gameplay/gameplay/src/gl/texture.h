#pragma once

#include "rendertarget.h"

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
    void set2D(const std::vector<glm::vec4>& data)
    {
        BOOST_ASSERT(m_width > 0 && m_height > 0);
        BOOST_ASSERT(data.empty() || static_cast<size_t>(m_width) * static_cast<size_t>(m_height) == data.size());

        glTexImage2D(m_type, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, data.empty() ? nullptr : data.data());
        checkGlError();

        if( m_mipmap )
        {
            glGenerateMipmap(m_type);
            checkGlError();
        }
    }


    void set2D(GLint width, GLint height, const std::vector<glm::vec4>& data, bool generateMipmaps, GLint multisample = 0)
    {
        BOOST_ASSERT(width > 0 && height > 0);
        BOOST_ASSERT(data.empty() || static_cast<size_t>(width) * static_cast<size_t>(height) == data.size());

        // Create the texture.
        bind();

        // Texture 2D
        if(multisample == 0)
            // (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
            glTexImage2D(m_type, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data.empty() ? nullptr : data.data());
        else
            // (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
            glTexImage2DMultisample(m_type, multisample, GL_RGBA32F, width, height, GL_TRUE);
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


    void set2D(GLint width, GLint height, GLenum internalFormat, bool generateMipmaps, GLint multisample = 0)
    {
        BOOST_ASSERT(width > 0 && height > 0);

        // Create the texture.
        bind();

        // Texture 2D
        glTexImage2DMultisample(m_type, multisample, internalFormat, width, height, GL_TRUE);
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


    void set2DDepth(GLint width, GLint height)
    {
        BOOST_ASSERT(width > 0 && height > 0);

        // Create the texture.
        bind();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        checkGlError();

        // Texture 2D
        glTexImage2D(m_type, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        checkGlError();

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
}
}
