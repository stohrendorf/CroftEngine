#pragma once

#include "audio/core.h"
#include "audio/streamsource.h"
#include "ffmpeg/avframeptr.h"
#include "filtergraph.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>

extern "C"
{
#include <libavcodec/packet.h>
}

extern "C"
{
#include <libavformat/avformat.h>
}

namespace audio
{
struct AudioStreamDecoder;
}

namespace ffmpeg
{
struct Stream;
struct AVFramePtr;
} // namespace ffmpeg

namespace video
{
struct AVDecoder final : public audio::AbstractStreamSource
{
  AVFormatContext* fmtContext = nullptr;
  std::unique_ptr<audio::AudioStreamDecoder> audioDecoder;
  std::unique_ptr<ffmpeg::Stream> videoStream;
  FilterGraph filterGraph;

  explicit AVDecoder(const std::string& filename);
  ~AVDecoder() override;

  bool stopped = false;
  AVPacket packet{};

  void fillQueues();

  std::queue<ffmpeg::AVFramePtr> imgQueue;
  mutable std::mutex imgQueueMutex;

  std::optional<ffmpeg::AVFramePtr> takeFrame();

  static constexpr size_t QueueLimit = 60;

  void decodeVideoPacket();

  size_t read(int16_t* buffer, size_t bufferSize, bool /*looping*/) override;
  int getChannels() const override;

  [[nodiscard]] int getSampleRate() const override;

  std::atomic<size_t> totalAudioFrames = 0;

  [[nodiscard]] std::chrono::milliseconds getPosition() const override
  {
    return std::chrono::milliseconds{0};
  }

  void seek(const std::chrono::milliseconds& /*position*/) override
  {
  }

  [[nodiscard]] audio::Clock::duration getDuration() const override;

  void play()
  {
    m_playStart = std::chrono::high_resolution_clock::now();
  }

private:
  [[nodiscard]] std::chrono::high_resolution_clock::time_point getVideoTs();

  std::chrono::high_resolution_clock::time_point m_playStart;
};
} // namespace video
