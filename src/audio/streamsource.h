#pragma once

#include "sndfile/helpers.h"

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <filesystem>
#include <fstream>
#include <gsl-lite.hpp>
#include <sndfile.h>
#include <soloud_audiosource.h>

namespace audio
{
namespace sndfile
{
inline size_t readStereo(float* sampleBuffer, const size_t frameCount, SNDFILE* sndFile, const int channels)
{
  size_t processedFrames = 0;
  std::vector<float> interleaved;
  while(processedFrames < frameCount)
  {
    const auto requestedFrames = frameCount - processedFrames;
    interleaved.resize(requestedFrames * channels);
    const auto readFrames = sf_readf_float(sndFile, interleaved.data(), requestedFrames);

    auto srcPtr = interleaved.cbegin();
    for(sf_count_t i = 0; i < readFrames; ++i)
    {
      for(int c = 0; c < channels; ++c)
      {
        // deinterlace
        BOOST_ASSERT(srcPtr != interleaved.cend());
        sampleBuffer[c * frameCount] = *srcPtr++;
      }
      ++processedFrames;
      ++sampleBuffer;
    }

    if(readFrames <= 0)
      break;
  }

  for(int c = 0; c < channels; ++c)
  {
    std::fill_n(&sampleBuffer[c * frameCount], frameCount - processedFrames, 0.0f);
  }

  return processedFrames;
}
} // namespace sndfile

class WadStreamInstance final : public SoLoud::AudioSourceInstance
{
private:
  std::ifstream m_wadFile;
  SF_INFO m_sfInfo{};
  SNDFILE* m_sndFile = nullptr;
  std::unique_ptr<sndfile::InputStreamViewWrapper> m_wrapper;
  bool m_hasEnded{false};

  // CDAUDIO.WAD step size defines CDAUDIO's header stride, on which each track
  // info is placed. Also CDAUDIO count specifies static amount of tracks existing
  // in CDAUDIO.WAD file. Name length specifies maximum string size for trackname.
  static constexpr size_t WADStride = 268;
  static constexpr size_t WADNameLength = 260;
  static constexpr size_t WADCount = 130;

public:
  WadStreamInstance(const std::filesystem::path& filename, const size_t trackIndex)
      : m_wadFile{filename, std::ios::in | std::ios::binary}
  {
    BOOST_LOG_TRIVIAL(trace) << "Creating WAD stream source from " << filename << ", track " << trackIndex;

    memset(&m_sfInfo, 0, sizeof(m_sfInfo));

    if(!m_wadFile.is_open())
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open WAD file"));

    m_wadFile.seekg(trackIndex * WADStride, std::ios::beg);

    std::array<char, WADNameLength> trackName{};
    m_wadFile.read(trackName.data(), WADNameLength);

    BOOST_LOG_TRIVIAL(info) << "Loading WAD track " << trackIndex << ": " << trackName.data();

    uint32_t offset = 0;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    m_wadFile.read(reinterpret_cast<char*>(&offset), 4);

    uint32_t length = 0;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    m_wadFile.read(reinterpret_cast<char*>(&length), 4);

    m_wadFile.seekg(offset, std::ios::beg);
    m_wrapper = std::make_unique<sndfile::InputStreamViewWrapper>(m_wadFile, offset, offset + length);

    m_sndFile = sf_open_virtual(m_wrapper.get(), SFM_READ, &m_sfInfo, m_wrapper.get());
    if(m_sndFile == nullptr)
    {
      BOOST_LOG_TRIVIAL(error) << "Failed to open WAD file: " << sf_strerror(nullptr);
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open WAD file"));
    }

    mChannels = m_sfInfo.channels;
    mBaseSamplerate = m_sfInfo.samplerate;
  }

  unsigned int getAudio(float* aBuffer, unsigned int aSamplesToRead, unsigned int /*aBufferSize*/) override
  {
    auto tmp = sndfile::readStereo(aBuffer, aSamplesToRead, m_sndFile, m_sfInfo.channels == 1);
    if(tmp <= 0)
      m_hasEnded = true;
    return tmp;
  }

  bool hasEnded() override
  {
    return m_hasEnded;
  }

  SoLoud::result seek(SoLoud::time aSeconds, float* /*mScratch*/, unsigned int /*mScratchSize*/) override
  {
    sf_seek(m_sndFile, m_sfInfo.samplerate * aSeconds, SF_SEEK_SET);
    return SoLoud::SO_NO_ERROR;
  }

  SoLoud::result rewind() override
  {
    sf_seek(m_sndFile, 0, SF_SEEK_SET);
    return SoLoud::SO_NO_ERROR;
  }
};

class WadStream final : public SoLoud::AudioSource
{
public:
  WadStream(const std::filesystem::path& filename, const size_t trackIndex)
      : m_filename{filename}
      , m_trackIndex{trackIndex}
  {
  }

  SoLoud::AudioSourceInstance* createInstance() override
  {
    return new WadStreamInstance{m_filename, m_trackIndex};
  }

private:
  const std::filesystem::path m_filename;
  const size_t m_trackIndex;
};
} // namespace audio
