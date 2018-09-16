#pragma once

#include "renderbuffer.h"
#include "texture.h"

namespace gameplay
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
                               [type](GLuint handle) { glBindFramebuffer( type, handle ); },
                               glDeleteFramebuffers,
                               type,
                               label}
            , m_type{type}
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void attachTexture1D(GLenum attachment, const Texture& texture, GLint level = 0)
    {
        bind();
        glFramebufferTexture1D( m_type, attachment, texture.getType(), texture.getHandle(), level );
        checkGlError();
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void attachTexture2D(GLenum attachment, const Texture& texture, GLint level = 0)
    {
        bind();
        glFramebufferTexture2D( m_type, attachment, texture.getType(), texture.getHandle(), level );
        checkGlError();
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void attachTextureLayer(GLenum attachment, const Texture& texture, GLint level = 0, GLint layer = 0)
    {
        bind();
        glFramebufferTextureLayer( m_type, attachment, texture.getHandle(), level, layer );
        checkGlError();
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void attachRenderbuffer(GLenum attachment, const RenderBuffer& renderBuffer)
    {
        bind();
        glFramebufferRenderbuffer( GL_DRAW_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderBuffer.getHandle() );
        checkGlError();
    }

    void blit(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
              GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
              GLbitfield mask = GL_COLOR_BUFFER_BIT,
              GLenum filter = GL_LINEAR)
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

        auto result = glCheckFramebufferStatus( m_type );
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

    static void unbindAll(GLenum type = GL_FRAMEBUFFER)
    {
        glBindFramebuffer( type, 0 );
        checkGlError();
    }
};
}
}
