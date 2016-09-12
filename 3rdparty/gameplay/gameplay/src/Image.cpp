#include "Base.h"
#include "Image.h"
#include "Vector4.h"


namespace gameplay
{
    std::shared_ptr<Image> Image::create(unsigned int width, unsigned int height, const Vector4* data)
    {
        GP_ASSERT(width > 0 && height > 0);

        auto image = std::make_shared<Image>();

        image->_width = width;
        image->_height = height;
        image->_data.assign(data, data + width * height);

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
