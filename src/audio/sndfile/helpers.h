#pragma once

#include <gsl/gsl>

#include <sndfile.h>
#include <boost/assert.hpp>
#include <boost/iostreams/restrict.hpp>
#include <boost/iostreams/filtering_stream.hpp>

namespace audio
{
namespace sndfile
{
class MemBufferFileIo : public SF_VIRTUAL_IO
{
public:
    MemBufferFileIo(const uint8_t* data, const sf_count_t dataSize)
            : SF_VIRTUAL_IO{}
            , m_data{data}
            , m_dataSize{dataSize}
    {
        BOOST_ASSERT( data != nullptr );

        get_filelen = &MemBufferFileIo::getFileLength;
        seek = &MemBufferFileIo::doSeek;
        read = &MemBufferFileIo::doRead;
        write = &MemBufferFileIo::doWrite;
        tell = &MemBufferFileIo::doTell;
    }

    static sf_count_t getFileLength(void* user_data)
    {
        const auto self = static_cast<MemBufferFileIo*>(user_data);
        return self->m_dataSize;
    }

    static sf_count_t doSeek(const sf_count_t offset, const int whence, void* user_data)
    {
        auto self = static_cast<MemBufferFileIo*>(user_data);
        switch( whence )
        {
            case SEEK_SET:
                BOOST_ASSERT( offset >= 0 && offset <= self->m_dataSize );
                self->m_where = offset;
                break;
            case SEEK_CUR:
                BOOST_ASSERT( self->m_where + offset <= self->m_dataSize && self->m_where + offset >= 0 );
                self->m_where += offset;
                break;
            case SEEK_END:
                BOOST_ASSERT( offset >= 0 && offset <= self->m_dataSize );
                self->m_where = self->m_dataSize - offset;
                break;
            default:
                BOOST_ASSERT( false );
        }
        return self->m_where;
    }

    static sf_count_t doRead(void* ptr, sf_count_t count, void* user_data)
    {
        auto self = static_cast<MemBufferFileIo*>(user_data);
        if( self->m_where + count > self->m_dataSize )
            count = self->m_dataSize - self->m_where;

        BOOST_ASSERT( self->m_where + count <= self->m_dataSize );

        const auto buf = static_cast<uint8_t*>(ptr);
        std::copy_n( self->m_data + self->m_where, count, buf );
        self->m_where += count;
        return count;
    }

    static sf_count_t doWrite(const void* /*ptr*/, sf_count_t /*count*/, void* /*user_data*/)
    {
        return 0; // read-only
    }

    static sf_count_t doTell(void* user_data)
    {
        const auto self = static_cast<MemBufferFileIo*>(user_data);
        return self->m_where;
    }

private:
    const uint8_t* const m_data;
    const sf_count_t m_dataSize;
    sf_count_t m_where = 0;
};


class InputStreamViewWrapper : public SF_VIRTUAL_IO
{
public:
    InputStreamViewWrapper(std::istream& stream, const std::streamoff begin, const std::streamoff end)
            : SF_VIRTUAL_IO{}
            , m_restriction{boost::iostreams::restrict( stream, begin, end - begin )}
            , m_stream{m_restriction}
    {
        Expects( begin <= end );

        get_filelen = &InputStreamViewWrapper::getFileLength;
        seek = &InputStreamViewWrapper::doSeek;
        read = &InputStreamViewWrapper::doRead;
        write = &InputStreamViewWrapper::doWrite;
        tell = &InputStreamViewWrapper::doTell;
    }

    static sf_count_t getFileLength(void* user_data)
    {
        auto self = static_cast<InputStreamViewWrapper*>(user_data);
        const auto pos = self->m_stream.tellg();
        self->m_stream.seekg( 0, std::ios::end );
        const auto len = self->m_stream.tellg();
        self->m_stream.seekg( pos, std::ios::beg );
        return len;
    }

    static sf_count_t doSeek(const sf_count_t offset, const int whence, void* user_data)
    {
        auto self = static_cast<InputStreamViewWrapper*>(user_data);
        switch( whence )
        {
            case SEEK_SET:
                BOOST_ASSERT( offset >= 0 && offset <= getFileLength( user_data ) );
                self->m_stream.seekg( offset, std::ios::beg );
                break;
            case SEEK_CUR:
                BOOST_ASSERT( self->m_stream.tellg() + offset <= getFileLength( user_data )
                              && self->m_stream.tellg() + offset >= 0 );
                self->m_stream.seekg( offset, std::ios::cur );
                break;
            case SEEK_END:
                BOOST_ASSERT( offset >= 0 && offset <= getFileLength( user_data ) );
                self->m_stream.seekg( offset, std::ios::end );
                break;
            default:
                BOOST_ASSERT( false );
        }
        return self->m_stream.tellg();
    }

    static sf_count_t doRead(void* ptr, sf_count_t count, void* user_data)
    {
        auto self = static_cast<InputStreamViewWrapper*>(user_data);
        if( self->m_stream.tellg() + count > getFileLength( user_data ) )
            count = getFileLength( user_data ) - self->m_stream.tellg();

        BOOST_ASSERT( self->m_stream.tellg() + count <= getFileLength( user_data ) );

        const auto buf = static_cast<char*>(ptr);
        self->m_stream.read( buf, count );
        return self->m_stream.gcount();
    }

    static sf_count_t doWrite(const void* /*ptr*/, sf_count_t /*count*/, void* /*user_data*/)
    {
        return 0; // read-only
    }

    static sf_count_t doTell(void* user_data)
    {
        auto self = static_cast<InputStreamViewWrapper*>(user_data);
        return self->m_stream.tellg();
    }

private:
    boost::iostreams::restriction<std::istream> m_restriction;
    boost::iostreams::filtering_istream m_stream;
};
}
}
