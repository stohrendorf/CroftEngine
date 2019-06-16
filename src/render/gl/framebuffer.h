#pragma once

#include "renderbuffer.h"
#include "texture.h"

#include <initializer_list>

namespace render
{
namespace gl
{
enum class FramebufferAttachment : RawGlEnum
{
    Color0 = (RawGlEnum)::gl::GL_COLOR_ATTACHMENT0,
    Color1 = (RawGlEnum)::gl::GL_COLOR_ATTACHMENT1,
    Color2 = (RawGlEnum)::gl::GL_COLOR_ATTACHMENT2,
    Color3 = (RawGlEnum)::gl::GL_COLOR_ATTACHMENT3,
    Color4 = (RawGlEnum)::gl::GL_COLOR_ATTACHMENT4,
    Color5 = (RawGlEnum)::gl::GL_COLOR_ATTACHMENT5,
    Color6 = (RawGlEnum)::gl::GL_COLOR_ATTACHMENT6,
    Color7 = (RawGlEnum)::gl::GL_COLOR_ATTACHMENT7,
    ColorMax = Color7,
    Depth = (RawGlEnum)::gl::GL_DEPTH_ATTACHMENT,
    DepthStencil = (RawGlEnum)::gl::GL_DEPTH_STENCIL_ATTACHMENT,
};

enum class FramebufferTarget : RawGlEnum
{
    Framebuffer = (RawGlEnum)::gl::GL_FRAMEBUFFER,
    DrawFramebuffer = (RawGlEnum)::gl::GL_DRAW_FRAMEBUFFER,
    ReadFramebuffer = (RawGlEnum)::gl::GL_READ_FRAMEBUFFER,
};

enum class FramebufferStatus : RawGlEnum
{
    Complete = (RawGlEnum)::gl::GL_FRAMEBUFFER_COMPLETE,
    Undefined = (RawGlEnum)::gl::GL_FRAMEBUFFER_UNDEFINED,
    IncompleteAttachment = (RawGlEnum)::gl::GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
    MissingAttachment = (RawGlEnum)::gl::GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
    IncompleteDrawBuffer = (RawGlEnum)::gl::GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
    IncompleteReadBuffer = (RawGlEnum)::gl::GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
    Unsupported = (RawGlEnum)::gl::GL_FRAMEBUFFER_UNSUPPORTED,
    IncompleteMultisample = (RawGlEnum)::gl::GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
    IncompleteLayerTargets = (RawGlEnum)::gl::GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS,
};

class Framebuffer;

class Attachment
{
    friend Framebuffer;
private:
    virtual void attach(const Framebuffer& frameBuffer, const FramebufferAttachment attachment) const = 0;

public:
    explicit Attachment() = default;

    virtual ~Attachment() = default;
};

class TextureAttachment final : public Attachment
{
public:
    explicit TextureAttachment(const std::shared_ptr<Texture>& texture, const ::gl::GLint level = 0)
      : m_texture{ texture }
      , m_level{ level }
    {}

    void attach(const Framebuffer& frameBuffer, const FramebufferAttachment attachment) const override;

private:
    const std::shared_ptr<Texture> m_texture;
    const ::gl::GLint m_level;
};

class Framebuffer : public BindableResource
{
public:
    using Attachment = std::pair<std::shared_ptr<Attachment>, FramebufferAttachment>;
    using Attachments = std::vector<Attachment>;

private:
    Attachments m_attachments;

public:
    explicit Framebuffer(Attachments attachments, const std::string& label = {})
      : BindableResource{ ::gl::glGenFramebuffers,
                          [](const ::gl::GLuint handle) {
                              ::gl::glBindFramebuffer( (::gl::GLenum)FramebufferTarget::Framebuffer, handle );
                          },
                          ::gl::glDeleteFramebuffers,
                          ObjectIdentifier::Framebuffer,
                          label }
      , m_attachments{ std::move( attachments ) }
    {
        bind();
        std::vector<::gl::GLenum> colorAttachments;
        for( const auto& attachment : m_attachments )
        {
            attachment.first->attach( *this, attachment.second );

            if( attachment.second >= FramebufferAttachment::Color0
              && attachment.second <= FramebufferAttachment::ColorMax )
                colorAttachments.emplace_back( (::gl::GLenum)attachment.second );
        }
        if( !colorAttachments.empty() )
            GL_ASSERT(
              glDrawBuffers( gsl::narrow<::gl::GLsizei>( colorAttachments.size() ), colorAttachments.data() ) );
        else
            GL_ASSERT( glDrawBuffer( ::gl::GL_NONE ) );

        Expects( isComplete() );
        unbind();
    }

    const Attachments& getAttachments() const
    {
        return m_attachments;
    }

    bool isComplete() const
    {
        bind();

        const auto result =
          (FramebufferStatus)GL_ASSERT_FN( glCheckFramebufferStatus( (::gl::GLenum)FramebufferTarget::Framebuffer ) );

#ifndef NDEBUG
        switch( result )
        {
        case FramebufferStatus::IncompleteAttachment:
            BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                         << " incomplete: incomplete attachment";
            break;
        case FramebufferStatus::Undefined:
            BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                         << " incomplete: default is undefined";
            break;
        case FramebufferStatus::MissingAttachment:
            BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                         << " incomplete: missing attachment";
            break;
        case FramebufferStatus::IncompleteDrawBuffer:
            BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                         << " incomplete: incomplete draw buffer";
            break;
        case FramebufferStatus::IncompleteReadBuffer:
            BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                         << " incomplete: incomplete read buffer";
            break;
        case FramebufferStatus::Unsupported:
            BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                         << " incomplete: unsupported attachment type";
            break;
        case FramebufferStatus::IncompleteMultisample:
            BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                         << " incomplete: incomplete multisample";
            break;
        case FramebufferStatus::IncompleteLayerTargets:
            BOOST_LOG_TRIVIAL( warning ) << "Framebuffer #" << getHandle()
                                         << " incomplete: incomplete layer targets";
            break;
        case FramebufferStatus::Complete:
            break;
        default:
            BOOST_LOG_TRIVIAL( error ) << "Framebuffer #" << getHandle() << " incomplete: unknown code #"
                                       << (::gl::GLenum)result;
        }
#endif

        return result == FramebufferStatus::Complete;
    }

    static void unbindAll()
    {
        GL_ASSERT( glBindFramebuffer( (::gl::GLenum)FramebufferTarget::Framebuffer, 0 ) );
    }
};

void TextureAttachment::attach(const Framebuffer& frameBuffer, const FramebufferAttachment attachment) const
{
    GL_ASSERT( glFramebufferTexture( (::gl::GLenum)FramebufferTarget::Framebuffer,
                                     (::gl::GLenum)attachment,
                                     m_texture->getHandle(),
                                     m_level ) );
}

class FrameBufferBuilder
{
private:
    Framebuffer::Attachments m_attachments;

public:
    std::shared_ptr<Framebuffer> build()
    {
        return std::make_shared<Framebuffer>( std::move( m_attachments ) );
    }

    FrameBufferBuilder& texture(FramebufferAttachment attachment,
                                const std::shared_ptr<Texture>& texture,
                                ::gl::GLint level = 0)
    {
        m_attachments.emplace_back( std::make_shared<TextureAttachment>( texture ), attachment );
        return *this;
    }
};
}
}
