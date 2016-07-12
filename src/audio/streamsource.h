#pragma once

#include <fstream>
#include <string>

#include <boost/throw_exception.hpp>
#include <boost/log/trivial.hpp>

#include <gsl.h>

#include <sndfile.h>
#include "bufferhandle.h"
#include "sourcehandle.h"

namespace audio
{
    class IStreamSource
    {
    public:
        virtual ~IStreamSource() = default;

        virtual bool atEnd() const = 0;
        virtual size_t readStereo(int16_t* buffer, size_t bufferSize) = 0;
        virtual int getSampleRate() const = 0;
    };

    class WadStreamSource final : public IStreamSource
    {
    private:
        struct InputStreamViewWrapper : public SF_VIRTUAL_IO
        {
            InputStreamViewWrapper(std::istream& stream, std::streamoff begin, std::streamoff end)
                : SF_VIRTUAL_IO()
                , m_stream(stream)
                , m_begin(begin)
                , m_end(end)
            {
                Expects(begin <= end);

                get_filelen = &InputStreamViewWrapper::getFileLength;
                seek = &InputStreamViewWrapper::doSeek;
                read = &InputStreamViewWrapper::doRead;
                write = &InputStreamViewWrapper::doWrite;
                tell = &InputStreamViewWrapper::doTell;
            }

            static sf_count_t getFileLength(void* user_data)
            {
                auto self = static_cast<InputStreamViewWrapper*>(user_data);
                return self->m_end - self->m_begin;
            }

            static sf_count_t doSeek(sf_count_t offset, int whence, void* user_data)
            {
                auto self = static_cast<InputStreamViewWrapper*>(user_data);
                switch(whence)
                {
                    case SEEK_SET:
                        BOOST_ASSERT(offset >= 0 && offset <= self->m_end - self->m_begin);
                        self->m_stream.seekg(offset + self->m_begin, std::ios::beg);
                        break;
                    case SEEK_CUR:
                        BOOST_ASSERT(self->m_stream.tellg() + offset <= self->m_end && self->m_stream.tellg() + offset >= 0);
                        self->m_stream.seekg(offset, std::ios::cur);
                        break;
                    case SEEK_END:
                        BOOST_ASSERT(offset >= 0 && offset <= self->m_end - self->m_begin);
                        self->m_stream.seekg(self->m_end - offset, std::ios::beg);
                        break;
                    default:
                        BOOST_ASSERT(false);
                }
                return self->m_stream.tellg();
            }

            static sf_count_t doRead(void* ptr, sf_count_t count, void* user_data)
            {
                auto self = static_cast<InputStreamViewWrapper*>(user_data);
                if(self->m_stream.tellg() + count > self->m_end)
                    count = self->m_end - self->m_stream.tellg();

                BOOST_ASSERT(self->m_stream.tellg() + count <= self->m_end);

                char* buf = static_cast<char*>(ptr);
                self->m_stream.read(buf, count);
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
            std::istream& m_stream;
            const std::streamoff m_begin;
            const std::streamoff m_end;
        };

        std::ifstream m_wadFile;
        SF_INFO m_sfInfo;
        SNDFILE* m_sndFile = nullptr;
        std::unique_ptr<InputStreamViewWrapper> m_wrapper;
        bool m_eof = false;

        // CDAUDIO.WAD step size defines CDAUDIO's header stride, on which each track
        // info is placed. Also CDAUDIO count specifies static amount of tracks existing
        // in CDAUDIO.WAD file. Name length specifies maximum string size for trackname.
        static constexpr size_t WADStride = 268;
        static constexpr size_t WADNameLength = 260;
        static constexpr size_t WADCount = 130;

    public:
        WadStreamSource(const std::string& filename, size_t trackIndex)
            : m_wadFile(filename, std::ios::in|std::ios::binary)
        {
            memset(&m_sfInfo, 0, sizeof(m_sfInfo));

            if(!m_wadFile.is_open())
                BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open WAD file"));

            m_wadFile.seekg(trackIndex * WADStride, std::ios::beg);

            char trackName[WADNameLength];
            m_wadFile.read(trackName, WADNameLength);

            BOOST_LOG_TRIVIAL(info) << "Loading WAD track " << trackIndex << ": " << trackName;

            uint32_t offset = 0;
            m_wadFile.read(reinterpret_cast<char*>(&offset), 4);

            uint32_t length = 0;
            m_wadFile.read(reinterpret_cast<char*>(&length), 4);

            m_wadFile.seekg(offset, std::ios::beg);
            m_wrapper = std::make_unique<InputStreamViewWrapper>(m_wadFile, offset, offset + length);

            m_sndFile = sf_open_virtual(m_wrapper.get(), SFM_READ, &m_sfInfo, m_wrapper.get());
            if(m_sndFile == nullptr)
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to open WAD file: " << sf_strerror(nullptr);
                BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open WAD file"));
            }
        }

        bool atEnd() const override
        {
            return m_eof;
        }

        size_t readStereo(int16_t* frameBuffer, size_t frameCount) override
        {
            sf_count_t count = 0;
            while(count < frameCount)
            {
                const auto n = sf_readf_short(m_sndFile, frameBuffer + count, frameCount - count);
                count += n;
                if(n == 0)
                    break;

                // restart if there are not enough samples
                if(count < frameCount)
                    sf_seek(m_sndFile, 0, SF_SEEK_SET);
            }

            if(m_sfInfo.channels != 2)
            {
                // Need to duplicate the samples from mono to stereo
                for(size_t i = 0; i < frameCount; ++i)
                {
                    const size_t src = frameCount - i - 1;
                    const size_t dest = 2*frameCount - i - 2;
                    frameBuffer[dest] = frameBuffer[dest + 1] = frameBuffer[src];
                }
            }

            if(count < frameCount)
                m_eof = true;

            return count;
        }

        int getSampleRate() const override
        {
            return m_sfInfo.samplerate;
        }
    };

    class SndfileStreamSource final : public IStreamSource
    {
    private:
        SF_INFO m_sfInfo;
        SNDFILE* m_sndFile = nullptr;
        bool m_eof = false;

        // CDAUDIO.WAD step size defines CDAUDIO's header stride, on which each track
        // info is placed. Also CDAUDIO count specifies static amount of tracks existing
        // in CDAUDIO.WAD file. Name length specifies maximum string size for trackname.
        static constexpr size_t WADStride = 268;
        static constexpr size_t WADNameLength = 260;
        static constexpr size_t WADCount = 130;

    public:
        explicit SndfileStreamSource(const std::string& filename)
        {
            memset(&m_sfInfo, 0, sizeof(m_sfInfo));

            m_sndFile = sf_open(filename.c_str(), SFM_READ, &m_sfInfo);
            if(m_sndFile == nullptr)
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to open OGG file: " << sf_strerror(nullptr);
                BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open WAD file"));
            }
        }

        bool atEnd() const override
        {
            return m_eof;
        }

        size_t readStereo(int16_t* frameBuffer, size_t frameCount) override
        {
            sf_count_t count = 0;
            while(count < frameCount)
            {
                const auto n = sf_readf_short(m_sndFile, frameBuffer + count, frameCount - count);
                count += n;
                if(n == 0)
                    break;

                // restart if there are not enough samples
                if(count < frameCount)
                    sf_seek(m_sndFile, 0, SF_SEEK_SET);
            }

            if(m_sfInfo.channels != 2)
            {
                // Need to duplicate the samples from mono to stereo
                for(size_t i = 0; i < frameCount; ++i)
                {
                    const size_t src = frameCount - i - 1;
                    const size_t dest = 2 * frameCount - i - 2;
                    frameBuffer[dest] = frameBuffer[dest + 1] = frameBuffer[src];
                }
            }

            if(count < frameCount)
                m_eof = true;

            return count;
        }

        int getSampleRate() const override
        {
            return m_sfInfo.samplerate;
        }
    };

    class Stream
    {
    private:
        std::unique_ptr<IStreamSource> m_stream;
        SourceHandle m_source;
        BufferHandle m_buffers[2];
        std::vector<int16_t> m_sampleBuffer;

    public:
        explicit Stream(std::unique_ptr<IStreamSource>&& src, size_t bufferSize)
            : m_stream(std::move(src))
            , m_sampleBuffer(bufferSize*2)
        {
            fillBuffer(m_buffers[0]);
            fillBuffer(m_buffers[1]);

            auto tmp = m_buffers[0].get();
            alSourceQueueBuffers(m_source.get(), 1, &tmp);
            tmp = m_buffers[1].get();
            alSourceQueueBuffers(m_source.get(), 1, &tmp);

            m_source.play();
        }

        void update()
        {
            ALint processed = 0;
            alGetSourcei(m_source.get(), AL_BUFFERS_PROCESSED, &processed);
            if(processed == 0)
                return;

            ALuint bufId;
            alSourceUnqueueBuffers(m_source.get(), 1, &bufId);

            if(bufId == m_buffers[0].get())
                fillBuffer(m_buffers[0]);
            else if(bufId == m_buffers[1].get())
                fillBuffer(m_buffers[1]);
            else
                BOOST_THROW_EXCEPTION(std::runtime_error("Stream buffer handle curruption"));

            alSourceQueueBuffers(m_source.get(), 1, &bufId);
        }

    private:
        void fillBuffer(BufferHandle& buffer)
        {
            const auto framesRead = m_stream->readStereo(m_sampleBuffer.data(), m_sampleBuffer.size() / 2);
            buffer.fill(m_sampleBuffer.data(), framesRead * 2, 2, m_stream->getSampleRate());
        }
    };
}
