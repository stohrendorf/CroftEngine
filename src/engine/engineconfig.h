#pragma once

#include "displaysettings.h"
#include "render/rendersettings.h"

namespace engine
{
struct EngineConfig
{
  render::RenderSettings renderSettings{};
  DisplaySettings displaySettings{};

  void serialize(const serialization::Serializer<EngineConfig>& ser);
};
} // namespace engine
