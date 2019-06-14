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
    virtual void attach(const FrameBuffer& frameBuffer, const ::gl::GLenum attachment) const = 0;

public:
    explicit FrameBufferAttachment() = default;

    virtual ~FrameBufferAttachment() = default;
};


class FrameBufferTextureAttachment final : public FrameBufferAttachment
{
public:
    explicit FrameBufferTextureAttachment(const std::shared_ptr<Texture>& texture, const ::gl::GLint level = 0)
            : m_texture{texture}
            , m_level{level}
    {}

    void attach(const FrameBuffer& frameBuffer, const ::gl::GLenum attachment) const override;

private:
    const std::shared_ptr<Texture> m_texture;
    const ::gl::GLint m_level;
};


class FrameBuffer : public BindableResource
{
public:
    using Attachment = std::pair<std::shared_ptr<FrameBufferAttachment>, ::gl::GLenum>;
    using Attachments = std::vector<Attachment>;

private:
    const ::gl::GLenum m_type;
    Attachments m_attachments;

public:
    explicit FrameBuffer(Attachments attachments, const std::string& label = {},
                         ::gl::GLenum type = ::gl::GL_FRAMEBUFFER)
            : BindableResource{::gl::glGenFramebuffers,
                               [type](const ::gl::GLuint handle) { ::gl::glBindFramebuffer( type, handle ); },
                               ::gl::glDeleteFramebuffers,
                               type,
                               label}
            , m_type{type}
            , m_attachments{std::move( attachments )}
    {
        bind();
        std::vector<::gl::GLenum> colorAttachments;
        for( const auto& attachment : m_attachments )
        {
            attachment.first->attach( *this, attachment.second );

            if( attachment.second >= ::gl::GL_COLOR_ATTACHMENT0 && attachment.second <= ::gl::GL_COLOR_ATTACHMENT31 )
                colorAttachments.emplace_back( attachment.second );
        }
        if( !colorAttachments.empty() )
            GL_ASSERT(
                    glDrawBuffers( gsl::narrow<::gl::GLsizei>( colorAttachments.size() ), colorAttachments.data() ) );
        else
            GL_ASSERT( glDrawBuffer( ::gl::GL_NONE ) );

        Expects( isComplete() );
        unbind();
    }

    ::gl::GLenum getType() const
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

        const auto result = GL_ASSERT_FN( glCheckFramebufferStatus( m_type ) );

#ifndef NDEBUG
        switch( result )
        {
            case ::gl::GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                             << " incomplete: incomplete attachment";
                break;
            case ::gl::GL_FRAMEBUFFER_UNDEFINED:
                BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                             << " incomplete: default is undefined";
                break;
            case ::gl::GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                             << " incomplete: missing attachment";
                break;
            case ::gl::GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                             << " incomplete: incomplete draw buffer";
                break;
            case ::gl::GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                             << " incomplete: incomplete read buffer";
                break;
            case ::gl::GL_FRAMEBUFFER_UNSUPPORTED:
                BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                             << " incomplete: unsupported attachment type";
                break;
            case ::gl::GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                             << " incomplete: incomplete multisample";
                break;
            case ::gl::GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                             << " incomplete: incomplete layer targets";
                break;
            case ::gl::GL_FRAMEBUFFER_COMPLETE:
                break;
            default:
                BOOST_LOG_TRIVIAL( error ) << "Framebuffer #" << getHandle() << " incomplete: unknown code #"
                                           << result;
        }
#endif

        return result == ::gl::GL_FRAMEBUFFER_COMPLETE;
    }

    static void unbindAll(const ::gl::GLenum type = ::gl::GL_FRAMEBUFFER)
    {
        GL_ASSERT( glBindFramebuffer( type, 0 ) );
    }
};


void FrameBufferTextureAttachment::attach(const FrameBuffer& frameBuffer, const ::gl::GLenum attachment) const
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

    FrameBufferBuilder& texture(::gl::GLenum attachment, const std::shared_ptr<Texture>& texture, ::gl::GLint level = 0)
    {
        m_attachments.emplace_back( std::make_shared<FrameBufferTextureAttachment>( texture ), attachment );
        return *this;
    }
};

}
}
