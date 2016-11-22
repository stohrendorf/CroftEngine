#pragma once

#include "Base.h"

#include <vector>

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
        explicit Image(unsigned width, unsigned height, const glm::vec4* data = nullptr);
        ~Image();

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


        glm::vec4& at(unsigned int x, unsigned int y)
        {
            static glm::vec4 none;
            if( x >= _width || y >= _height )
                return none;

            return _data[y * _width + x];
        }


        const glm::vec4& at(unsigned int x, unsigned int y) const
        {
            static const glm::vec4 none;
            if( x >= _width || y >= _height )
                return none;

            return _data[y * _width + x];
        }


        void fill(const glm::vec4& color)
        {
            std::fill(_data.begin(), _data.end(), color);
        }


        void line(int x0, int y0, int x1, int y1, const glm::vec4& color)
        {
            // shamelessly copied from wikipedia
            const int dx = abs(x1 - x0);
            const int sx = x0 < x1 ? 1 : -1;
            const int dy = -abs(y1 - y0);
            const int sy = y0 < y1 ? 1 : -1;

            int err = dx + dy;

            while( true )
            {
                if( color.a != 1 )
                {
                    at(x0, y0) = glm::mix(at(x0, y0), color, color.a);
                }
                else
                {
                    at(x0, y0) = color;
                }

                if( x0 == x1 && y0 == y1 )
                    break;

                auto e2 = 2 * err;
                if( e2 > dy )
                {
                    err += dy;
                    x0 += sx;
                }
                if( e2 < dx )
                {
                    err += dx;
                    y0 += sy;
                }
            }
        }


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
