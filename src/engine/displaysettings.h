#pragma once

#include "serialization/serialization_fwd.h"

// IWYU pragma: no_forward_declare serialization::Serializer

namespace engine
{
struct EngineConfig;

struct DisplaySettings
{
  bool performanceMeter = false;

  void serialize(const serialization::Serializer<engine::EngineConfig>& ser);
};
} // namespace engine
