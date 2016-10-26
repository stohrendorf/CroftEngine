#pragma once

#include <glm/glm.hpp>

namespace gameplay
{
    /**
     * Defines an image buffer of RGB or RGBA color data.
     *
     * Currently only supports loading from .png image files.
     */
    class Image
    {
    public:
        explicit Image();
        ~Image();


        /**
         * Creates an image from the data provided
         *
         * @param width The width of the image data.
         * @param height The height of the image data.
         * @param data The image data. If NULL, the data will be allocated.
         * @return The newly created image.
         * @script{create}
         */
        static std::shared_ptr<Image> create(unsigned width, unsigned height, const glm::vec4* data = nullptr);
        static std::shared_ptr<Image> createRGB(unsigned width, unsigned height, const char* data);
        static std::shared_ptr<Image> createRGBA(unsigned width, unsigned height, const char* data);

        /**
         * Gets the image's raw pixel data.
         *
         * @return The image's pixel data.
         * @script{ignore}
         */
        inline const std::vector<glm::vec4>& getData() const;

        /**
         * Gets the height of the image.
         *
         * @return The height of the image.
         */
        inline unsigned int getHeight() const;

        /**
         * Gets the width of the image.
         *
         * @return The width of the image.
         */
        inline unsigned int getWidth() const;

    private:

        Image& operator=(const Image&) = delete;

        std::vector<glm::vec4> _data;
        unsigned int _width;
        unsigned int _height;
    };


    inline const std::vector<glm::vec4>& Image::getData() const
    {
        return _data;
    }


    inline unsigned int Image::getHeight() const
    {
        return _height;
    }


    inline unsigned int Image::getWidth() const
    {
        return _width;
    }
}
