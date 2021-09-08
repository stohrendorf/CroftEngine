#pragma once

#include "core.h"
#include "sndfile/helpers.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <gsl/gsl-lite.hpp>
#include <sndfile.h>

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

  virtual size_t readStereo(int16_t* buffer, size_t bufferSize, bool looping) = 0;

  [[nodiscard]] virtual int getSampleRate() const = 0;

  [[nodiscard]] virtual std::chrono::milliseconds getPosition() const = 0;
  virtual void seek(const std::chrono::milliseconds& position) = 0;

  [[nodiscard]] virtual Clock::duration getDuration() const = 0;

protected:
  explicit AbstractStreamSource() = default;
};

class WadStreamSource final : public AbstractStreamSource
{
public:
  WadStreamSource(const std::filesystem::path& filename, size_t trackIndex);

  size_t readStereo(int16_t* frameBuffer, size_t frameCount, bool looping) override;

  [[nodiscard]] int getSampleRate() const override;

  [[nodiscard]] std::chrono::milliseconds getPosition() const override;
  void seek(const std::chrono::milliseconds& position) override;

  [[nodiscard]] Clock::duration getDuration() const override;

private:
  std::ifstream m_wadFile;
  SF_INFO m_sfInfo{};
  SNDFILE* m_sndFile = nullptr;
  std::unique_ptr<sndfile::InputStreamViewWrapper> m_wrapper;

  // CDAUDIO.WAD step size defines CDAUDIO's header stride, on which each track
  // info is placed. Also CDAUDIO count specifies static amount of tracks existing
  // in CDAUDIO.WAD file. Name length specifies maximum string size for trackname.
  static constexpr size_t WADStride = 268;
  static constexpr size_t WADNameLength = 260;
  static constexpr size_t WADCount = 130;
};

class SndfileStreamSource final : public AbstractStreamSource
{
private:
  SF_INFO m_sfInfo{};
  SNDFILE* m_sndFile = nullptr;

public:
  explicit SndfileStreamSource(const std::filesystem::path& filename);

  size_t readStereo(int16_t* frameBuffer, size_t frameCount, bool looping) override;

  [[nodiscard]] int getSampleRate() const override;

  [[nodiscard]] std::chrono::milliseconds getPosition() const override;
  void seek(const std::chrono::milliseconds& position) override;

  [[nodiscard]] Clock::duration getDuration() const override;
};
} // namespace audio
