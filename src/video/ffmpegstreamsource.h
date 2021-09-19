#pragma once

#include "audio/core.h"
#include "audio/streamsource.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>

extern "C"
{
#include <libavformat/avformat.h>
}

namespace video
{
struct AudioStreamDecoder;

struct FfmpegStreamSource final : public audio::AbstractStreamSource
{
  AVFormatContext* fmtContext = nullptr;
  std::unique_ptr<AudioStreamDecoder> decoder;
  AVPacket packet{};

  explicit FfmpegStreamSource(const std::filesystem::path& filename);
  ~FfmpegStreamSource() override;

  void fillQueues(bool looping);

  size_t readStereo(int16_t* buffer, size_t bufferSize, bool looping) override;

  [[nodiscard]] int getSampleRate() const override;

  [[nodiscard]] std::chrono::milliseconds getPosition() const override;

  void seek(const std::chrono::milliseconds& position) override;

  [[nodiscard]] audio::Clock::duration getDuration() const override;
};
} // namespace video
