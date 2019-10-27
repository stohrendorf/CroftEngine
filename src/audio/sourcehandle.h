#pragma once

#include "bufferhandle.h"
#include "filterhandle.h"
#include "util/helpers.h"

namespace audio
{
class SourceHandle final
{
  const ALuint m_handle;
  std::shared_ptr<BufferHandle> m_buffer;

  static ALuint createHandle()
  {
    ALuint handle;
    AL_ASSERT(alGenSources(1, &handle));

    Expects(alIsSource(handle));

    return handle;
  }

public:
  explicit SourceHandle()
      : m_handle{createHandle()}
  {
    set(AL_MAX_DISTANCE, 8 * 1024);
  }

  explicit SourceHandle(const SourceHandle&) = delete;

  explicit SourceHandle(SourceHandle&&) = delete;

  SourceHandle& operator=(const SourceHandle&) = delete;

  SourceHandle& operator=(SourceHandle&&) = delete;

  ~SourceHandle()
  {
    AL_ASSERT(alSourceStop(m_handle));
    AL_ASSERT(alDeleteSources(1, &m_handle));
  }

  ALuint get() const noexcept
  {
    return m_handle;
  }

  void setBuffer(const std::shared_ptr<BufferHandle>& b)
  {
    m_buffer = b;
    AL_ASSERT(alSourcei(m_handle, AL_BUFFER, m_buffer == nullptr ? 0 : m_buffer->get()));
  }

  const std::shared_ptr<BufferHandle>& getBuffer() const noexcept
  {
    return m_buffer;
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void setDirectFilter(const std::shared_ptr<FilterHandle>& f)
  {
    AL_ASSERT(alSourcei(m_handle, AL_DIRECT_FILTER, f ? f->get() : AL_FILTER_NULL));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const ALenum e, const ALint v)
  {
    AL_ASSERT(alSourcei(m_handle, e, v));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const ALenum e, const ALint* v)
  {
    AL_ASSERT(alSourceiv(m_handle, e, v));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const ALenum e, const ALfloat v)
  {
    AL_ASSERT(alSourcef(m_handle, e, v));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const ALenum e, const ALfloat a, const ALfloat b, const ALfloat c)
  {
    AL_ASSERT(alSource3f(m_handle, e, a, b, c));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const ALenum e, const ALfloat* v)
  {
    AL_ASSERT(alSourcefv(m_handle, e, v));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void play()
  {
    AL_ASSERT(alSourcePlay(m_handle));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void pause()
  {
    AL_ASSERT(alSourcePause(m_handle));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void stop()
  {
    AL_ASSERT(alSourceStop(m_handle));
  }

  bool isStopped() const
  {
    ALenum state = AL_STOPPED;
    AL_ASSERT(alGetSourcei(m_handle, AL_SOURCE_STATE, &state));

    return state == AL_STOPPED;
  }

  bool isPaused() const
  {
    ALenum state = AL_STOPPED;
    AL_ASSERT(alGetSourcei(m_handle, AL_SOURCE_STATE, &state));

    return state == AL_PAUSED;
  }

  void setLooping(const bool isLooping)
  {
    set(AL_LOOPING, isLooping ? AL_TRUE : AL_FALSE);
  }

  void setGain(const ALfloat gain_value)
  {
    set(AL_GAIN, util::clamp(gain_value, 0.0f, 1.0f));
  }

  void setPosition(const glm::vec3& position)
  {
    set(AL_POSITION, position.x, position.y, position.z);
  }

  void setPitch(const ALfloat pitch_value)
  {
    // Clamp pitch value according to specs
    set(AL_PITCH, util::clamp(pitch_value, 0.5f, 2.0f));
  }

  ALint getBuffersProcessed() const
  {
    ALint processed = 0;
    AL_ASSERT(alGetSourcei(m_handle, AL_BUFFERS_PROCESSED, &processed));
    return processed;
  }

  ALuint unqueueBuffer()
  {
    ALuint result;
    AL_ASSERT(alSourceUnqueueBuffers(m_handle, 1, &result));
    return result;
  }

  void queueBuffer(ALuint buffer)
  {
    AL_ASSERT(alSourceQueueBuffers(m_handle, 1, &buffer));
  }

  void queueBuffer(const BufferHandle& buffer)
  {
    queueBuffer(buffer.get());
  }
};
} // namespace audio
