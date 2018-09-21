#include "cimgwrapper.h"

#ifdef _X
#undef _X
#endif

#include <CImg.h>

#include <boost/assert.hpp>
#include <gsl/gsl>

namespace util
{
CImgWrapper::CImgWrapper(const std::string& filename)
        : m_image{std::make_unique<cimg_library::CImg<uint8_t>>( filename.c_str() )}
{
    if( m_image->spectrum() == 3 )
    {
        m_image->channels( 0, 3 );
        BOOST_ASSERT( m_image->spectrum() == 4 );
        m_image->get_shared_channel( 3 ).fill( 255 );
    }

    Expects( m_image->spectrum() == 4 );
}

CImgWrapper::CImgWrapper(const CImgWrapper& other)
        : m_image{std::make_unique<cimg_library::CImg<uint8_t>>( *other.m_image )}
        , m_interleaved{other.m_interleaved}
{
}

CImgWrapper::CImgWrapper(const uint8_t* data, int width, int height, bool shared)
        : m_image{std::make_unique<cimg_library::CImg<uint8_t>>( data, 4, width, height, 1, shared )}
        , m_interleaved{true}
{

}

CImgWrapper& CImgWrapper::operator=(const CImgWrapper& other)
{
    m_image = std::make_unique<cimg_library::CImg<uint8_t>>( *other.m_image );
    m_interleaved = other.m_interleaved;
    return *this;
}

CImgWrapper::~CImgWrapper() = default;

void CImgWrapper::interleave()
{
    if( m_interleaved )
        return;

    unshare();

    m_interleaved = true;
    m_image->permute_axes( "cxyz" );
}

void CImgWrapper::deinterleave()
{
    if( !m_interleaved )
        return;

    unshare();

    m_interleaved = false;
    m_image->permute_axes( "yzcx" );
}

int CImgWrapper::width() const
{
    return m_interleaved ? m_image->height() : m_image->width();
}

int CImgWrapper::height() const
{
    return m_interleaved ? m_image->depth() : m_image->height();
}

void CImgWrapper::resize(int width, int height)
{
    unshare();
    if( !m_interleaved )
        m_image->resize( width, height, 1, 4, 6 );
    else
        m_image->resize( 4, width, height, 1, 6 );
}

void CImgWrapper::crop(int x0, int y0, int x1, int y1)
{
    unshare();
    if( !m_interleaved )
        m_image->crop( x0, y0, 0, 0,
                       x1, y1, 0, 3
        );
    else
        m_image->crop( 0, x0, y0, 0,
                       3, x1, y1, 0
        );
}

uint8_t& CImgWrapper::operator()(int x, int y, int c)
{
    BOOST_ASSERT( x >= 0 && x < width() );
    BOOST_ASSERT( y >= 0 && y < height() );
    if( !m_interleaved )
        return (*m_image)( x, y, 0, c );
    else
        return (*m_image)( c, x, y, 0 );
}

gameplay::gl::RGBA8& CImgWrapper::operator()(int x, int y)
{
    BOOST_ASSERT( x >= 0 && x < width() );
    BOOST_ASSERT( y >= 0 && y < height() );
    interleave();
    return reinterpret_cast<gameplay::gl::RGBA8&>( (*m_image)( 0, x, y, 0 ) );
}

uint8_t CImgWrapper::operator()(int x, int y, int c) const
{
    if( !m_interleaved )
        return (*m_image)( x, y, 0, c );
    else
        return (*m_image)( c, x, y, 0 );
}

const uint8_t* CImgWrapper::data() const
{
    return m_image->data();
}

void CImgWrapper::savePng(const std::string& filename)
{
    deinterleave();
    m_image->save_png( filename.c_str(), 1 );
}

void CImgWrapper::unshare()
{
    if(m_image->is_shared())
        m_image = std::make_unique<cimg_library::CImg<uint8_t>>( *m_image, false );
}
}
