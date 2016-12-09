#include "Base.h"

#include "DepthStencilTarget.h"
#include "Texture.h"

#ifndef GL_DEPTH24_STENCIL8_OES
#define GL_DEPTH24_STENCIL8_OES 0x88F0
#endif
#ifndef GL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24_OES
#endif

namespace gameplay
{
    DepthStencilTarget::DepthStencilTarget(unsigned int width, unsigned int height)
        : _depthTexture{std::make_shared<Texture>(width, height)}
        , _width(width)
        , _height(height)
    {
        // Create a render buffer for this new depth+stencil target
        _depthBuffer.bind();

        GL_ASSERT( glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height) );
    }


    DepthStencilTarget::~DepthStencilTarget() = default;


    unsigned int DepthStencilTarget::getWidth() const
    {
        return _width;
    }


    unsigned int DepthStencilTarget::getHeight() const
    {
        return _height;
    }
}
