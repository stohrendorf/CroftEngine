#pragma once

#include "core.h"
#include "voice.h"

#include <gslu.h>
#include <memory>

namespace audio
{
class BufferHandle;
class SourceHandle;

class BufferVoice final : public Voice
{
private:
  gslu::nn_shared<BufferHandle> m_buffer;

public:
  explicit BufferVoice(gslu::nn_shared<BufferHandle> buffer);

  ~BufferVoice() override;

  void associate(std::unique_ptr<SourceHandle>&& source) override;

  [[nodiscard]] Clock::duration getDuration() const override;
};
} // namespace audio
