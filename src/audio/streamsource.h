#pragma once

#include "sndfile/helpers.h"

#include <boost/throw_exception.hpp>
#include <boost/log/trivial.hpp>

#include <gsl/gsl>

#include <sndfile.h>

#include <fstream>

namespace audio
{
    class AbstractStreamSource
    {
    public:
        virtual ~AbstractStreamSource() = default;

        bool atEnd() const noexcept
        {
            return m_eof;
        }

        virtual size_t readStereo(int16_t* buffer, size_t bufferSize) = 0;
        virtual int getSampleRate() const = 0;

    protected:
        size_t readStereo(int16_t* frameBuffer, size_t frameCount, SNDFILE* sndFile, bool sourceIsMono)
        {
            size_t count = 0;
            while( count < frameCount )
            {
                const auto n = sf_readf_short(sndFile, frameBuffer + count, frameCount - count);
                count += n;
                if( n == 0 )
                    break;

                // restart if there are not enough samples
                if( count < frameCount )
                    sf_seek(sndFile, 0, SEEK_SET);
            }

            if( sourceIsMono )
            {
                // Need to duplicate the samples from mono to stereo
                for( size_t i = 0; i < frameCount; ++i )
                {
                    const size_t src = frameCount - i - 1;
                    const size_t dest = 2 * frameCount - i - 2;
                    frameBuffer[dest] = frameBuffer[dest + 1] = frameBuffer[src];
                }
            }

            if( count < frameCount )
                m_eof = true;

            return count;
        }

    private:
        bool m_eof = false;
    };

    class WadStreamSource final : public AbstractStreamSource
    {
    private:
        std::ifstream m_wadFile;
        SF_INFO m_sfInfo;
        SNDFILE* m_sndFile = nullptr;
        std::unique_ptr<sndfile::InputStreamViewWrapper> m_wrapper;

        // CDAUDIO.WAD step size defines CDAUDIO's header stride, on which each track
        // info is placed. Also CDAUDIO count specifies static amount of tracks existing
        // in CDAUDIO.WAD file. Name length specifies maximum string size for trackname.
        static constexpr size_t WADStride = 268;
        static constexpr size_t WADNameLength = 260;
        static constexpr size_t WADCount = 130;

    public:
        WadStreamSource(const std::string& filename, size_t trackIndex)
            : m_wadFile(filename, std::ios::in | std::ios::binary)
        {
            memset(&m_sfInfo, 0, sizeof(m_sfInfo));

            if( !m_wadFile.is_open() )
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
            m_wrapper = std::make_unique<sndfile::InputStreamViewWrapper>(m_wadFile, offset, offset + length);

            m_sndFile = sf_open_virtual(m_wrapper.get(), SFM_READ, &m_sfInfo, m_wrapper.get());
            if( m_sndFile == nullptr )
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to open WAD file: " << sf_strerror(nullptr);
                BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open WAD file"));
            }
        }

        size_t readStereo(int16_t* frameBuffer, size_t frameCount) override
        {
            return AbstractStreamSource::readStereo(frameBuffer, frameCount, m_sndFile, m_sfInfo.channels == 1);
        }

        int getSampleRate() const override
        {
            return m_sfInfo.samplerate;
        }
    };

    class SndfileStreamSource final : public AbstractStreamSource
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
            if( m_sndFile == nullptr )
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to open audio file: " << sf_strerror(nullptr);
                BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open audio file"));
            }
        }

        size_t readStereo(int16_t* frameBuffer, size_t frameCount) override
        {
            return AbstractStreamSource::readStereo(frameBuffer, frameCount, m_sndFile, m_sfInfo.channels == 1);
        }

        int getSampleRate() const override
        {
            return m_sfInfo.samplerate;
        }
    };
}
