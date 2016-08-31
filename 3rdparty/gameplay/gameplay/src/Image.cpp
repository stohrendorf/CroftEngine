#include "Base.h"
#include "Image.h"


namespace gameplay
{
    Image* Image::create(unsigned int width, unsigned int height, Image::Format format, const unsigned char* data)
    {
        GP_ASSERT(width > 0 && height > 0);
        GP_ASSERT(format >= RGB && format <= RGBA);

        unsigned int pixelSize = 0;
        switch( format )
        {
            case Image::RGB:
                pixelSize = 3;
                break;
            case Image::RGBA:
                pixelSize = 4;
                break;
        }

        Image* image = new Image();

        unsigned int dataSize = width * height * pixelSize;

        image->_width = width;
        image->_height = height;
        image->_format = format;
        image->_data = new unsigned char[dataSize];
        if( data )
            memcpy(image->_data, data, dataSize);

        return image;
    }


    Image::Image() : _data(nullptr)
                   , _format(RGB)
                   , _width(0)
                   , _height(0)
    {
    }


    Image::~Image()
    {
        SAFE_DELETE_ARRAY(_data);
    }
}
