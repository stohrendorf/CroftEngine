#pragma once

#include "serialization/serialization_fwd.h"

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
  bool waterDenoise = true;
  bool performanceMeter = false;

  void serialize(const serialization::Serializer<engine::EngineConfig>& ser);
};
} // namespace render
