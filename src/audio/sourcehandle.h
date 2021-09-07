#pragma once

#include <AL/al.h>
#include <glm/vec3.hpp>
#include <mutex>
#include <unordered_set>

namespace audio
{
class BufferHandle;
class FilterHandle;

class SourceHandle
{
public:
  explicit SourceHandle(const SourceHandle&) = delete;
  explicit SourceHandle(SourceHandle&&) = delete;
  SourceHandle& operator=(const SourceHandle&) = delete;
  SourceHandle& operator=(SourceHandle&&) = delete;

  explicit SourceHandle();

  virtual ~SourceHandle();

  [[nodiscard]] ALuint get() const noexcept
  {
    return m_handle;
  }

  void setDirectFilter(const std::shared_ptr<FilterHandle>& f);

  void set(ALenum e, ALint v);

  [[nodiscard]] auto geti(ALenum e) const;

  void set(ALenum e, const ALint* v);

  void set(ALenum e, ALfloat v);

  [[nodiscard]] auto getf(ALenum e) const;

  void set(ALenum e, ALfloat a, ALfloat b, ALfloat c);

  void set(ALenum e, const ALfloat* v);

  void play();

  void pause();

  void rewind();

  virtual void stop();

  [[nodiscard]] virtual bool isStopped() const;

  [[nodiscard]] bool isPaused() const;

  void setLooping(bool isLooping);

  void setGain(ALfloat gain);

  void setPosition(const glm::vec3& position);

  void setPitch(ALfloat pitch_value);

  [[nodiscard]] ALint getBuffersProcessed() const;

private:
  const ALuint m_handle{};
};

class StreamingSourceHandle : public SourceHandle
{
private:
  mutable std::mutex m_queueMutex{};
  std::unordered_set<std::shared_ptr<BufferHandle>> m_queuedBuffers{};

public:
  ~StreamingSourceHandle() override;

  [[nodiscard]] std::shared_ptr<BufferHandle> unqueueBuffer();

  void queueBuffer(const std::shared_ptr<BufferHandle>& buffer);

  [[nodiscard]] bool isStopped() const override;

  void gracefullyStop(const std::chrono::milliseconds& sleep);

  void stop() override;
};
} // namespace audio
