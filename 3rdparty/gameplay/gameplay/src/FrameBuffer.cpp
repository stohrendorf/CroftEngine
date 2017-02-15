#include "Base.h"
#include "FrameBuffer.h"

#include <boost/log/trivial.hpp>


namespace gameplay
{
    size_t FrameBuffer::_maxRenderTargets = 0;
    std::shared_ptr<FrameBuffer> FrameBuffer::_currentFrameBuffer = nullptr;


    FrameBuffer::FrameBuffer()
        : _renderTargets()
        , _renderTargetCount(0)
        , _depthTexture(nullptr)
    {
    }


    FrameBuffer::~FrameBuffer() = default;


    void FrameBuffer::initialize()
    {
        _currentFrameBuffer = nullptr;

        // Query the max supported color attachments. This glGet operation is not supported
        // on GL ES 2.x, so if the define does not exist, assume a value of 1.
        GLint val;
        GL_ASSERT( glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &val) );
        _maxRenderTargets = static_cast<size_t>(std::max(1, val));
    }


    FrameBuffer::FrameBuffer(unsigned int width, unsigned int height)
        : FrameBuffer()
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
            _handle.bind();
            GLenum attachment = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + index);
            GL_ASSERT( glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textureTarget, _renderTargets[index]->getTexture()->getHandle(), 0) );
            auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if( fboStatus != GL_FRAMEBUFFER_COMPLETE )
            {
                BOOST_LOG_TRIVIAL(error) << "Framebuffer status incomplete (color attachment): 0x" << std::hex << fboStatus;
            }

            // Restore the FBO binding
            if(_currentFrameBuffer != nullptr)
                _currentFrameBuffer->bind();
            else
                FrameBufferHandle::unbindAll();
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


    void FrameBuffer::setDepthTexture(const std::shared_ptr<TextureHandle>& target)
    {
        if( _depthTexture == target )
            return;

        _depthTexture = target;

        if( target )
        {
            _handle.bind();

            GL_ASSERT( glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthTexture->getHandle(), 0) );

            // Check the framebuffer is good to go.
            GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if( fboStatus != GL_FRAMEBUFFER_COMPLETE )
            {
                BOOST_LOG_TRIVIAL(error) << "Framebuffer status incomplete (depth attachment): 0x" << std::hex << fboStatus;
                BOOST_THROW_EXCEPTION(std::runtime_error("Framebuffer status incomplete (depth attachment)"));
            }

            // Restore the FBO binding
            if(_currentFrameBuffer != nullptr)
                _currentFrameBuffer->bind();
            else
                FrameBufferHandle::unbindAll();
        }
    }





    void FrameBuffer::bind()
    {
        _handle.bind();
        _currentFrameBuffer = shared_from_this();
    }


    std::shared_ptr<Image> FrameBuffer::getScreenshot()
    {
        const auto width = _currentFrameBuffer->getWidth();
        const auto height = _currentFrameBuffer->getHeight();

        auto image = std::make_shared<Image>(width, height);

        if( image->getWidth() == width && image->getHeight() == height )
        {
            GL_ASSERT( glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, const_cast<float*>(&image->getData()[0].x)) );
        }

        return image;
    }


    void FrameBuffer::bindDefault()
    {
        FrameBufferHandle::unbindAll();
        _currentFrameBuffer.reset();
    }


    const std::shared_ptr<FrameBuffer>& FrameBuffer::getCurrent()
    {
        return _currentFrameBuffer;
    }
}
