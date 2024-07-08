#pragma once

#include "core.h"
#include "voice.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <vector>

namespace audio
{
class StreamingSourceHandle;
class BufferHandle;
class AbstractStreamSource;
class SourceHandle;

class StreamVoice : public Voice
{
public:
  explicit StreamVoice(std::unique_ptr<StreamingSourceHandle>&& streamSource,
                       std::unique_ptr<AbstractStreamSource>&& source,
                       size_t bufferSize,
                       size_t bufferCount,
                       const std::chrono::milliseconds& initialPosition = std::chrono::milliseconds{0});

  ~StreamVoice() override;

  void update();

  void setLooping(const bool looping) override
  {
    m_looping = looping;
  }

  [[nodiscard]] auto isLooping() const
  {
    return m_looping;
  }

  [[nodiscard]] std::chrono::milliseconds getStreamPosition() const;
  void seek(const std::chrono::milliseconds& position);

  [[noreturn]] void associate(std::unique_ptr<SourceHandle>&& source) override;

  [[nodiscard]] Clock::duration getDuration() const override;

private:
  gsl::not_null<StreamingSourceHandle*> m_streamSource;
  std::unique_ptr<AbstractStreamSource> m_stream;
  std::vector<gslu::nn_shared<BufferHandle>> m_buffers;
  std::vector<int16_t> m_sampleBuffer;
  bool m_looping = false;

  bool fillBuffer(BufferHandle& buffer);
};
} // namespace audio
