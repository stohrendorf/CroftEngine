#pragma once

#include "sourcehandle.h"
#include "streamsource.h"
#include "voice.h"

namespace audio
{
class Device;

class StreamVoice : public Voice
{
public:
  explicit StreamVoice(Device& device,
                       std::unique_ptr<AbstractStreamSource>&& source,
                       size_t bufferSize,
                       size_t bufferCount,
                       const std::chrono::milliseconds& initialPosition = std::chrono::milliseconds{0});

  void update();

  void setLooping(const bool looping)
  {
    m_looping = looping;
  }

  [[nodiscard]] std::chrono::milliseconds getPosition() const;
  void seek(const std::chrono::milliseconds& position);

private:
  std::unique_ptr<AbstractStreamSource> m_stream;
  std::vector<gsl::not_null<std::shared_ptr<BufferHandle>>> m_buffers{};
  std::vector<int16_t> m_sampleBuffer;
  bool m_looping = false;

  bool fillBuffer(BufferHandle& buffer);
};
} // namespace audio
