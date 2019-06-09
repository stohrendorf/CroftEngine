#pragma once

#include "renderbuffer.h"
#include "texture.h"

#include <initializer_list>

namespace render
{
namespace gl
{
class FrameBuffer;


class FrameBufferAttachment
{
    friend FrameBuffer;
private:
    virtual void attach(const FrameBuffer& frameBuffer, const GLenum attachment) const = 0;

public:
    explicit FrameBufferAttachment() = default;

    virtual ~FrameBufferAttachment() = default;
};


class FrameBufferTextureAttachment final : public FrameBufferAttachment
{
public:
    explicit FrameBufferTextureAttachment(const std::shared_ptr<Texture>& texture, const GLint level = 0)
            : m_texture{texture}
            , m_level{level}
    {}

    void attach(const FrameBuffer& frameBuffer, const GLenum attachment) const override;

private:
    const std::shared_ptr<Texture> m_texture;
    const GLint m_level;
};


class FrameBuffer : public BindableResource
{
public:
    using Attachment = std::pair<std::shared_ptr<FrameBufferAttachment>, GLenum>;
    using Attachments = std::vector<Attachment>;

private:
    const GLenum m_type;
    Attachments m_attachments;

public:
    explicit FrameBuffer(Attachments attachments, const std::string& label = {}, GLenum type = GL_FRAMEBUFFER)
            : BindableResource{glGenFramebuffers,
                               [type](const GLuint handle) { glBindFramebuffer( type, handle ); },
                               glDeleteFramebuffers,
                               type,
                               label}
            , m_type{type}
            , m_attachments{std::move( attachments )}
    {
        bind();
        std::vector<GLenum> tmp;
        for( const auto& attachment : m_attachments )
        {
            attachment.first->attach( *this, attachment.second );

            if( attachment.second >= GL_COLOR_ATTACHMENT0 && attachment.second <= GL_COLOR_ATTACHMENT15 )
                tmp.emplace_back( attachment.second );
        }
        if( !tmp.empty() )
            GL_ASSERT( glDrawBuffers( gsl::narrow<GLsizei>( tmp.size() ), tmp.data() ) );

        Expects( isComplete() );
        unbind();
    }

    GLenum getType() const
    {
        return m_type;
    }

    const Attachments& getAttachments() const
    {
        return m_attachments;
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
        GL_ASSERT( glBindFramebuffer( type, 0 ) );
    }
};


void FrameBufferTextureAttachment::attach(const FrameBuffer& frameBuffer, const GLenum attachment) const
{
    GL_ASSERT( glFramebufferTexture( frameBuffer.getType(), attachment, m_texture->getHandle(), m_level ) );
}


class FrameBufferBuilder
{
private:
    FrameBuffer::Attachments m_attachments;

public:
    std::shared_ptr<FrameBuffer> build()
    {
        return std::make_shared<FrameBuffer>( std::move( m_attachments ) );
    }

    FrameBufferBuilder& texture(GLenum attachment, const std::shared_ptr<Texture>& texture, GLint level = 0)
    {
        m_attachments.emplace_back( std::make_shared<FrameBufferTextureAttachment>( texture ), attachment );
        return *this;
    }
};

}
}
