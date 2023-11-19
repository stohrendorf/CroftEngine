#pragma once

#include "serialization/serialization_fwd.h"

namespace engine
{
struct EngineConfig;

struct DisplaySettings
{
  bool ghost = false;
  bool showCoopNames = true;

  void serialize(const serialization::Serializer<engine::EngineConfig>& ser) const;
  void deserialize(const serialization::Deserializer<engine::EngineConfig>& ser);
};
} // namespace engine
