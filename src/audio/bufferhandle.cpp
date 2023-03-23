#include "bufferhandle.h"

#include "ffmpegstreamsource.h"
#include "utils.h"

#include <AL/al.h>
#include <cstring>
#include <gsl/gsl-lite.hpp>
#include <iterator>
#include <memory>
#include <vector>

namespace audio
{
// NOLINTNEXTLINE(readability-make-member-function-const)
void BufferHandle::fill(const int16_t* samples, const size_t frameCount, const int channels, const int sampleRate)
{
  gsl_Expects(channels == 1 || channels == 2);
  m_frameCount = frameCount;
  m_sampleRate = sampleRate;
  gsl_Assert(channels == 1 || channels == 2);
  AL_ASSERT(alBufferData(*this,
                         channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
                         samples,
                         gsl::narrow<ALsizei>(frameCount * sizeof(samples[0]) * channels),
                         sampleRate));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void BufferHandle::fillFromWav(const uint8_t* data)
{
  gsl_Expects(data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F');
  gsl_Expects(data[8] == 'W' && data[9] == 'A' && data[10] == 'V' && data[11] == 'E');

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  uint32_t dataSize = 0;
  std::memcpy(&dataSize, data + 4, sizeof(uint32_t));
  auto tmp = std::make_unique<FfmpegMemoryStreamSource>(gsl::span{data, dataSize + 8});

  static constexpr size_t ChunkSize = 8192;
  std::vector<int16_t> pcm;
  while(true)
  {
    const auto offset = pcm.size();
    pcm.resize(pcm.size() + tmp->getChannels() * ChunkSize);
    const auto read = tmp->read(&pcm[offset], ChunkSize, false);
    if(read != ChunkSize)
    {
      pcm.erase(
        std::next(pcm.begin(), gsl::narrow<std::vector<int16_t>::difference_type>(offset + tmp->getChannels() * read)),
        pcm.end());
      break;
    }
  }

  fill(pcm.data(), pcm.size() / tmp->getChannels(), tmp->getChannels(), tmp->getSampleRate());
}
} // namespace audio
