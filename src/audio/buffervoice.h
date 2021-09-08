#pragma once

#include "bufferhandle.h"
#include "voice.h"

namespace audio
{
class BufferVoice : public Voice
{
private:
  gsl::not_null<std::shared_ptr<BufferHandle>> m_buffer;

public:
  explicit BufferVoice(gsl::not_null<std::shared_ptr<BufferHandle>> buffer)
      : Voice{}
      , m_buffer{std::move(buffer)}
  {
  }

  void associate(std::unique_ptr<SourceHandle>&& source) override
  {
    if(source != nullptr)
      AL_ASSERT(alSourcei(source->get(), AL_BUFFER, m_buffer->get()));
    Voice::associate(std::move(source));
  }

  [[nodiscard]] Clock::duration getDuration() const override
  {
    return m_buffer->getDuration();
  }
};
} // namespace audio
