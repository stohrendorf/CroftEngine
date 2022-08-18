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
  bool crtActive = false;
  uint8_t crtVersion = 1;
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
  bool fxaaActive = true;
  uint8_t fxaaPreset = 39;
  bool bloom = true;
  bool brightnessEnabled = false;
  int8_t brightness = 0;
  bool contrastEnabled = false;
  int8_t contrast = 0;
  bool moreLights = true;
  bool dustActive = true;
  uint8_t dustDensity = 1;
  bool highQualityShadows = true;
  uint8_t renderResolutionDivisor = 2;
  bool renderResolutionDivisorActive = false;
  uint8_t uiScaleMultiplier = 2;
  bool uiScaleActive = false;
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
