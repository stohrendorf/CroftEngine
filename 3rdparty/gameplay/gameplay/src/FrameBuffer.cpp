#include "Base.h"
#include "FrameBuffer.h"

#include <boost/log/trivial.hpp>


namespace gameplay
{
    size_t FrameBuffer::_maxRenderTargets = 0;
    std::shared_ptr<FrameBuffer> FrameBuffer::_defaultFrameBuffer = nullptr;
    std::shared_ptr<FrameBuffer> FrameBuffer::_currentFrameBuffer = nullptr;


    FrameBuffer::FrameBuffer(FrameBufferHandle handle)
        : _handle(handle)
        , _renderTargets()
        , _renderTargetCount(0)
        , _depthStencilTarget(nullptr)
    {
    }


    FrameBuffer::~FrameBuffer()
    {
        // Release GL resource.
        if( _handle )
        GL_ASSERT( glDeleteFramebuffers(1, &_handle) );
    }


    void FrameBuffer::initialize()
    {
        // Query the current/initial FBO handle and store is as out 'default' frame buffer.
        // On many platforms this will simply be the zero (0) handle, but this is not always the case.
        GLint fbo;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
        _defaultFrameBuffer = std::make_shared<FrameBuffer>(static_cast<FrameBufferHandle>(fbo));
        _currentFrameBuffer = _defaultFrameBuffer;

        // Query the max supported color attachments. This glGet operation is not supported
        // on GL ES 2.x, so if the define does not exist, assume a value of 1.
        GLint val;
        GL_ASSERT( glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &val) );
        _maxRenderTargets = static_cast<size_t>(std::max(1, val));
    }


    void FrameBuffer::finalize()
    {
        _defaultFrameBuffer.reset();
    }


    FrameBuffer::FrameBuffer()
        : FrameBuffer(0, 0)
    {
    }


    FrameBuffer::FrameBuffer(unsigned int width, unsigned int height)
        : FrameBuffer(0)
    {
        std::shared_ptr<RenderTarget> renderTarget = nullptr;
        if( width > 0 && height > 0 )
        {
            // Create a default RenderTarget with same ID.
            renderTarget = std::make_shared<RenderTarget>(width, height);
            if( renderTarget == nullptr )
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to create render target for frame buffer.";
                BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create render target for frame buffer"));
            }
        }

        // Create the frame buffer
        FrameBufferHandle handle = 0;
        GL_ASSERT( glGenFramebuffers(1, &handle) );

        // Create the render target array for the new frame buffer
        _renderTargets.resize(_maxRenderTargets);

        if( renderTarget )
        {
            setRenderTarget(renderTarget, 0);
        }
    }


    unsigned int FrameBuffer::getWidth() const
    {
        if( _renderTargetCount > 0 && !_renderTargets.empty() && _renderTargets[0] != nullptr )
            return _renderTargets[0]->getWidth();

        return 0;
    }


    unsigned int FrameBuffer::getHeight() const
    {
        if( _renderTargetCount > 0 && !_renderTargets.empty() && _renderTargets[0] != nullptr )
            return _renderTargets[0]->getHeight();

        return 0;
    }


    size_t FrameBuffer::getMaxRenderTargets()
    {
        return _maxRenderTargets;
    }


    void FrameBuffer::setRenderTarget(const std::shared_ptr<RenderTarget>& target, size_t index)
    {
        BOOST_ASSERT(!target || target->getTexture() != nullptr);

        // No change
        if( _renderTargets[index] == target )
            return;

        setRenderTarget(target, index, GL_TEXTURE_2D);
    }


    void FrameBuffer::setRenderTarget(const std::shared_ptr<RenderTarget>& target, Texture::CubeFace face, size_t index)
    {
        BOOST_ASSERT(face >= Texture::POSITIVE_X && face <= Texture::NEGATIVE_Z);
        BOOST_ASSERT(!target || target->getTexture() != nullptr);

        setRenderTarget(target, index, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face);
    }


    void FrameBuffer::setRenderTarget(const std::shared_ptr<RenderTarget>& target, size_t index, GLenum textureTarget)
    {
        BOOST_ASSERT(index < _maxRenderTargets);
        BOOST_ASSERT(!_renderTargets.empty());

        // Release our reference to the current RenderTarget at this index.
        if( _renderTargets[index] )
        {
            --_renderTargetCount;
        }

        _renderTargets[index] = target;

        if( target )
        {
            ++_renderTargetCount;

            // Now set this target as the color attachment corresponding to index.
            GL_ASSERT( glBindFramebuffer(GL_FRAMEBUFFER, _handle) );
            GLenum attachment = GL_COLOR_ATTACHMENT0 + index;
            GL_ASSERT( glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textureTarget, _renderTargets[index]->getTexture()->getHandle(), 0) );
            GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if( fboStatus != GL_FRAMEBUFFER_COMPLETE )
            {
                BOOST_LOG_TRIVIAL(error) << "Framebuffer status incomplete: 0x" << std::hex << fboStatus;
            }

            // Restore the FBO binding
            GL_ASSERT( glBindFramebuffer(GL_FRAMEBUFFER, _currentFrameBuffer->_handle) );
        }
    }


    std::shared_ptr<RenderTarget> FrameBuffer::getRenderTarget(size_t index) const
    {
        BOOST_ASSERT(!_renderTargets.empty());
        if( index < _maxRenderTargets )
        {
            return _renderTargets[index];
        }
        return nullptr;
    }


    size_t FrameBuffer::getRenderTargetCount() const
    {
        return _renderTargetCount;
    }


    void FrameBuffer::setDepthStencilTarget(const std::shared_ptr<DepthStencilTarget>& target)
    {
        if( _depthStencilTarget == target )
            return;

        _depthStencilTarget = target;

        if( target )
        {
            // Now set this target as the color attachment corresponding to index.
            GL_ASSERT( glBindFramebuffer(GL_FRAMEBUFFER, _handle) );

            // Attach the render buffer to the framebuffer
            GL_ASSERT( glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthStencilTarget->_depthBuffer) );
            if( target->isPacked() )
            {
                GL_ASSERT( glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthStencilTarget->_depthBuffer) );
            }
            else if( target->getFormat() == DepthStencilTarget::DEPTH_STENCIL )
            {
                GL_ASSERT( glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthStencilTarget->_stencilBuffer) );
            }

            // Check the framebuffer is good to go.
            GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if( fboStatus != GL_FRAMEBUFFER_COMPLETE )
            {
                BOOST_LOG_TRIVIAL(error) << "Framebuffer status incomplete: 0x" << std::hex << fboStatus;
            }

            // Restore the FBO binding
            GL_ASSERT( glBindFramebuffer(GL_FRAMEBUFFER, _currentFrameBuffer->_handle) );
        }
    }


    const std::shared_ptr<DepthStencilTarget>& FrameBuffer::getDepthStencilTarget() const
    {
        return _depthStencilTarget;
    }


    bool FrameBuffer::isDefault() const
    {
        return (this == _defaultFrameBuffer.get());
    }


    std::shared_ptr<FrameBuffer> FrameBuffer::bind()
    {
        GL_ASSERT( glBindFramebuffer(GL_FRAMEBUFFER, _handle) );
        auto previousFrameBuffer = _currentFrameBuffer;
        _currentFrameBuffer = shared_from_this();
        return previousFrameBuffer;
    }


    void FrameBuffer::getScreenshot(const std::shared_ptr<Image>& image)
    {
        BOOST_ASSERT( image );

        unsigned int width = _currentFrameBuffer->getWidth();
        unsigned int height = _currentFrameBuffer->getHeight();

        if( image->getWidth() == width && image->getHeight() == height )
        {
            GL_ASSERT( glReadPixels(0, 0, width, height, GL_RGBA32F, GL_FLOAT, const_cast<float*>(&image->getData()[0].x)) );
        }
    }


    std::shared_ptr<Image> FrameBuffer::createScreenshot()
    {
        auto screenshot = std::make_shared<Image>(_currentFrameBuffer->getWidth(), _currentFrameBuffer->getHeight(), nullptr);
        getScreenshot(screenshot);

        return screenshot;
    }


    const std::shared_ptr<FrameBuffer>& FrameBuffer::bindDefault()
    {
        GL_ASSERT( glBindFramebuffer(GL_FRAMEBUFFER, _defaultFrameBuffer->_handle) );
        _currentFrameBuffer = _defaultFrameBuffer;
        return _defaultFrameBuffer;
    }


    const std::shared_ptr<FrameBuffer>& FrameBuffer::getCurrent()
    {
        return _currentFrameBuffer;
    }
}
