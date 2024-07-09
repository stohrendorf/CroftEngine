#pragma once

#include "audio/core.h"
#include "ffmpeg/avframeptr.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

namespace ffmpeg
{
struct Stream;
}

namespace audio
{
struct AudioStreamDecoder final
{
  static constexpr size_t QueueLimit = 60;

  mutable std::mutex mutex;

  AVFormatContext* fmtContext = nullptr;
  ffmpeg::AVFramePtr audioFrame;
  std::unique_ptr<ffmpeg::Stream> stream;
  SwrContext* swrContext = nullptr;
  std::queue<std::vector<int16_t>> queue;
  int64_t lastPacketPts = 0;

  explicit AudioStreamDecoder(AVFormatContext* fmtContext, bool rplFakeAudioHack);
  ~AudioStreamDecoder();

  [[nodiscard]] bool empty() const
  {
    const std::unique_lock lock{mutex};
    return queue.empty();
  }

  [[nodiscard]] bool filled() const
  {
    const std::unique_lock lock{mutex};
    return queue.size() >= QueueLimit;
  }

  bool push(const AVPacket& packet);

  size_t read(int16_t* buffer, size_t bufferSize);

  [[nodiscard]] int getSampleRate() const noexcept;

  [[nodiscard]] std::chrono::milliseconds getPosition() const;

  void seek(const std::chrono::milliseconds& position);

  [[nodiscard]] audio::Clock::duration getDuration() const;

  [[nodiscard]] int getChannels() const noexcept;
};
} // namespace audio
