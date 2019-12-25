#pragma once

#include "sndfile/helpers.h"

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <filesystem>
#include <fstream>
#include <gsl-lite.hpp>
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

  virtual int getSampleRate() const = 0;

protected:
  explicit AbstractStreamSource() = default;
};

namespace sndfile
{
inline short clampSample(float v) noexcept
{
  if(v <= -1)
    return std::numeric_limits<short>::lowest();
  else if(v >= 1)
    return std::numeric_limits<short>::max();
  else
    return static_cast<short>(std::numeric_limits<short>::max() * v);
}

inline size_t readStereo(
  short* sampleBuffer, const size_t frameCount, SNDFILE* sndFile, const bool sourceIsMono, const bool looping)
{
  size_t processedFrames = 0;
  std::vector<float> tmp;
  const auto samplesPerFrame = sourceIsMono ? 1 : 2;
  while(processedFrames < frameCount)
  {
    const auto requestedFrames = frameCount - processedFrames;
    const auto requestedSamples = requestedFrames * samplesPerFrame;
    tmp.resize(requestedSamples);
    const auto readFrames = sf_readf_float(sndFile, tmp.data(), requestedFrames);
    if(readFrames > 0)
    {
      for(size_t i = 0; i < static_cast<size_t>(readFrames * samplesPerFrame); ++i)
        sampleBuffer[processedFrames * samplesPerFrame + i] = clampSample(tmp[i]);

      processedFrames += readFrames;
    }
    else
    {
      if(!looping)
      {
        std::fill_n(sampleBuffer + processedFrames * samplesPerFrame, requestedSamples, 0);
        break;
      }

      // restart if there are not enough samples
      sf_seek(sndFile, 0, SEEK_SET);
    }
  }

  if(sourceIsMono)
  {
    // Need to duplicate the samples from mono to stereo
    for(size_t i = 0; i < frameCount; ++i)
    {
      const size_t src = frameCount - i - 1;
      const size_t dest = 2 * frameCount - i - 2;
      sampleBuffer[dest] = sampleBuffer[dest + 1] = sampleBuffer[src];
    }
  }

  return processedFrames;
}
} // namespace sndfile

class WadStreamSource final : public AbstractStreamSource
{
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

public:
  WadStreamSource(const std::filesystem::path& filename, const size_t trackIndex)
      : m_wadFile{filename, std::ios::in | std::ios::binary}
  {
    BOOST_LOG_TRIVIAL(trace) << "Creating WAD stream source from " << filename << ", track " << trackIndex;

    memset(&m_sfInfo, 0, sizeof(m_sfInfo));

    if(!m_wadFile.is_open())
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open WAD file"));

    m_wadFile.seekg(trackIndex * WADStride, std::ios::beg);

    char trackName[WADNameLength];
    m_wadFile.read(trackName, WADNameLength);

    BOOST_LOG_TRIVIAL(info) << "Loading WAD track " << trackIndex << ": " << trackName;

    uint32_t offset = 0;
    m_wadFile.read(reinterpret_cast<char*>(&offset), 4);

    uint32_t length = 0;
    m_wadFile.read(reinterpret_cast<char*>(&length), 4);

    m_wadFile.seekg(offset, std::ios::beg);
    m_wrapper = std::make_unique<sndfile::InputStreamViewWrapper>(m_wadFile, offset, offset + length);

    m_sndFile = sf_open_virtual(m_wrapper.get(), SFM_READ, &m_sfInfo, m_wrapper.get());
    if(m_sndFile == nullptr)
    {
      BOOST_LOG_TRIVIAL(error) << "Failed to open WAD file: " << sf_strerror(nullptr);
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open WAD file"));
    }
  }

  size_t readStereo(int16_t* frameBuffer, const size_t frameCount, const bool looping) override
  {
    return sndfile::readStereo(frameBuffer, frameCount, m_sndFile, m_sfInfo.channels == 1, looping);
  }

  int getSampleRate() const override
  {
    return m_sfInfo.samplerate;
  }
};

class SndfileStreamSource final : public AbstractStreamSource
{
private:
  SF_INFO m_sfInfo{};
  SNDFILE* m_sndFile = nullptr;

public:
  explicit SndfileStreamSource(const std::filesystem::path& filename)
  {
    BOOST_LOG_TRIVIAL(trace) << "Creating sndfile stream source from " << filename;

    memset(&m_sfInfo, 0, sizeof(m_sfInfo));

    m_sndFile = sf_open(filename.string().c_str(), SFM_READ, &m_sfInfo);
    if(m_sndFile == nullptr)
    {
      BOOST_LOG_TRIVIAL(error) << "Failed to open audio file: " << sf_strerror(nullptr);
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open audio file"));
    }
  }

  size_t readStereo(int16_t* frameBuffer, const size_t frameCount, const bool looping) override
  {
    return sndfile::readStereo(frameBuffer, frameCount, m_sndFile, m_sfInfo.channels == 1, looping);
  }

  int getSampleRate() const override
  {
    return m_sfInfo.samplerate;
  }
};
} // namespace audio
