#pragma once

#include "render/rendersettings.h"

namespace engine
{
struct EngineConfig
{
  render::RenderSettings renderSettings{};

  void serialize(const serialization::Serializer<EngineConfig>& ser);
};
} // namespace engine
