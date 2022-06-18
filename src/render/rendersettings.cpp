#include "rendersettings.h"

#include "serialization/optional.h"
#include "serialization/optional_value.h"
#include "serialization/serialization.h"

#include <exception>

namespace render
{
void RenderSettings::serialize(const serialization::Serializer<engine::EngineConfig>& ser)
{
  ser(S_NVO("crt", crt),
      S_NVO("dof", dof),
      S_NVO("lensDistortion", lensDistortion),
      S_NVO("filmGrain", filmGrain),
      S_NVO("fullscreen", fullscreen),
      S_NVO("bilinearFiltering", bilinearFiltering),
      S_NVO("waterDenoise", waterDenoise),
      S_NVO("hbao", hbao),
      S_NVO("velvia", velvia),
      S_NVO("fxaa", fxaa),
      S_NVO("bloom", bloom),
      S_NVO("moreLights", moreLights),
      S_NVO("dust", dust),
      S_NVO("highQualityShadows", highQualityShadows),
      S_NVO("anisotropyLevel", anisotropyLevel),
      S_NVO("anisotropyActive", anisotropyActive),
      S_NVO("halfResRender", halfResRender),
      S_NVO("doubleUiScale", doubleUiScale),
      S_NVO("glidosPack", glidosPack));
}
} // namespace render
