#pragma once

#include "Base.h"

#include "gl/util.h"

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
        explicit Image(GLint width, GLint height, const glm::vec4* data = nullptr);
        ~Image();

        /**
         * Gets the image's raw pixel data.
         *
         * @return The image's pixel data.
         * @script{ignore}
         */
        const std::vector<glm::vec4>& getData() const
        {
            return _data;
        }

        /**
         * Gets the height of the image.
         *
         * @return The height of the image.
         */
        GLint getHeight() const
        {
            return _height;
        }

        /**
         * Gets the width of the image.
         *
         * @return The width of the image.
         */
        GLint getWidth() const
        {
            return _width;
        }


        glm::vec4& at(GLint x, GLint y)
        {
            static glm::vec4 none;
            if( x < 0 || x >= _width || y < 0 || y >= _height )
                return none;

            return _data[y * _width + x];
        }


        const glm::vec4& at(GLint x, GLint y) const
        {
            static const glm::vec4 none;
            if( x < 0 || x >= _width || y < 0 || y >= _height )
                return none;

            return _data[y * _width + x];
        }


        void fill(const glm::vec4& color)
        {
            std::fill_n(_data.data(), _data.size(), color);
        }


        void line(GLint x0, GLint y0, GLint x1, GLint y1, const glm::vec4& color)
        {
            // shamelessly copied from wikipedia
            const GLint dx = abs(x1 - x0);
            const GLint sx = x0 < x1 ? 1 : -1;
            const GLint dy = -abs(y1 - y0);
            const GLint sy = y0 < y1 ? 1 : -1;

            GLint err = dx + dy;

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
        GLint _width;
        GLint _height;
    };
}
