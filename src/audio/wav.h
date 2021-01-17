#pragma once

#include "sndfile/helpers.h"

#include <boost/log/trivial.hpp>
#include <soloud_wav.h>
#include <vector>

namespace audio
{
inline std::shared_ptr<SoLoud::Wav> loadWav(const uint8_t* data)
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
    return nullptr;
  }

  BOOST_ASSERT(sfInfo.frames >= 0);

  std::vector<short> pcm(sfInfo.frames);
  sf_readf_short(sfFile, pcm.data(), sfInfo.frames);

  auto wav = std::make_shared<SoLoud::Wav>();
  if(wav->loadRawWave16(pcm.data(), pcm.size(), sfInfo.samplerate, sfInfo.channels) != SoLoud::SO_NO_ERROR)
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to create WAV from memory";
  }

  sf_close(sfFile);

  return wav;
}
} // namespace audio
