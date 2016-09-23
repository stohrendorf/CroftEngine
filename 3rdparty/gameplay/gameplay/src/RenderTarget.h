#pragma once

#include "Base.h"
#include "Texture.h"


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
        explicit RenderTarget(const std::string& id);

        ~RenderTarget();

        /**
         * Create a RenderTarget and add it to the list of available RenderTargets.
         *
         * The created RenderTarget contains a 32-bit texture with a single/base mipmap level only.
         *
         * @param id The ID of the new RenderTarget.
         * @param width The width of the new RenderTarget.
         * @param height The height of the new RenderTarget.
         *
         * @return A newly created RenderTarget.
         * @script{create}
         */
        static std::shared_ptr<RenderTarget> create(const std::string& id, unsigned int width, unsigned int height);

        /**
         * Create a RenderTarget from the given Texture and add it to the list of
         * available RenderTargets.
         *
         * Note that different hardware and OpenGL versions have different capabilities
         * and restrictions on what texture formats are supported as render targets.
         *
         * @param id The ID of the new RenderTarget.
         * @param texture The texture for the new RenderTarget.
         *
         * @return A newly created RenderTarget.
         * @script{create}
         */
        static std::shared_ptr<RenderTarget> create(const std::string& id, const std::shared_ptr<Texture>& texture);

        /**
         * Get a named RenderTarget from its ID.
         *
         * @param id The ID of the RenderTarget to search for.
         *
         * @return The RenderTarget with the specified ID, or NULL if one was not found.
         */
        static std::shared_ptr<RenderTarget> getRenderTarget(const std::string& id);

        /**
         * Get the ID of this RenderTarget.
         *
         * @return The ID of this RenderTarget.
         */
        const std::string& getId() const;

        /**
         * Get the backing texture of this RenderTarget.
         *
         * @return The backing texture of this RenderTarget.
         */
        const std::shared_ptr<Texture>& getTexture() const;

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

        std::string _id;
        std::shared_ptr<Texture> _texture;
    };
}
