#include "audiosettings.h"

#include "serialization/default.h"
#include "serialization/serialization.h"

namespace engine
{
void AudioSettings::serialize(const serialization::Serializer<EngineConfig>& ser)
{
  ser(S_NVD("globalVolume", globalVolume, 1.0f),
      S_NVD("musicVolume", musicVolume, 0.8f),
      S_NVD("sfxVolume", sfxVolume, 0.8f));
}
} // namespace engine
