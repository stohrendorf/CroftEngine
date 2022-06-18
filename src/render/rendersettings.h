#pragma once

#include "serialization/serialization_fwd.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>

namespace engine
{
struct EngineConfig;
}

namespace render
{
struct RenderSettings
{
  bool crt = true;
  bool dof = true;
  bool lensDistortion = true;
  bool filmGrain = true;
  bool fullscreen = false;
  bool bilinearFiltering = false;
  uint32_t anisotropyLevel = std::numeric_limits<uint32_t>::max();
  bool anisotropyActive = true;
  bool waterDenoise = false;
  bool hbao = true;
  bool velvia = true;
  bool fxaa = true;
  bool bloom = true;
  bool moreLights = true;
  bool dust = true;
  bool highQualityShadows = true;
  bool halfResRender = false;
  bool doubleUiScale = false;
  std::optional<std::string> glidosPack = std::nullopt;

  [[nodiscard]] size_t getLightCollectionDepth() const
  {
    return moreLights ? 2 : 1;
  }

  [[nodiscard]] int32_t getCSMResolution() const
  {
    return highQualityShadows ? 2048 : 1024;
  }

  void serialize(const serialization::Serializer<engine::EngineConfig>& ser);
};
} // namespace render
