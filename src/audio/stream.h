#pragma once

#include "streamsource.h"
#include "sourcehandle.h"

namespace audio
{
class Device;


class Stream : public boost::noncopyable
{
private:
    std::unique_ptr<AbstractStreamSource> m_stream;
    std::weak_ptr<SourceHandle> m_source;
    gsl::not_null<std::shared_ptr<BufferHandle>> m_buffers[2];
    std::vector<int16_t> m_sampleBuffer;
    bool m_looping = false;

public:
    explicit Stream(Device& device, std::unique_ptr<AbstractStreamSource>&& src, size_t bufferSize);

    void update();

    const std::weak_ptr<SourceHandle>& getSource() const noexcept
    {
        return m_source;
    }

    std::weak_ptr<SourceHandle>& getSource() noexcept
    {
        return m_source;
    }

    void setLooping(const bool looping)
    {
        m_looping = looping;
    }

private:
    void init();

    void fillBuffer(BufferHandle& buffer);
};


inline bool isPlaying(const std::weak_ptr<Stream>& stream)
{
    if( stream.expired() )
        return false;

    const auto s = stream.lock();
    if( s->getSource().expired() )
        return false;

    const auto src = s->getSource().lock();
    return !src->isPaused() && !src->isStopped();
}
}
