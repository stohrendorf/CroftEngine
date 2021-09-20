#include "bufferhandle.h"

#include "utils.h"
#include "video/ffmpegstreamsource.h"

#include <AL/al.h>
#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>
#include <cstring>
#include <gsl/gsl-lite.hpp>
#include <vector>

namespace audio
{
// NOLINTNEXTLINE(readability-make-member-function-const)
void BufferHandle::fill(const int16_t* samples, const size_t frameCount, const int sampleRate)
{
  m_frameCount = frameCount;
  m_sampleRate = sampleRate;
  AL_ASSERT(alBufferData(
    *this, AL_FORMAT_STEREO16, samples, gsl::narrow<ALsizei>(frameCount * sizeof(samples[0]) * 2), sampleRate));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void BufferHandle::fillFromWav(const uint8_t* data)
{
  Expects(data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F');
  Expects(data[8] == 'W' && data[9] == 'A' && data[10] == 'V' && data[11] == 'E');

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  uint32_t dataSize = 0;
  std::memcpy(&dataSize, data + 4, sizeof(uint32_t));
  auto tmp = std::make_unique<video::FfmpegMemoryStreamSource>(gsl::span{data, dataSize + 8});

  static constexpr size_t ChunkSize = 8192;
  std::vector<int16_t> pcm;
  while(true)
  {
    const auto offset = pcm.size();
    pcm.resize(pcm.size() + 2 * ChunkSize);
    const auto read = tmp->readStereo(&pcm[offset], ChunkSize, false);
    if(read != ChunkSize)
    {
      pcm.erase(std::next(pcm.begin(), offset + 2 * read), pcm.end());
      break;
    }
  }

  fill(pcm.data(), pcm.size() / 2, tmp->getSampleRate());
}
} // namespace audio
