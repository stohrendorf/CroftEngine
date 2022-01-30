#pragma once

#include "serialization/serialization_fwd.h"

namespace engine
{
struct EngineConfig;

struct DisplaySettings
{
  bool ghost = false;

  void serialize(const serialization::Serializer<engine::EngineConfig>& ser);
};
} // namespace engine
