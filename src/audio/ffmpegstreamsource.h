#pragma once

#include "audio/core.h"
#include "audio/streamsource.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <gsl/gsl-lite.hpp>
#include <istream>
#include <memory>
#include <utility>

extern "C"
{
#include <libavformat/avformat.h>
}

namespace audio
{
struct AudioStreamDecoder;

class BasicFfmpegStreamSource : public audio::AbstractStreamSource
{
public:
  explicit BasicFfmpegStreamSource(AVFormatContext* fmtContext, const gsl::czstring& streamName);
  ~BasicFfmpegStreamSource() override;

  void fillQueues(bool looping);

  size_t read(int16_t* buffer, size_t bufferSize, bool looping) final;
  [[nodiscard]] int getChannels() const final;

  [[nodiscard]] int getSampleRate() const final;

  [[nodiscard]] std::chrono::milliseconds getPosition() const final;

  void seek(const std::chrono::milliseconds& position) final;

  [[nodiscard]] audio::Clock::duration getDuration() const final;

protected:
  [[nodiscard]] auto getFmtContext() const
  {
    return m_fmtContext;
  }

private:
  AVFormatContext* m_fmtContext = nullptr;
  std::unique_ptr<AudioStreamDecoder> m_decoder;
  AVPacket m_packet{};
};

class FfmpegStreamSource final : public BasicFfmpegStreamSource
{
public:
  explicit FfmpegStreamSource(const std::filesystem::path& filename);
  ~FfmpegStreamSource() override = default;
};

namespace detail
{
// base-from-member
struct FfmpegMemoryStreamSourceScratchData
{
  uint8_t* dataBuffer{static_cast<uint8_t*>(av_malloc(0))};

  explicit FfmpegMemoryStreamSourceScratchData() = default;
  virtual ~FfmpegMemoryStreamSourceScratchData() = default;
};

struct FfmpegMemoryStreamSourceFileData
{
  const gsl::span<const uint8_t> data;
  size_t dataPosition = 0;

  explicit FfmpegMemoryStreamSourceFileData(const gsl::span<const uint8_t>& data)
      : data{data}
  {
  }

  virtual ~FfmpegMemoryStreamSourceFileData() = default;
};

struct FfmpegMemoryStreamSourceSubFileData
{
  std::unique_ptr<std::istream> istream;
  size_t dataStart = 0;
  size_t dataEnd = 0;
  size_t dataPosition = 0;

  explicit FfmpegMemoryStreamSourceSubFileData(std::unique_ptr<std::istream>&& istream, size_t start, size_t end)
      : istream{std::move(istream)}
      , dataStart{start}
      , dataEnd{end}
  {
    gsl_Expects(start <= end);
  }

  virtual ~FfmpegMemoryStreamSourceSubFileData() = default;
};
} // namespace detail

class FfmpegMemoryStreamSource final
    : private detail::FfmpegMemoryStreamSourceScratchData
    , private detail::FfmpegMemoryStreamSourceFileData
    , public BasicFfmpegStreamSource
{
public:
  explicit FfmpegMemoryStreamSource(const gsl::span<const uint8_t>& data);
  ~FfmpegMemoryStreamSource() override;

private:
  static int ffmpegRead(void* opaque, uint8_t* buf, int bufSize);
  static int64_t ffmpegSeek(void* opaque, int64_t offset, int whence);
};

class FfmpegSubStreamStreamSource
    : private detail::FfmpegMemoryStreamSourceScratchData
    , private detail::FfmpegMemoryStreamSourceSubFileData
    , public BasicFfmpegStreamSource
{
public:
  explicit FfmpegSubStreamStreamSource(std::unique_ptr<std::istream>&& istream, size_t start, size_t end);
  ~FfmpegSubStreamStreamSource() override;

private:
  static int ffmpegRead(void* opaque, uint8_t* buf, int bufSize);
  static int64_t ffmpegSeek(void* opaque, int64_t offset, int whence);
};
} // namespace audio
