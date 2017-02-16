#pragma once

#include "Base.h"

#include "gl/texture.h"

namespace gameplay
{
    /**
     * Defines a linear area of display memory and usually resides
     * in the display memory of the graphics device.
     */
    class RenderTarget : public std::enable_shared_from_this<RenderTarget>
    {
        friend class FrameBuffer;

    public:
        explicit RenderTarget(unsigned int width, unsigned int height);
        explicit RenderTarget(const std::shared_ptr<gl::Texture>& texture);

        ~RenderTarget();

        /**
         * Get the backing texture of this RenderTarget.
         *
         * @return The backing texture of this RenderTarget.
         */
        const std::shared_ptr<gl::Texture>& getTexture() const
        {
            return _texture;
        }

        /**
         * Returns the width of the RenderTarget.
         *
         * @return The width.
         */
        unsigned int getWidth() const;

        /**
         * Returns the height of the RenderTarget.
         *
         * @return The height.
         */
        unsigned int getHeight() const;

    private:

        RenderTarget& operator=(const RenderTarget&) = delete;

        std::shared_ptr<gl::Texture> _texture;
    };
}
