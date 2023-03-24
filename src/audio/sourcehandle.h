#pragma once

#include "handle.h"

#include <chrono>
#include <glm/vec3.hpp>
#include <memory>
#include <mutex>
#include <unordered_set>

namespace audio
{
class BufferHandle;
class FilterHandle;

class SourceHandle : public Handle
{
public:
  explicit SourceHandle(bool positional);
  ~SourceHandle() override;

  void setDirectFilter(const std::shared_ptr<FilterHandle>& f);

  void set(ALenum e, ALint v);
  void set(ALenum e, ALfloat v);
  void set(ALenum e, ALfloat a, ALfloat b, ALfloat c);

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
};

class StreamingSourceHandle final : public SourceHandle
{
public:
  explicit StreamingSourceHandle()
      : SourceHandle{false}
  {
  }

  ~StreamingSourceHandle() override;

  [[nodiscard]] std::shared_ptr<BufferHandle> unqueueBuffer();

  void queueBuffer(const std::shared_ptr<BufferHandle>& buffer);

  [[nodiscard]] bool isStopped() const override;

  void gracefullyStop(const std::chrono::milliseconds& sleep);
  void stop() final;

  [[nodiscard]] ALint getBuffersProcessed() const;

private:
  mutable std::mutex m_queueMutex{};
  std::unordered_set<std::shared_ptr<BufferHandle>> m_queuedBuffers{};
};
} // namespace audio
