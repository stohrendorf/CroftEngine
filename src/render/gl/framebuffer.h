#pragma once

#include "renderbuffer.h"
#include "texture.h"

namespace render
{
namespace gl
{
class FrameBuffer : public BindableResource
{
private:
    const GLenum m_type;

public:
    explicit FrameBuffer(const std::string& label = {}, GLenum type = GL_FRAMEBUFFER)
            : BindableResource{glGenFramebuffers,
                               [type](const GLuint handle) { glBindFramebuffer( type, handle ); },
                               glDeleteFramebuffers,
                               type,
                               label}
            , m_type{type}
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void attachTexture1D(const GLenum attachment, const Texture& texture, const GLint level = 0)
    {
        bind();
        glFramebufferTexture1D( m_type, attachment, texture.getType(), texture.getHandle(), level );
        checkGlError();
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void attachTexture2D(const GLenum attachment, const Texture& texture, const GLint level = 0)
    {
        bind();
        glFramebufferTexture2D( m_type, attachment, texture.getType(), texture.getHandle(), level );
        checkGlError();
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void attachTextureLayer(const GLenum attachment, const Texture& texture,
                            const GLint level = 0, const GLint layer = 0)
    {
        bind();
        glFramebufferTextureLayer( m_type, attachment, texture.getHandle(), level, layer );
        checkGlError();
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void attachRenderbuffer(const GLenum attachment, const RenderBuffer& renderBuffer)
    {
        bind();
        glFramebufferRenderbuffer( GL_DRAW_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderBuffer.getHandle() );
        checkGlError();
    }

    void blit(const GLint srcX0, const GLint srcY0, const GLint srcX1, const GLint srcY1,
              const GLint dstX0, const GLint dstY0, const GLint dstX1, const GLint dstY1,
              const GLbitfield mask = GL_COLOR_BUFFER_BIT,
              const GLenum filter = GL_LINEAR) const
    {
        bind();
        glBlitFramebuffer( srcX0, srcY0, srcX1, srcY1,
                           dstX0, dstY0, dstX1, dstY1,
                           mask, filter );
        checkGlError();
    }

    bool isComplete() const
    {
        bind();

        const auto result = glCheckFramebufferStatus( m_type );
        checkGlError();

#ifndef NDEBUG
        switch( result )
        {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                             << " incomplete: incomplete attachment";
                break;
            case GL_FRAMEBUFFER_UNDEFINED:
                BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                             << " incomplete: default is undefined";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                             << " incomplete: missing attachment";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                             << " incomplete: incomplete draw buffer";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                             << " incomplete: incomplete read buffer";
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                             << " incomplete: unsupported attachment type";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                             << " incomplete: incomplete multisample";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                             << " incomplete: incomplete layer targets";
                break;
            case GL_FRAMEBUFFER_COMPLETE:
                break;
            default:
                BOOST_LOG_TRIVIAL( error ) << "Framebuffer #" << getHandle() << " incomplete: unknown code #"
                                           << result;
        }
#endif

        return result == GL_FRAMEBUFFER_COMPLETE;
    }

    static void unbindAll(const GLenum type = GL_FRAMEBUFFER)
    {
        glBindFramebuffer( type, 0 );
        checkGlError();
    }
};
}
}
