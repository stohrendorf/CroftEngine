#include "rendersettings.h"

#include "serialization/optional.h"
#include "serialization/optional_value.h"
#include "serialization/serialization.h"

#include <exception>

namespace render
{
void RenderSettings::serialize(const serialization::Serializer<engine::EngineConfig>& ser)
{
  ser(S_NVO("crtActive", crtActive),
      S_NVO("crtVersion", crtVersion),
      S_NVO("dof", dof),
      S_NVO("lensDistortion", lensDistortion),
      S_NVO("filmGrain", filmGrain),
      S_NVO("fullscreen", fullscreen),
      S_NVO("bilinearFiltering", bilinearFiltering),
      S_NVO("waterDenoise", waterDenoise),
      S_NVO("hbao", hbao),
      S_NVO("edges", edges),
      S_NVO("velvia", velvia),
      S_NVO("fxaaActive", fxaaActive),
      S_NVO("fxaaPreset", fxaaPreset),
      S_NVO("bloom", bloom),
      S_NVO("brightnessEnabled", brightnessEnabled),
      S_NVO("brightness", brightness),
      S_NVO("contrastEnabled", contrastEnabled),
      S_NVO("constrast", contrast),
      S_NVO("moreLights", moreLights),
      S_NVO("dustActive", dustActive),
      S_NVO("dustDensity", dustDensity),
      S_NVO("highQualityShadows", highQualityShadows),
      S_NVO("anisotropyLevel", anisotropyLevel),
      S_NVO("anisotropyActive", anisotropyActive),
      S_NVO("renderResolutionDivisor", renderResolutionDivisor),
      S_NVO("renderResolutionDivisorActive", renderResolutionDivisorActive),
      S_NVO("uiScaleMultiplier", uiScaleMultiplier),
      S_NVO("uiScaleActive", uiScaleActive),
      S_NVO("glidosPack", glidosPack),
      S_NVO("muzzleFlashLight", muzzleFlashLight));
}
} // namespace render
