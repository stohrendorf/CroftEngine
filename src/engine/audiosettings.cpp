#include "audiosettings.h"

#include "serialization/default.h"
#include "serialization/serialization.h"

#include <exception>

namespace engine
{
void AudioSettings::serialize(const serialization::Serializer<EngineConfig>& ser) const
{
  ser(S_NV("globalVolume", globalVolume), S_NV("musicVolume", musicVolume), S_NV("sfxVolume", sfxVolume));
}

void AudioSettings::deserialize(const serialization::Deserializer<EngineConfig>& ser)
{
  ser(S_NVD("globalVolume", std::ref(globalVolume), 1.0f),
      S_NVD("musicVolume", std::ref(musicVolume), 0.8f),
      S_NVD("sfxVolume", std::ref(sfxVolume), 0.8f));
}
} // namespace engine
