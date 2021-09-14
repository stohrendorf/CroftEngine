#pragma once

#include "serialization/serialization_fwd.h" // IWYU pragma: keep

namespace engine
{
struct EngineConfig;

struct AudioSettings
{
  float globalVolume = 1.0f;
  float musicVolume = 0.8f;
  float sfxVolume = 0.8f;

  void serialize(const serialization::Serializer<EngineConfig>& ser);
};
} // namespace engine
