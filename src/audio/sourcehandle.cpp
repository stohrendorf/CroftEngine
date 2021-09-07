#include "sourcehandle.h"

#include "bufferhandle.h"
#include "core/magic.h"
#include "filterhandle.h"
#include "util/helpers.h"

#include <boost/log/trivial.hpp>
#include <thread>

namespace audio
{
namespace
{
[[nodiscard]] ALuint createHandle()
{
  ALuint handle;
  AL_ASSERT(alGenSources(1, &handle));

  Expects(alIsSource(handle));

  return handle;
}
} // namespace

SourceHandle::SourceHandle()
    : m_handle{createHandle()}
{
  set(AL_REFERENCE_DISTANCE, core::SectorSize.get());
}

SourceHandle::~SourceHandle()
{
  AL_ASSERT(alSourceStop(m_handle));
  AL_ASSERT(alDeleteSources(1, &m_handle));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::setDirectFilter(const std::shared_ptr<FilterHandle>& f)
{
  AL_ASSERT(alSourcei(m_handle, AL_DIRECT_FILTER, f ? f->get() : AL_FILTER_NULL));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::set(const ALenum e, const ALint v)
{
  AL_ASSERT(alSourcei(m_handle, e, v));
}

auto SourceHandle::geti(ALenum e) const
{
  ALint value{};
  AL_ASSERT(alGetSourcei(m_handle, e, &value));
  return value;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::set(const ALenum e, const ALint* v)
{
  AL_ASSERT(alSourceiv(m_handle, e, v));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::set(const ALenum e, const ALfloat v)
{
  AL_ASSERT(alSourcef(m_handle, e, v));
}

auto SourceHandle::getf(ALenum e) const
{
  ALfloat value{};
  AL_ASSERT(alGetSourcef(m_handle, e, &value));
  return value;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::set(const ALenum e, const ALfloat a, const ALfloat b, const ALfloat c)
{
  AL_ASSERT(alSource3f(m_handle, e, a, b, c));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::set(const ALenum e, const ALfloat* v)
{
  AL_ASSERT(alSourcefv(m_handle, e, v));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::play()
{
  AL_ASSERT(alSourcePlay(m_handle));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::pause()
{
  AL_ASSERT(alSourcePause(m_handle));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::rewind()
{
  AL_ASSERT(alSourceRewind(m_handle));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void SourceHandle::stop()
{
  AL_ASSERT(alSourceStop(m_handle));
}

bool SourceHandle::isStopped() const
{
  ALenum state = AL_STOPPED;
  AL_ASSERT(alGetSourcei(m_handle, AL_SOURCE_STATE, &state));

  return state == AL_STOPPED;
}

bool SourceHandle::isPaused() const
{
  ALenum state = AL_STOPPED;
  AL_ASSERT(alGetSourcei(m_handle, AL_SOURCE_STATE, &state));

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

ALint SourceHandle::getBuffersProcessed() const
{
  ALint processed = 0;
  AL_ASSERT(alGetSourcei(m_handle, AL_BUFFERS_PROCESSED, &processed));
  return processed;
}

StreamingSourceHandle::~StreamingSourceHandle()
{
  std::unique_lock lock{m_queueMutex};
  if(!m_queuedBuffers.empty())
  {
    lock.unlock();
    BOOST_LOG_TRIVIAL(warning) << "Streaming source handle still processing on destruction";
    gracefullyStop(std::chrono::milliseconds{10});
  }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
std::shared_ptr<BufferHandle> StreamingSourceHandle::unqueueBuffer()
{
  std::unique_lock lock{m_queueMutex};

  ALuint unqueued;
  AL_ASSERT(alSourceUnqueueBuffers(get(), 1, &unqueued));

  auto it = std::find_if(m_queuedBuffers.begin(),
                         m_queuedBuffers.end(),
                         [unqueued](const std::shared_ptr<BufferHandle>& buffer) { return buffer->get() == unqueued; });

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

  ALuint bufferId = buffer->get();
  AL_ASSERT(alSourceQueueBuffers(get(), 1, &bufferId));
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
  while(!m_queuedBuffers.empty())
  {
    lock.unlock();
    (void)unqueueBuffer();
    lock.lock();
  }
}
} // namespace audio
