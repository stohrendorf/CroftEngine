#pragma once

#include "utils.h"

#include <al.h>
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <gsl.h>
#include <sndfile.h>
#include <boost/log/trivial.hpp>
#include <vector>

namespace audio
{
    namespace details
    {
        struct MemBufferFileIo : public SF_VIRTUAL_IO
        {
            MemBufferFileIo(const uint8_t* data, sf_count_t dataSize)
                : SF_VIRTUAL_IO()
                , m_data(data)
                , m_dataSize(dataSize)
            {
                BOOST_ASSERT(data != nullptr);

                get_filelen = &MemBufferFileIo::getFileLength;
                seek = &MemBufferFileIo::doSeek;
                read = &MemBufferFileIo::doRead;
                write = &MemBufferFileIo::doWrite;
                tell = &MemBufferFileIo::doTell;
            }

            static sf_count_t getFileLength(void* user_data)
            {
                auto self = static_cast<MemBufferFileIo*>(user_data);
                return self->m_dataSize;
            }

            static sf_count_t doSeek(sf_count_t offset, int whence, void* user_data)
            {
                auto self = static_cast<MemBufferFileIo*>(user_data);
                switch( whence )
                {
                case SEEK_SET:
                    BOOST_ASSERT(offset >= 0 && offset <= self->m_dataSize);
                    self->m_where = offset;
                    break;
                case SEEK_CUR:
                    BOOST_ASSERT(self->m_where + offset <= self->m_dataSize && self->m_where + offset >= 0);
                    self->m_where += offset;
                    break;
                case SEEK_END:
                    BOOST_ASSERT(offset >= 0 && offset <= self->m_dataSize);
                    self->m_where = self->m_dataSize - offset;
                    break;
                default:
                    BOOST_ASSERT(false);
                }
                return self->m_where;
            }

            static sf_count_t doRead(void* ptr, sf_count_t count, void* user_data)
            {
                auto self = static_cast<MemBufferFileIo*>(user_data);
                if( self->m_where + count > self->m_dataSize )
                    count = self->m_dataSize - self->m_where;

                BOOST_ASSERT(self->m_where + count <= self->m_dataSize);

                uint8_t* buf = static_cast<uint8_t*>(ptr);
                std::copy(self->m_data + self->m_where, self->m_data + self->m_where + count, buf);
                self->m_where += count;
                return count;
            }

            static sf_count_t doWrite(const void* /*ptr*/, sf_count_t /*count*/, void* /*user_data*/)
            {
                return 0; // read-only
            }

            static sf_count_t doTell(void* user_data)
            {
                auto self = static_cast<MemBufferFileIo*>(user_data);
                return self->m_where;
            }

        private:
            const uint8_t* const m_data;
            const sf_count_t m_dataSize;
            sf_count_t m_where = 0;
        };
    }

    class BufferHandle : public boost::noncopyable
    {
        const ALuint m_handle;

        static ALuint createHandle()
        {
            ALuint handle;
            alGenBuffers(1, &handle);
            DEBUG_CHECK_AL_ERROR();

            Expects(alIsBuffer(handle));

            return handle;
        }

    public:
        explicit BufferHandle(ALuint h)
            : m_handle(h)
        {
            Expects(alIsBuffer(h));
        }

        explicit BufferHandle()
            : m_handle(createHandle())
        {
        }

        ~BufferHandle()
        {
            alDeleteBuffers(1, &m_handle);
            DEBUG_CHECK_AL_ERROR();
        }

        ALuint get() const noexcept
        {
            return m_handle;
        }

        bool fillFromWav(const uint8_t* data)
        {
            Expects(data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F');
            Expects(data[8] == 'W' && data[9] == 'A' && data[10] == 'V' && data[11] == 'E');

            details::MemBufferFileIo wavMem(data, *reinterpret_cast<const uint32_t*>(data + 4) + 8);
            SF_INFO sfInfo;
            memset(&sfInfo, 0, sizeof(sfInfo));
            SNDFILE* sfFile = sf_open_virtual(&wavMem, SFM_READ, &sfInfo, &wavMem);

            if(!sfFile)
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to load WAV sample from memory: " << sf_strerror(sfFile);
                return false;
            }

            BOOST_ASSERT(sfInfo.frames >= 0);

            std::vector<int16_t> pcm(sfInfo.frames);
            sf_readf_short(sfFile, pcm.data(), sfInfo.frames);

            alBufferData(m_handle, sfInfo.channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, pcm.data(), pcm.size() * sizeof(pcm[0]), sfInfo.samplerate);
            DEBUG_CHECK_AL_ERROR();

            sf_close(sfFile);

            return true;
        }
    };
}
