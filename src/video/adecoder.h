#pragma once

#include "audio/core.h"
#include "audio/streamsource.h"
#include "avframeptr.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <queue>
#include <vector>

extern "C"
{
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

namespace video
{
struct Stream;

struct ADecoder final : public audio::AbstractStreamSource
{
  static constexpr size_t QueueLimit = 60;

  AVFormatContext* fmtContext = nullptr;
  AVFramePtr audioFrame;
  std::unique_ptr<Stream> audioStream;
  SwrContext* swrContext = nullptr;
  std::queue<std::vector<int16_t>> audioQueue;

  explicit ADecoder(const std::filesystem::path& filename);
  ~ADecoder() override;

  AVPacket packet{};

  void fillQueues(bool looping);

  void decodePacket();

  size_t readStereo(int16_t* buffer, size_t bufferSize, bool /*looping*/) override;

  [[nodiscard]] int getSampleRate() const override;

  [[nodiscard]] std::chrono::milliseconds getPosition() const override;

  void seek(const std::chrono::milliseconds& position) override;

  [[nodiscard]] audio::Clock::duration getDuration() const override;
};
} // namespace video