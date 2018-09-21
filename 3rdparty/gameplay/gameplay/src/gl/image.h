#pragma once

#include "util.h"
#include "pixel.h"
#include "../gsl_util.h"

#include <gsl/gsl>

#include <vector>

namespace gameplay
{
namespace gl
{
namespace detail
{
template<typename T, size_t N>
struct FastFill
{
    static_assert( sizeof( T ) == N, "Type size mismatch" );

    static inline void fill(const gsl::not_null<T*>& data, size_t n, const T& value)
    {
        std::fill_n( data.get(), n, value );
    }
};


template<typename T>
struct FastFill<T, 1>
{
    static_assert( sizeof( T ) == 1, "Type size mismatch" );

    static inline void fill(const gsl::not_null<T*>& data, size_t n, const T& value)
    {
        std::memset( data.get(), value, n );
    }
};


template<typename T>
struct FastFill<T, 2>
{
    static_assert( sizeof( T ) == 2, "Type size mismatch" );

    static inline void fill(const gsl::not_null<T*>& data, size_t n, const T& value)
    {
        std::wmemset( data.get(), value, n );
    }
};


template<typename T>
struct FastFill<T, 4>
{
    static_assert( sizeof( T ) == 2, "Type size mismatch" );

    static inline void fill(const gsl::not_null<T*>& data, size_t n, const T& value)
    {
        std::wmemset( data.get(), value, n );
    }
};


template<>
struct FastFill<RGBA8, 4>
{
    static_assert( sizeof( RGBA8 ) == 4, "Type size mismatch" );

    static inline void fill(const gsl::not_null<RGBA8*>& data, size_t n, const RGBA8& value)
    {
        const auto scalar = value.r;
        if( scalar == value.g && scalar == value.b && scalar == value.a )
            std::memset( data.get(), scalar, n * 4u );
        else
            std::fill_n( data.get(), n, value );
    }
};


template<typename T>
inline void fill(const gsl::not_null<T*>& data, size_t n, const T& value)
{
    detail::FastFill<T, sizeof( T )>::fill( data, n, value );
}
}

template<typename TStorage>
class Image
{
public:
    using StorageType = TStorage;

    explicit Image(GLint width, GLint height, const StorageType* data = nullptr)
            : m_data{}
            , m_width{width}
            , m_height{height}
    {
        Expects( width > 0 && height > 0 );

        const auto dataSize = static_cast<size_t>(width * height);
        if( data == nullptr )
            m_data.resize( dataSize );
        else
            m_data.assign( data, data + dataSize );
    }

    ~Image() = default;

    const std::vector<StorageType>& getData() const
    {
        return m_data;
    }

    void assign(const std::vector<StorageType>& data)
    {
        Expects( m_data.size() == data.size() );
        m_data = data;
    }

    void assign(const StorageType* data, size_t size)
    {
        Expects( m_data.size() == size );
        m_data.assign( data + 0, data + size );
    }

    void assign(std::vector<StorageType>&& data)
    {
        Expects( m_data.size() == data.size() );
        m_data = std::move( data );
    }

    GLint getHeight() const
    {
        return m_height;
    }

    GLint getWidth() const
    {
        return m_width;
    }

    StorageType& at(GLint x, GLint y)
    {
        Expects( x >= 0 );
        Expects( x < m_width );
        Expects( y >= 0 );
        Expects( y < m_height );

        if( x < 0 || x >= m_width || y < 0 || y >= m_height )
        {
            BOOST_THROW_EXCEPTION( std::out_of_range{"Image coordinates out of range"} );
        }

        return m_data[y * m_width + x];
    }

    void set(GLint x, GLint y, const StorageType& pixel, bool blend = false)
    {
        if( x < 0 || x >= m_width || y < 0 || y >= m_height )
            return;

        const auto o = gsl::narrow_cast<size_t>( y * m_width + x );

        if( !blend )
        {
            m_data[o] = pixel;
        }
        else
        {
            m_data[o] = mixAlpha( m_data[o], pixel );
        }
    }

    const StorageType& at(GLint x, GLint y) const
    {
        Expects( x >= 0 );
        Expects( x < m_width );
        Expects( y >= 0 );
        Expects( y < m_height );

        if( x < 0 || x >= m_width || y < 0 || y >= m_height )
        {
            BOOST_THROW_EXCEPTION( std::out_of_range{"Image coordinates out of range"} );
        }

        return m_data[y * m_width + x];
    }

    void fill(const StorageType& color)
    {
        if( !m_data.empty() )
            detail::fill( gsl::make_not_null( m_data.data() ), m_data.size(), color );
    }

    void line(GLint x0, GLint y0, GLint x1, GLint y1, const StorageType& color, bool blend = false)
    {
        // shamelessly copied from wikipedia
        const GLint dx = abs( x1 - x0 );
        const GLint sx = x0 < x1 ? 1 : -1;
        const GLint dy = -abs( y1 - y0 );
        const GLint sy = y0 < y1 ? 1 : -1;

        GLint err = dx + dy;

        while( true )
        {
            set( x0, y0, color, blend );

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

    std::vector<StorageType> m_data;

    GLint m_width;

    GLint m_height;
};
}
}
