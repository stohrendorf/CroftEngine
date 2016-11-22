#pragma once

#include "Base.h"

#include <memory>

namespace gameplay
{
    class FrameBuffer;
    class Texture;

    /**
     * Defines a container for depth and stencil targets in a frame buffer object.
     */
    class DepthStencilTarget
    {
        friend class FrameBuffer;

    public:
        /**
         * Defines the accepted formats for DepthStencilTargets.
         */
        enum Format
        {
            /**
             * A target with depth data.
             */
            DEPTH,

            /**
             * A target with depth data and stencil data.
             */
            DEPTH_STENCIL
        };


        /**
        * Create a DepthStencilTarget and add it to the list of available DepthStencilTargets.
        *
        * @param format The format of the new DepthStencilTarget.
        * @param width Width of the new DepthStencilTarget.
        * @param height Height of the new DepthStencilTarget.
        *
        * @return A newly created DepthStencilTarget.
        * @script{create}
        */
        DepthStencilTarget(Format format, unsigned int width, unsigned int height);
        ~DepthStencilTarget();

        /**
         * Returns the format of the DepthStencilTarget.
         *
         * @return The format.
         */
        Format getFormat() const;

        /**
         * Returns the width of the DepthStencilTarget.
         *
         * @return The width.
         */
        unsigned int getWidth() const;

        /**
         * Returns the height of the DepthStencilTarget.
         *
         * @return The height.
         */
        unsigned int getHeight() const;

        const std::shared_ptr<Texture>& getDepthTexture() const
        {
            return _depthTexture;
        }

    private:
        DepthStencilTarget& operator=(const DepthStencilTarget&) = delete;

        Format _format;
        RenderBufferHandle _depthBuffer{};
        std::shared_ptr<Texture> _depthTexture;
        unsigned int _width;
        unsigned int _height;
    };
}
