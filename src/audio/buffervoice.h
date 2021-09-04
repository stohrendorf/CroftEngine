#pragma once

#include "voice.h"

namespace audio
{
class BufferVoice : public Voice
{
private:
  gsl::not_null<std::shared_ptr<BufferHandle>> m_buffer;

public:
  explicit BufferVoice(gsl::not_null<std::shared_ptr<SourceHandle>> source,
                       gsl::not_null<std::shared_ptr<BufferHandle>> buffer)
      : Voice{std::move(source)}
      , m_buffer{std::move(buffer)}
  {
    AL_ASSERT(alSourcei(getSource()->get(), AL_BUFFER, m_buffer->get()));
  }
};
} // namespace audio
