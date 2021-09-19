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
#include <vector>

extern "C"
{
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

namespace video
{
struct Stream;

struct AVDecoder final : public audio::AbstractStreamSource
{
  AVFormatContext* fmtContext = nullptr;
  AVFramePtr audioFrame;
  std::unique_ptr<Stream> audioStream;
  std::unique_ptr<Stream> videoStream;
  SwrContext* swrContext = nullptr;
  FilterGraph filterGraph;

  explicit AVDecoder(const std::string& filename);
  ~AVDecoder() override;

  bool stopped = false;
  AVPacket packet{};

  void fillQueues();

  std::queue<AVFramePtr> imgQueue;
  std::queue<std::vector<int16_t>> audioQueue;
  mutable std::mutex imgQueueMutex;
  std::condition_variable frameReadyCondition;
  bool frameReady = false;

  std::optional<AVFramePtr> takeFrame();

  static constexpr size_t QueueLimit = 60;

  void decodeVideoPacket();

  void decodeAudioPacket();

  void decodePacket();

  size_t readStereo(int16_t* buffer, size_t bufferSize, bool /*looping*/) override;

  [[nodiscard]] int getSampleRate() const override;

  size_t audioFrameSize = 0;
  size_t audioFrameOffset = 0;
  std::atomic<size_t> totalAudioFrames = 0;

  [[nodiscard]] std::chrono::milliseconds getPosition() const override
  {
    return std::chrono::milliseconds{0};
  }

  void seek(const std::chrono::milliseconds& /*position*/) override
  {
  }

  [[nodiscard]] audio::Clock::duration getDuration() const override;
};
} // namespace video
