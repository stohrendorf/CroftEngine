#include "sourcehandle.h"

#include "bufferhandle.h"
#include "core/magic.h"
#include "filterhandle.h"
#include "handle.h"
#include "qs/quantity.h"
#include "utils.h"

#include <AL/efx.h>
#include <algorithm>
#include <boost/throw_exception.hpp>
#include <glm/fwd.hpp>
#include <map>
#include <stdexcept>
#include <thread>

namespace audio
{
SourceHandle::SourceHandle(bool positional)
    : Handle{alGenSources, alIsSource, alDeleteSources}
{
  if(positional)
  {
    set(AL_REFERENCE_DISTANCE, 2 * core::SectorSize.get());
    set(AL_AIR_ABSORPTION_FACTOR, 1.0f);
  }
  else
  {
    set(AL_REFERENCE_DISTANCE, 0.0f);
    set(AL_AIR_ABSORPTION_FACTOR, 0.0f);
  }
}

SourceHandle::~SourceHandle()
{
  SourceHandle::stop();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::setDirectFilter(const std::shared_ptr<FilterHandle>& f)
{
  AL_ASSERT(alSourcei(*this, AL_DIRECT_FILTER, f ? *f : AL_FILTER_NULL));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::set(const ALenum e, const ALint v)
{
  AL_ASSERT(alSourcei(*this, e, v));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::set(const ALenum e, const ALfloat v)
{
  AL_ASSERT(alSourcef(*this, e, v));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::set(const ALenum e, const ALfloat a, const ALfloat b, const ALfloat c)
{
  AL_ASSERT(alSource3f(*this, e, a, b, c));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::play()
{
  AL_ASSERT(alSourcePlay(*this));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::pause()
{
  AL_ASSERT(alSourcePause(*this));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::rewind()
{
  AL_ASSERT(alSourceRewind(*this));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::stop()
{
  AL_ASSERT(alSourceStop(*this));
}

bool SourceHandle::isStopped() const
{
  ALenum state = AL_STOPPED;
  AL_ASSERT(alGetSourcei(*this, AL_SOURCE_STATE, &state));

  return state == AL_STOPPED;
}

bool SourceHandle::isPaused() const
{
  ALenum state = AL_STOPPED;
  AL_ASSERT(alGetSourcei(*this, AL_SOURCE_STATE, &state));

  return state == AL_PAUSED;
}

void SourceHandle::setLooping(const bool isLooping)
{
  set(AL_LOOPING, isLooping ? AL_TRUE : AL_FALSE);
}

void SourceHandle::setGain(const ALfloat gain)
{
  set(AL_GAIN, std::clamp(gain, 0.0f, 1.0f));
}

void SourceHandle::setPosition(const glm::vec3& position)
{
  set(AL_POSITION, position.x, position.y, position.z);
}

void SourceHandle::setPitch(const ALfloat pitch_value)
{
  // Clamp pitch value according to specs
  set(AL_PITCH, std::clamp(pitch_value, 0.5f, 2.0f));
}

StreamingSourceHandle::~StreamingSourceHandle()
{
  gracefullyStop(std::chrono::milliseconds{10});
}

// NOLINTNEXTLINE(readability-make-member-function-const)
std::shared_ptr<BufferHandle> StreamingSourceHandle::unqueueBuffer()
{
  std::unique_lock lock{m_queueMutex};

  ALuint unqueued;
  AL_ASSERT(alSourceUnqueueBuffers(*this, 1, &unqueued));

  auto it = std::find_if(m_queuedBuffers.begin(),
                         m_queuedBuffers.end(),
                         [unqueued](const std::shared_ptr<BufferHandle>& buffer) { return *buffer == unqueued; });

  if(it == m_queuedBuffers.end())
    BOOST_THROW_EXCEPTION(std::runtime_error("Unqueued buffer not in queue"));
  auto result = *it;
  m_queuedBuffers.erase(it);
  return result;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void StreamingSourceHandle::queueBuffer(const std::shared_ptr<BufferHandle>& buffer)
{
  std::unique_lock lock{m_queueMutex};

  if(!m_queuedBuffers.emplace(buffer).second)
    BOOST_THROW_EXCEPTION(std::runtime_error("Buffer enqueued more than once"));

  ALuint bufferId = *buffer;
  AL_ASSERT(alSourceQueueBuffers(*this, 1, &bufferId));
}

bool StreamingSourceHandle::isStopped() const
{
  std::unique_lock lock{m_queueMutex};
  return m_queuedBuffers.empty() && SourceHandle::isStopped();
}

void StreamingSourceHandle::gracefullyStop(const std::chrono::milliseconds& sleep)
{
  stop();

  while(!isStopped())
  {
    std::this_thread::sleep_for(sleep);
  }
}

void StreamingSourceHandle::stop()
{
  SourceHandle::stop();

  std::unique_lock lock{m_queueMutex};
  AL_ASSERT(alSourcei(*this, AL_BUFFER, AL_NONE));
  m_queuedBuffers.clear();
}

ALint StreamingSourceHandle::getBuffersProcessed() const
{
  ALint processed = 0;
  AL_ASSERT(alGetSourcei(*this, AL_BUFFERS_PROCESSED, &processed));
  return processed;
}
} // namespace audio
