#pragma once

#include "renderbuffer.h"
#include "texture.h"

namespace gameplay
{
    namespace gl
    {
        class FrameBuffer : public BindableResource
        {
        public:
            explicit FrameBuffer(const std::string& label = {})
                    : BindableResource{glGenFramebuffers,
                                       [](GLuint handle) { glBindFramebuffer( GL_FRAMEBUFFER, handle ); },
                                       glDeleteFramebuffers,
                                       GL_FRAMEBUFFER,
                                       label}
            {
            }

            // ReSharper disable once CppMemberFunctionMayBeConst
            void attachTexture1D(GLenum attachment, const Texture& texture, GLint level = 0)
            {
                bind();
                glFramebufferTexture1D( GL_FRAMEBUFFER, attachment, texture.getType(), texture.getHandle(), level );
                checkGlError();
            }

            // ReSharper disable once CppMemberFunctionMayBeConst
            void attachTexture2D(GLenum attachment, const Texture& texture, GLint level = 0)
            {
                bind();
                glFramebufferTexture2D( GL_FRAMEBUFFER, attachment, texture.getType(), texture.getHandle(), level );
                checkGlError();
            }

            // ReSharper disable once CppMemberFunctionMayBeConst
            void attachTextureLayer(GLenum attachment, const Texture& texture, GLint level = 0, GLint layer = 0)
            {
                bind();
                glFramebufferTextureLayer( GL_FRAMEBUFFER, attachment, texture.getHandle(), level, layer );
                checkGlError();
            }

            // ReSharper disable once CppMemberFunctionMayBeConst
            void attachRenderbuffer(GLenum attachment, const RenderBuffer& renderBuffer)
            {
                bind();
                glFramebufferRenderbuffer( GL_DRAW_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderBuffer.getHandle() );
                checkGlError();
            }

            bool isComplete() const
            {
                bind();

                auto result = glCheckFramebufferStatus( GL_FRAMEBUFFER );
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

            static void unbindAll()
            {
                glBindFramebuffer( GL_FRAMEBUFFER, 0 );
                checkGlError();
            }
        };
    }
}
