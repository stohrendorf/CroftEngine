#pragma once

#include "Base.h"
#include "RenderTarget.h"
#include "Image.h"

#include "gl/framebuffer.h"

namespace gameplay
{
    /**
     * Defines a frame buffer object that may contain one or more render targets and optionally
     * a depth-stencil target.
     *
     * Frame buffers can be created and used for off-screen rendering, which is useful for
     * techniques such as shadow mapping and post-processing. Render targets within a frame
     * buffer can be both written to and read (by calling RenderTarget::getTexture).
     *
     * When binding a custom frame buffer, you should always store the return value of
     * FrameBuffer::bind and restore it when you are finished drawing to your frame buffer.
     *
     * To bind the default frame buffer, call FrameBuffer::bindDefault.
     */
    class FrameBuffer : public std::enable_shared_from_this<FrameBuffer>
    {
        friend class Game;

    public:
        /**
         * Creates a new, empty FrameBuffer object.
         *
         * The new FrameBuffer does not have any render targets or a depth/stencil target and these
         * must be added before it can be used. The FrameBuffer is added to the list of available
         * FrameBuffers.
         */
        explicit FrameBuffer();

        /**
         * Creates a new FrameBuffer with a single RenderTarget of the specified width and height,
         * and adds the FrameBuffer to the list of available FrameBuffers.
         *
         * If width and height are non-zero a default RenderTarget of type RGBA will be created
         * and added to the FrameBuffer, with the same ID. The ID of the render target can be
         * changed later via RenderTarget::setId(const char*).
         *
         * You can additionally add a DepthStencilTarget using FrameBuffer::setDepthTexture.
         *
         * @param width The width of the RenderTarget to be created and attached.
         * @param height The height of the RenderTarget to be created and attached.
         */
        FrameBuffer(unsigned int width, unsigned int height);

        ~FrameBuffer();

        /**
         * Gets the width of the frame buffer.
         *
         * @return The width of the frame buffer.
         */
        unsigned int getWidth() const;

        /**
         * Gets the height of the frame buffer.
         *
         * @return The height of the frame buffer.
         */
        unsigned int getHeight() const;

        /**
         * Get the number of color attachments available on the current hardware.
         *
         * @return The number of color attachments available on the current hardware.
         */
        static size_t getMaxRenderTargets();

        /**
         * Set a RenderTarget on this FrameBuffer's color attachment at the specified index.
         *
         * @param target The 2D RenderTarget to set.
         * @param index The index of the color attachment to set.
         */
        void setRenderTarget(const std::shared_ptr<RenderTarget>& target, size_t index = 0);

        /**
         * Get the RenderTarget attached to the FrameBuffer's color attachment at the specified index.
         *
         * @param index The index of the color attachment to retrieve a RenderTarget from.
         *
         * @return The RenderTarget attached at the specified index.
         */
        std::shared_ptr<RenderTarget> getRenderTarget(size_t index = 0) const;

        /**
         * Returns the current number of render targets attached to this frame buffer.
         *
         * @return The number of render targets attached.
         */
        size_t getRenderTargetCount() const;

        void setDepthTexture(const std::shared_ptr<gl::Texture>& target);

        const std::shared_ptr<gl::Texture>& getDepthTexture() const
        {
            return _depthTexture;
        }

        /**
         * Binds this FrameBuffer for off-screen rendering and return you the currently bound one.
         *
         * You should keep the return FrameBuffer and store it and call bind() when you rendering is complete.
         *
         * @ return The currently bound framebuffer.
         */
        void bind();

        /**
         * Records a screenshot of what is stored on the current FrameBuffer to an Image.
         *
         * The Image must be the same size as the FrameBuffer, otherwise the operation will fail.
         */
        static std::shared_ptr<Image> getScreenshot();

        /**
         * Binds the default FrameBuffer for rendering to the display.
         *
         * @ return The default framebuffer.
         */
        static void bindDefault();

        /**
         * Gets the currently bound FrameBuffer.
         *
         * @return The currently bound FrameBuffer.
         */
        static const std::shared_ptr<FrameBuffer>& getCurrent();

    private:

        FrameBuffer& operator=(const FrameBuffer&) = delete;

        void setRenderTarget(const std::shared_ptr<RenderTarget>& target, size_t index, GLenum textureTarget);

        static void initialize();

        gl::FrameBuffer _handle;
        std::vector<std::shared_ptr<RenderTarget>> _renderTargets;
        size_t _renderTargetCount;
        std::shared_ptr<gl::Texture> _depthTexture;

        static size_t _maxRenderTargets;
        static std::shared_ptr<FrameBuffer> _currentFrameBuffer;
    };
}
