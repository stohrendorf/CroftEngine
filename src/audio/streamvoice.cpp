#include "streamvoice.h"

#include "bufferhandle.h"

#include <boost/log/trivial.hpp>

namespace audio
{
StreamVoice::StreamVoice(std::unique_ptr<AbstractStreamSource>&& source,
                         const size_t bufferSize,
                         const size_t bufferCount,
                         const std::chrono::milliseconds& initialPosition)
    : Voice{std::make_shared<StreamingSourceHandle>()}
    , m_stream{std::move(source)}
    , m_sampleBuffer(bufferSize * 2)
{
  BOOST_LOG_TRIVIAL(trace) << "Created AL stream with buffer size " << bufferSize << " and " << bufferCount
                           << " buffers";

  Expects(bufferSize > 0);
  Expects(bufferCount >= 2);

  m_stream->seek(initialPosition);

  auto sourceHandle = static_cast<StreamingSourceHandle*>(getSource().get().get());
  m_buffers.reserve(bufferCount);
  for(size_t i = 0; i < bufferCount; ++i)
  {
    auto buffer = std::make_shared<BufferHandle>();
    m_buffers.emplace_back(buffer);
    fillBuffer(*buffer);
    sourceHandle->queueBuffer(buffer);
  }
}

void StreamVoice::update()
{
  if(isPaused())
    return;

  if(isStopped() && !m_looping)
    return;

  auto sourceHandle = static_cast<StreamingSourceHandle*>(getSource().get().get());

  ALint processed = sourceHandle->getBuffersProcessed();
  Expects(processed >= 0 && static_cast<size_t>(processed) <= m_buffers.size());

  if(static_cast<size_t>(processed) >= m_buffers.size())
    BOOST_LOG_TRIVIAL(warning) << "Lost stream sync";

  while(processed-- > 0)
  {
    const auto buffer = sourceHandle->unqueueBuffer();
    auto it = std::find(m_buffers.begin(), m_buffers.end(), buffer);
    if(it == m_buffers.end())
    {
      BOOST_LOG_TRIVIAL(error) << "Got unexpected buffer ID #" << buffer->get();
      continue;
    }

    if(fillBuffer(*buffer))
      sourceHandle->queueBuffer(buffer);
    break;
  }
}

bool StreamVoice::fillBuffer(BufferHandle& buffer)
{
  const auto framesRead = m_stream->readStereo(m_sampleBuffer.data(), m_sampleBuffer.size() / 2, m_looping);
  buffer.fill(m_sampleBuffer.data(), framesRead * 2, 2, m_stream->getSampleRate());
  return framesRead > 0;
}

std::chrono::milliseconds StreamVoice::getPosition() const
{
  return m_stream->getPosition();
}

void StreamVoice::seek(const std::chrono::milliseconds& position)
{
  m_stream->seek(position);
}
} // namespace audio
