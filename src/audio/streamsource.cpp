#include "streamsource.h"

#include "core.h"
#include "sndfile/helpers.h"

#include <algorithm>
#include <array>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cstdio>
#include <cstring>
#include <gsl/gsl-lite.hpp>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace audio
{
namespace
{
size_t readStereo(
  short* sampleBuffer, const size_t frameCount, SNDFILE* sndFile, const bool sourceIsMono, const bool looping)
{
  size_t processedFrames = 0;
  std::vector<int16_t> tmp;
  const auto samplesPerFrame = sourceIsMono ? 1 : 2;
  while(processedFrames < frameCount)
  {
    const auto requestedFrames = frameCount - processedFrames;
    const auto requestedSamples = requestedFrames * samplesPerFrame;
    tmp.resize(requestedSamples);
    const auto readFrames = sf_readf_short(sndFile, tmp.data(), gsl::narrow<sf_count_t>(requestedFrames));
    if(readFrames > 0)
    {
      std::copy_n(tmp.begin(),
                  static_cast<size_t>(readFrames * samplesPerFrame),
                  &sampleBuffer[processedFrames * samplesPerFrame]);

      processedFrames += readFrames;
    }
    else
    {
      if(!looping)
      {
        std::fill_n(sampleBuffer + processedFrames * samplesPerFrame, requestedSamples, int16_t{0});
        break;
      }

      // restart if there are not enough samples
      Expects(sf_seek(sndFile, 0, SEEK_SET) != -1);
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
} // namespace

WadStreamSource::WadStreamSource(const std::filesystem::path& filename, const size_t trackIndex)
    : m_wadFile{filename, std::ios::in | std::ios::binary}
{
  BOOST_LOG_TRIVIAL(trace) << "Creating WAD stream source from " << filename << ", track " << trackIndex;

  memset(&m_sfInfo, 0, sizeof(m_sfInfo));

  if(!m_wadFile.is_open())
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open WAD file"));

  m_wadFile.seekg(gsl::narrow<std::ifstream::off_type>(trackIndex * WADStride), std::ios::beg);

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
}

size_t WadStreamSource::readStereo(int16_t* frameBuffer, const size_t frameCount, const bool looping)
{
  return audio::readStereo(frameBuffer, frameCount, m_sndFile, m_sfInfo.channels == 1, looping);
}

int WadStreamSource::getSampleRate() const
{
  return m_sfInfo.samplerate;
}

std::chrono::milliseconds WadStreamSource::getPosition() const
{
  const auto frames = sf_seek(m_sndFile, 0, SF_SEEK_CUR);
  Expects(frames != -1);
  return std::chrono::milliseconds{frames * 1000 / m_sfInfo.samplerate};
}

void WadStreamSource::seek(const std::chrono::milliseconds& position)
{
  const auto frames = position.count() * m_sfInfo.samplerate / 1000;
  Expects(sf_seek(m_sndFile, frames, SF_SEEK_SET) != -1);
}

Clock::duration WadStreamSource::getDuration() const
{
  return Clock::duration{(m_sfInfo.frames * Clock::duration::period::den)
                         / (m_sfInfo.samplerate * Clock::duration::period::num)};
}

SndfileStreamSource::SndfileStreamSource(const std::filesystem::path& filename)
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

size_t SndfileStreamSource::readStereo(int16_t* frameBuffer, const size_t frameCount, const bool looping)
{
  return audio::readStereo(frameBuffer, frameCount, m_sndFile, m_sfInfo.channels == 1, looping);
}

int SndfileStreamSource::getSampleRate() const
{
  return m_sfInfo.samplerate;
}

std::chrono::milliseconds SndfileStreamSource::getPosition() const
{
  const auto frames = sf_seek(m_sndFile, 0, SF_SEEK_CUR);
  Expects(frames != -1);
  return std::chrono::milliseconds{frames * 1000 / m_sfInfo.samplerate};
}

void SndfileStreamSource::seek(const std::chrono::milliseconds& position)
{
  const auto frames = position.count() * m_sfInfo.samplerate / 1000;
  Expects(sf_seek(m_sndFile, frames, SF_SEEK_SET) != -1);
}

Clock::duration SndfileStreamSource::getDuration() const
{
  return Clock::duration{(m_sfInfo.frames * Clock::duration::period::den)
                         / (m_sfInfo.samplerate * Clock::duration::period::num)};
}
} // namespace audio
