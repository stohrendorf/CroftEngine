#include "streamvoice.h"

#include "audio/core.h"
#include "bufferhandle.h"
#include "sourcehandle.h"
#include "streamsource.h"
#include "voice.h"

#include <AL/al.h>
#include <algorithm>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <chrono>
#include <cstddef>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <stdexcept>
#include <utility>

namespace audio
{
StreamVoice::StreamVoice(std::unique_ptr<StreamingSourceHandle>&& streamSource,
                         std::unique_ptr<AbstractStreamSource>&& source,
                         const size_t bufferSize,
                         const size_t bufferCount,
                         const std::chrono::milliseconds& initialPosition)
    : m_streamSource{dynamic_cast<StreamingSourceHandle*>(streamSource.get())}
    , m_stream{std::move(source)}
    , m_sampleBuffer(bufferSize * 2)
{
  BOOST_LOG_TRIVIAL(trace) << "Created AL stream with buffer size " << bufferSize << " and " << bufferCount
                           << " buffers";

  gsl_Expects(bufferSize > 0);
  gsl_Expects(bufferCount >= 2);

  m_stream->seek(initialPosition);

  m_buffers.reserve(bufferCount);
  for(size_t i = 0; i < bufferCount; ++i)
  {
    auto buffer = std::make_shared<BufferHandle>();
    m_buffers.emplace_back(buffer);
    fillBuffer(*buffer);
    m_streamSource->queueBuffer(buffer);
  }

  Voice::associate(std::move(streamSource));
}

void StreamVoice::update()
{
  if(isPaused())
    return;

  if(isStopped() && !m_looping)
    return;

  ALint processed = m_streamSource->getBuffersProcessed();
  gsl_Assert(processed >= 0 && static_cast<size_t>(processed) <= m_buffers.size());

  if(static_cast<size_t>(processed) >= m_buffers.size())
  {
    BOOST_LOG_TRIVIAL(warning) << "Lost stream sync";
    // source should in theory be stopped now as all buffers are processed, but ensure it really is to guarantee
    // a cleared queue
    m_streamSource->stop();
    // refill buffers and play
    for(const auto& buffer : m_buffers)
    {
      if(fillBuffer(*buffer))
        m_streamSource->queueBuffer(buffer);
    }
    m_streamSource->play();
    return;
  }

  while(processed-- > 0)
  {
    const auto buffer = m_streamSource->unqueueBuffer();
    auto it = std::find(m_buffers.begin(), m_buffers.end(), buffer);
    if(it == m_buffers.end())
    {
      BOOST_LOG_TRIVIAL(error) << "Got unexpected buffer ID #" << static_cast<ALuint>(*buffer);
      continue;
    }

    if(fillBuffer(*buffer))
      m_streamSource->queueBuffer(buffer);
  }
}

bool StreamVoice::fillBuffer(BufferHandle& buffer)
{
  const auto framesRead
    = m_stream->read(m_sampleBuffer.data(), m_sampleBuffer.size() / m_stream->getChannels(), m_looping);
  buffer.fill(m_sampleBuffer.data(), framesRead, m_stream->getChannels(), m_stream->getSampleRate());
  return framesRead > 0;
}

std::chrono::milliseconds StreamVoice::getStreamPosition() const
{
  return m_stream->getPosition();
}

void StreamVoice::seek(const std::chrono::milliseconds& position)
{
  m_stream->seek(position);
}

void StreamVoice::associate(std::unique_ptr<SourceHandle>&& /*source*/)
{
  BOOST_THROW_EXCEPTION(std::runtime_error("Re-association of streams not supported"));
}

Clock::duration StreamVoice::getDuration() const
{
  return m_stream->getDuration();
}

StreamVoice::~StreamVoice() = default;
} // namespace audio
