#pragma once

#include "streamsource.h"

namespace audio
{
    class Stream : public boost::noncopyable
    {
    private:
        std::unique_ptr<AbstractStreamSource> m_stream;
        SourceHandle m_source;
        BufferHandle m_buffers[2];
        std::vector<int16_t> m_sampleBuffer;

    public:
        explicit Stream(std::unique_ptr<AbstractStreamSource>&& src, size_t bufferSize)
            : m_stream(std::move(src))
            , m_sampleBuffer(bufferSize * 2)
        {
            init();

            m_source.play();
        }

        void update()
        {
            ALint processed = 0;
            alGetSourcei(m_source.get(), AL_BUFFERS_PROCESSED, &processed);
            if( processed == 0 )
                return;

            ALuint bufId;
            alSourceUnqueueBuffers(m_source.get(), 1, &bufId);

            if(bufId == m_buffers[0].get())
            {
                fillBuffer(m_buffers[0]);
            }
            else if(bufId == m_buffers[1].get())
            {
                fillBuffer(m_buffers[1]);
            }
            else
            {
                BOOST_LOG_TRIVIAL(error) << "Stream buffer torn, re-init";
                init();
                return;
            }

            alSourceQueueBuffers(m_source.get(), 1, &bufId);
        }

        const SourceHandle& getSource() const noexcept
        {
            return m_source;
        }

        SourceHandle& getSource() noexcept
        {
            return m_source;
        }

    private:
        void init()
        {
            fillBuffer(m_buffers[0]);
            fillBuffer(m_buffers[1]);

            auto tmp = m_buffers[0].get();
            alSourceQueueBuffers(m_source.get(), 1, &tmp);
            tmp = m_buffers[1].get();
            alSourceQueueBuffers(m_source.get(), 1, &tmp);
        }


        void fillBuffer(BufferHandle& buffer)
        {
            const auto framesRead = m_stream->readStereo(m_sampleBuffer.data(), m_sampleBuffer.size() / 2);
            buffer.fill(m_sampleBuffer.data(), framesRead * 2, 2, m_stream->getSampleRate());
        }
    };
}
