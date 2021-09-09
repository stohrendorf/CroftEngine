#include "bufferhandle.h"

#include "sndfile/helpers.h"
#include "utils.h"

#include <boost/log/trivial.hpp>
#include <vector>

namespace audio
{
// NOLINTNEXTLINE(readability-make-member-function-const)
void BufferHandle::fill(const int16_t* samples, const size_t sampleCount, const int channels, const int sampleRate)
{
  m_sampleCount = sampleCount;
  m_sampleRate = sampleRate;
  AL_ASSERT(alBufferData(m_handle,
                         channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16,
                         samples,
                         gsl::narrow<ALsizei>(sampleCount * sizeof(samples[0])),
                         sampleRate));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void BufferHandle::fillFromWav(const uint8_t* data)
{
  Expects(data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F');
  Expects(data[8] == 'W' && data[9] == 'A' && data[10] == 'V' && data[11] == 'E');

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  uint32_t dataSize = 0;
  std::memcpy(&dataSize, data + 4, sizeof(uint32_t));
  sndfile::MemBufferFileIo wavMem(data, gsl::narrow_cast<sf_count_t>(dataSize) + 8);
  SF_INFO sfInfo;
  memset(&sfInfo, 0, sizeof(sfInfo));
  SNDFILE* sfFile = sf_open_virtual(&wavMem, SFM_READ, &sfInfo, &wavMem);

  if(sfFile == nullptr)
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to load WAV sample from memory: " << sf_strerror(sfFile);
    return;
  }

  BOOST_ASSERT(sfInfo.frames >= 0);

  std::vector<int16_t> pcm(sfInfo.frames * sfInfo.channels);
  sf_readf_short(sfFile, pcm.data(), sfInfo.frames);
  fill(pcm.data(), pcm.size(), sfInfo.channels, sfInfo.samplerate);

  sf_close(sfFile);
}
} // namespace audio
