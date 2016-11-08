#include "Base.h"
#include "DepthStencilTarget.h"

#ifndef GL_DEPTH24_STENCIL8_OES
#define GL_DEPTH24_STENCIL8_OES 0x88F0
#endif
#ifndef GL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24_OES
#endif

namespace gameplay
{
    DepthStencilTarget::DepthStencilTarget(const char* id, Format format, unsigned int width, unsigned int height)
        : _format(format)
        , _depthBuffer(0)
        , _stencilBuffer(0)
        , _width(width)
        , _height(height)
        , _packed(false)
    {
        // Create a render buffer for this new depth+stencil target
        GL_ASSERT(glGenRenderbuffers(1, &_depthBuffer));
        GL_ASSERT(glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer));

        // First try to add storage for the most common standard GL_DEPTH24_STENCIL8
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

        // Fall back to less common GLES2 extension combination for seperate depth24 + stencil8 or depth16 + stencil8
        __gl_error_code = glGetError();
        if( __gl_error_code != GL_NO_ERROR )
        {
            const char* extString = (const char*)glGetString(GL_EXTENSIONS);

            if( strstr(extString, "GL_OES_packed_depth_stencil") != nullptr )
            {
                GL_ASSERT(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, width, height));
                _packed = true;
            }
            else
            {
                if( strstr(extString, "GL_OES_depth24") != nullptr )
                {
                    GL_ASSERT(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height));
                }
                else
                {
                    GL_ASSERT(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height));
                }
                if( format == DepthStencilTarget::DEPTH_STENCIL )
                {
                    GL_ASSERT(glGenRenderbuffers(1, &_stencilBuffer));
                    GL_ASSERT(glBindRenderbuffer(GL_RENDERBUFFER, _stencilBuffer));
                    GL_ASSERT(glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height));
                }
            }
        }
        else
        {
            // Packed format GL_DEPTH24_STENCIL8 is used mark format as packed.
            _packed = true;
        }
    }


    DepthStencilTarget::~DepthStencilTarget()
    {
        // Destroy GL resources.
        if( _depthBuffer )
        GL_ASSERT( glDeleteRenderbuffers(1, &_depthBuffer) );
        if( _stencilBuffer )
        GL_ASSERT( glDeleteRenderbuffers(1, &_stencilBuffer) );
    }


    DepthStencilTarget::Format DepthStencilTarget::getFormat() const
    {
        return _format;
    }


    unsigned int DepthStencilTarget::getWidth() const
    {
        return _width;
    }


    unsigned int DepthStencilTarget::getHeight() const
    {
        return _height;
    }


    bool DepthStencilTarget::isPacked() const
    {
        return _packed;
    }
}
