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
class AudioStreamDecoder final
{
public:
  explicit AudioStreamDecoder(AVFormatContext* fmtContext, bool rplFakeAudioHack);
  ~AudioStreamDecoder();

  [[nodiscard]] bool empty() const
  {
    const std::unique_lock lock{m_mutex};
    return m_queue.empty();
  }

  [[nodiscard]] bool filled() const
  {
    const std::unique_lock lock{m_mutex};
    return m_queue.size() >= QueueLimit;
  }

  bool push(const AVPacket& packet);

  size_t read(int16_t* buffer, size_t bufferSize);

  [[nodiscard]] int getSampleRate() const noexcept;

  [[nodiscard]] std::chrono::milliseconds getPosition() const;

  void seek(const std::chrono::milliseconds& position);

  [[nodiscard]] audio::Clock::duration getDuration() const;

  [[nodiscard]] int getChannels() const noexcept;

  [[nodiscard]] const auto& getStream() const
  {
    return m_stream;
  }

private:
  static constexpr size_t QueueLimit = 60;

  mutable std::mutex m_mutex;

  AVFormatContext* m_fmtContext = nullptr;
  ffmpeg::AVFramePtr m_audioFrame;
  std::unique_ptr<ffmpeg::Stream> m_stream;
  SwrContext* m_swrContext = nullptr;
  std::queue<std::vector<int16_t>> m_queue;
  int64_t m_lastPacketPts = 0;
};
} // namespace audio
