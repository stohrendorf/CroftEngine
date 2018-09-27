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

private:
    void init();

    void fillBuffer(BufferHandle& buffer);
};
}
