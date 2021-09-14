#pragma once

#include "core.h"
#include "voice.h"

#include <gsl/gsl-lite.hpp>
#include <memory>

namespace audio
{
class BufferHandle;
class SourceHandle;

class BufferVoice final : public Voice
{
private:
  gsl::not_null<std::shared_ptr<BufferHandle>> m_buffer;

public:
  explicit BufferVoice(gsl::not_null<std::shared_ptr<BufferHandle>> buffer);

  ~BufferVoice() override;

  void associate(std::unique_ptr<SourceHandle>&& source) override;

  [[nodiscard]] Clock::duration getDuration() const override;
};
} // namespace audio
