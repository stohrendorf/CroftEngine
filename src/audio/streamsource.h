#pragma once

#include "core.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>

namespace audio
{
class AbstractStreamSource
{
public:
  explicit AbstractStreamSource(const AbstractStreamSource&) = delete;
  explicit AbstractStreamSource(AbstractStreamSource&&) = delete;
  AbstractStreamSource& operator=(const AbstractStreamSource&) = delete;
  AbstractStreamSource& operator=(AbstractStreamSource&&) = delete;

  virtual ~AbstractStreamSource() = default;

  virtual size_t read(int16_t* buffer, size_t bufferSize, bool looping) = 0;
  [[nodiscard]] virtual int getChannels() const = 0;

  [[nodiscard]] virtual int getSampleRate() const = 0;

  [[nodiscard]] virtual std::chrono::milliseconds getPosition() const = 0;
  virtual void seek(const std::chrono::milliseconds& position) = 0;

  [[nodiscard]] virtual Clock::duration getDuration() const = 0;

protected:
  explicit AbstractStreamSource() = default;
};
} // namespace audio
