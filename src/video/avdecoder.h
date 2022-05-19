#pragma once

#include "audio/core.h"
#include "audio/streamsource.h"
#include "avframeptr.h"
#include "filtergraph.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>

extern "C"
{
#include <libavformat/avformat.h>
}

namespace video
{
struct Stream;
struct AudioStreamDecoder;

struct AVDecoder final : public audio::AbstractStreamSource
{
  AVFormatContext* fmtContext = nullptr;
  std::unique_ptr<AudioStreamDecoder> audioDecoder;
  std::unique_ptr<Stream> videoStream;
  FilterGraph filterGraph;

  explicit AVDecoder(const std::string& filename);
  ~AVDecoder() override;

  bool stopped = false;
  AVPacket packet{};

  void fillQueues();

  std::queue<AVFramePtr> imgQueue;
  mutable std::mutex imgQueueMutex;
  std::condition_variable frameReadyCondition;
  bool frameReady = false;

  std::optional<AVFramePtr> takeFrame();

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

private:
  [[nodiscard]] double getVideoTs(bool lock);
};
} // namespace video
