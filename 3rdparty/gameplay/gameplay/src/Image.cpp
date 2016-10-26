#include "Base.h"
#include "Image.h"


namespace gameplay
{
    std::shared_ptr<Image> Image::create(unsigned int width, unsigned int height, const glm::vec4* data)
    {
        BOOST_ASSERT(width > 0 && height > 0);

        auto image = std::make_shared<Image>();

        image->_width = width;
        image->_height = height;
        image->_data.assign(data, data + width * height);

        return image;
    }


    std::shared_ptr<Image> Image::createRGB(unsigned int width, unsigned int height, const char* data)
    {
        BOOST_ASSERT(width > 0 && height > 0);

        auto image = std::make_shared<Image>();

        image->_width = width;
        image->_height = height;
        image->_data.resize(width * height);

        auto d = data;
        for(size_t i = 0; i < width*height; ++i, d += 3)
            image->_data[i] = glm::vec4{ d[0] / 255.0f, d[1] / 255.0f, d[2] / 255.0f, 1 };

        return image;
    }


    Image::Image()
        : _data()
        , _width(0)
        , _height(0)
    {
    }


    Image::~Image() = default;
}
