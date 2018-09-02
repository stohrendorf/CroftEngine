#pragma once

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include <zlib.h>

#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>

#include <boost/version.hpp>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace loader
{
namespace io
{
using DataStreamBuf = boost::iostreams::filtering_istreambuf;


class SDLReader
{
public:
    SDLReader(const SDLReader&) = delete;

    SDLReader& operator=(const SDLReader&) = delete;

    explicit SDLReader(std::shared_ptr<DataStreamBuf> stream)
            : m_streamBuf( std::move( stream ) )
            , m_stream( m_streamBuf.get() )
    {
    }

    SDLReader(SDLReader&& rhs) noexcept
            : m_memory( move( rhs.m_memory ) )
            , m_file( move( rhs.m_file ) )
            , m_array( move( rhs.m_array ) )
            , m_streamBuf( move( rhs.m_streamBuf ) )
            , m_stream( m_streamBuf.get() )
    {
    }

    explicit SDLReader(const std::string& filename)
            : m_file( std::make_unique<boost::iostreams::file>( filename, std::ios::in | std::ios::binary,
                                                                std::ios::in | std::ios::binary ) )
            , m_streamBuf( std::make_shared<DataStreamBuf>( *m_file ) )
            , m_stream( m_streamBuf.get() )
    {
    }

    explicit SDLReader(const std::vector<char>& data)
            : m_memory( data )
            , m_array( std::make_unique<boost::iostreams::array>( m_memory.data(), m_memory.size() ) )
            , m_streamBuf( std::make_shared<DataStreamBuf>( *m_array ) )
            , m_stream( m_streamBuf.get() )
    {
    }

    explicit SDLReader(std::vector<char>&& data)
            : m_memory( move( data ) )
            , m_array( std::make_unique<boost::iostreams::array>( m_memory.data(), m_memory.size() ) )
            , m_streamBuf( std::make_shared<DataStreamBuf>( *m_array ) )
            , m_stream( m_streamBuf.get() )
    {
    }

    ~SDLReader() = default;

    static SDLReader decompress(const std::vector<uint8_t>& compressed, size_t uncompressedSize)
    {
        std::vector<char> uncomp_buffer( uncompressedSize );

        auto size = static_cast<uLongf>(uncompressedSize);
        if( uncompress( reinterpret_cast<Bytef*>(uncomp_buffer.data()), &size, compressed.data(),
                        static_cast<uLong>(compressed.size()) ) != Z_OK )
            BOOST_THROW_EXCEPTION( std::runtime_error( "Decompression failed" ) );

        if( size != uncompressedSize )
            BOOST_THROW_EXCEPTION( std::runtime_error( "Decompressed size mismatch" ) );

        SDLReader reader( move( uncomp_buffer ) );
        if( !reader.isOpen() )
            BOOST_THROW_EXCEPTION( std::runtime_error( "Failed to create reader from decompressed memory" ) );

        return reader;
    }

    bool isOpen() const
    {
        return !m_stream.bad();
    }

    std::streampos tell()
    {
        return m_stream.tellg();
    }

    std::streamsize size()
    {
        const auto pos = m_stream.tellg();
        m_stream.seekg( 0, std::ios::end );
        const auto size = m_stream.tellg();
        m_stream.seekg( pos, std::ios::beg );
        return size;
    }

    void skip(std::streamoff delta)
    {
        m_stream.seekg( delta, std::ios::cur );
    }

    void seek(std::streampos position)
    {
        m_stream.seekg( position, std::ios::beg );
    }

    template<typename T>
    void readBytes(T* dest, size_t n)
    {
        static_assert( std::is_integral<T>::value && sizeof( T ) == 1,
                       "readBytes() only allowed for byte-compatible data" );
        m_stream.read( reinterpret_cast<char*>(dest), n );
        if( static_cast<size_t>(m_stream.gcount()) != n )
        {
            BOOST_THROW_EXCEPTION( std::runtime_error( "EOF unexpectedly reached" ) );
        }
    }

    template<typename T>
    using PtrProducer = std::unique_ptr<T>(SDLReader&);

    template<typename T>
    using StackProducer = T(SDLReader&);

    template<typename T>
    void readVector(std::vector<T>& elements, size_t count, PtrProducer<T> producer)
    {
        elements.clear();
        appendVector( elements, count, producer );
    }

    template<typename T>
    void readVector(std::vector<T>& elements, size_t count, StackProducer<T> producer)
    {
        elements.clear();
        appendVector( elements, count, producer );
    }

    template<typename T>
    void appendVector(std::vector<T>& elements, size_t count, PtrProducer<T> producer)
    {
        elements.reserve( elements.size() + count );
        for( size_t i = 0; i < count; ++i )
        {
            elements.emplace_back( std::move( *producer( *this ) ) );
        }
    }

    template<typename T>
    void appendVector(std::vector<T>& elements, size_t count, StackProducer<T> producer)
    {
        elements.reserve( elements.size() + count );
        for( size_t i = 0; i < count; ++i )
        {
            elements.emplace_back( producer( *this ) );
        }
    }

    template<typename T>
    void readVector(std::vector<T>& elements, size_t count)
    {
        elements.clear();
        elements.reserve( count );
        for( size_t i = 0; i < count; ++i )
        {
            elements.emplace_back( read<T>() );
        }
    }

    void readVector(std::vector<uint8_t>& elements, size_t count)
    {
        elements.clear();
        elements.resize( count );
        readBytes( elements.data(), count );
    }

    void readVector(std::vector<int8_t>& elements, size_t count)
    {
        elements.clear();
        elements.resize( count );
        readBytes( elements.data(), count );
    }

    template<typename T>
    T read()
    {
        T result;
        m_stream.read( reinterpret_cast<char*>(&result), sizeof( T ) );
        if( m_stream.gcount() != sizeof( T ) )
        {
            BOOST_THROW_EXCEPTION( std::runtime_error( "EOF unexpectedly reached" ) );
        }

        SwapTraits<T, sizeof( T ), std::is_integral<T>::value || std::is_floating_point<T>::value>::doSwap( result );

        return result;
    }

    uint8_t readU8()
    {
        return read<uint8_t>();
    }

    int8_t readI8()
    {
        return read<int8_t>();
    }

    uint16_t readU16()
    {
        return read<uint16_t>();
    }

    int16_t readI16()
    {
        return read<int16_t>();
    }

    uint32_t readU32()
    {
        return read<uint32_t>();
    }

    int32_t readI32()
    {
        return read<int32_t>();
    }

    float readF()
    {
        return read<float>();
    }

private:
    // Do not change the order of these member variables.
    std::vector<char> m_memory;

    std::unique_ptr<boost::iostreams::file> m_file;

    std::unique_ptr<boost::iostreams::array> m_array;

    std::shared_ptr<DataStreamBuf> m_streamBuf;

    std::istream m_stream;

    template<typename T, int dataSize, bool isIntegral>
    struct SwapTraits
    {
    };


    template<typename T, int dataSize>
    struct SwapTraits<T, dataSize, true>
    {
        static void doSwap(T& /*data*/)
        {
            //! @todo For now, no endian conversion
        }
    };
};
} // namespace io
} // namespace loader
