#include "Base.h"
#include "Image.h"


namespace gameplay
{
    Image::Image(GLint width, GLint height, const glm::vec4* data)
        : _data()
        , _width(width)
        , _height(height)
    {
        BOOST_ASSERT(width > 0 && height > 0);

        if( data == nullptr )
            _data.resize(width * height);
        else
            _data.assign(data, data + width * height);
    }


    Image::~Image() = default;
}
