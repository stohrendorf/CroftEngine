#pragma once

#include "sndfile/helpers.h"
#include "utils.h"

#include <AL/al.h>
#include <boost/log/trivial.hpp>
#include <vector>

namespace audio
{
class BufferHandle final
{
  const ALuint m_handle{};

  [[nodiscard]] static ALuint createHandle()
  {
    ALuint handle;
    AL_ASSERT(alGenBuffers(1, &handle));

    Expects(alIsBuffer(handle));

    return handle;
  }

public:
  explicit BufferHandle()
      : m_handle{createHandle()}
  {
  }

  explicit BufferHandle(const BufferHandle&) = delete;
  explicit BufferHandle(BufferHandle&&) = delete;
  BufferHandle& operator=(const BufferHandle&) = delete;
  BufferHandle& operator=(BufferHandle&&) = delete;

  ~BufferHandle()
  {
    // it's not possible to check if a buffer is in use by using the OpenAL API.
    // thus errors are ignored here.
    alDeleteBuffers(1, &m_handle);
    alGetError();
  }

  [[nodiscard]] ALuint get() const noexcept
  {
    return m_handle;
  }

  // NOLINTNEXTLINE(readability-make-member-function-const)
  void fill(const int16_t* samples, const size_t sampleCount, const int channels, const int sampleRate)
  {
    AL_ASSERT(alBufferData(m_handle,
                           channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16,
                           samples,
                           gsl::narrow<ALsizei>(sampleCount * sizeof(samples[0])),
                           sampleRate));
  }

  // NOLINTNEXTLINE(readability-make-member-function-const)
  bool fillFromWav(const uint8_t* data)
  {
    Expects(data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F');
    Expects(data[8] == 'W' && data[9] == 'A' && data[10] == 'V' && data[11] == 'E');

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    sndfile::MemBufferFileIo wavMem(data, static_cast<size_t>(*reinterpret_cast<const uint32_t*>(data + 4)) + 8u);
    SF_INFO sfInfo;
    memset(&sfInfo, 0, sizeof(sfInfo));
    SNDFILE* sfFile = sf_open_virtual(&wavMem, SFM_READ, &sfInfo, &wavMem);

    if(sfFile == nullptr)
    {
      BOOST_LOG_TRIVIAL(error) << "Failed to load WAV sample from memory: " << sf_strerror(sfFile);
      return false;
    }

    BOOST_ASSERT(sfInfo.frames >= 0);

    std::vector<int16_t> pcm(sfInfo.frames * sfInfo.channels);
    sf_readf_short(sfFile, pcm.data(), sfInfo.frames);
    fill(pcm.data(), pcm.size(), sfInfo.channels, sfInfo.samplerate);

    sf_close(sfFile);

    return true;
  }
};
} // namespace audio
