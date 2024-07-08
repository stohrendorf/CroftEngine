#include "rendersettings.h"

#include "serialization/optional.h"
#include "serialization/optional_value.h"
#include "serialization/serialization.h"

#include <functional>

namespace render
{
void RenderSettings::serialize(const serialization::Serializer<engine::EngineConfig>& ser) const
{
  ser(S_NV("crtActive", crtActive),
      S_NV("crtVersion", crtVersion),
      S_NV("dof", dof),
      S_NV("lensDistortion", lensDistortion),
      S_NV("filmGrain", filmGrain),
      S_NV("fullscreen", fullscreen),
      S_NV("bilinearFiltering", bilinearFiltering),
      S_NV("waterDenoise", waterDenoise),
      S_NV("hbao", hbao),
      S_NV("edges", edges),
      S_NV("velvia", velvia),
      S_NV("fxaaActive", fxaaActive),
      S_NV("fxaaPreset", fxaaPreset),
      S_NV("bloom", bloom),
      S_NV("brightnessEnabled", brightnessEnabled),
      S_NV("brightness", brightness),
      S_NV("contrastEnabled", contrastEnabled),
      S_NV("constrast", contrast),
      S_NV("moreLights", moreLights),
      S_NV("dustActive", dustActive),
      S_NV("dustDensity", dustDensity),
      S_NV("highQualityShadows", highQualityShadows),
      S_NV("anisotropyLevel", anisotropyLevel),
      S_NV("anisotropyActive", anisotropyActive),
      S_NV("renderResolutionDivisor", renderResolutionDivisor),
      S_NV("renderResolutionDivisorActive", renderResolutionDivisorActive),
      S_NV("uiScaleMultiplier", uiScaleMultiplier),
      S_NV("uiScaleActive", uiScaleActive),
      S_NV("muzzleFlashLight", muzzleFlashLight),
      S_NV("lightingMode", lightingMode),
      S_NV("lightingModeActive", lightingModeActive),
      S_NV("glidosPack", glidosPack));
}

void RenderSettings::deserialize(const serialization::Deserializer<engine::EngineConfig>& ser)
{
  ser(S_NVO("crtActive", std::ref(crtActive)),
      S_NVO("crtVersion", std::ref(crtVersion)),
      S_NVO("dof", std::ref(dof)),
      S_NVO("lensDistortion", std::ref(lensDistortion)),
      S_NVO("filmGrain", std::ref(filmGrain)),
      S_NVO("fullscreen", std::ref(fullscreen)),
      S_NVO("bilinearFiltering", std::ref(bilinearFiltering)),
      S_NVO("waterDenoise", std::ref(waterDenoise)),
      S_NVO("hbao", std::ref(hbao)),
      S_NVO("edges", std::ref(edges)),
      S_NVO("velvia", std::ref(velvia)),
      S_NVO("fxaaActive", std::ref(fxaaActive)),
      S_NVO("fxaaPreset", std::ref(fxaaPreset)),
      S_NVO("bloom", std::ref(bloom)),
      S_NVO("brightnessEnabled", std::ref(brightnessEnabled)),
      S_NVO("brightness", std::ref(brightness)),
      S_NVO("contrastEnabled", std::ref(contrastEnabled)),
      S_NVO("constrast", std::ref(contrast)),
      S_NVO("moreLights", std::ref(moreLights)),
      S_NVO("dustActive", std::ref(dustActive)),
      S_NVO("dustDensity", std::ref(dustDensity)),
      S_NVO("highQualityShadows", std::ref(highQualityShadows)),
      S_NVO("anisotropyLevel", std::ref(anisotropyLevel)),
      S_NVO("anisotropyActive", std::ref(anisotropyActive)),
      S_NVO("renderResolutionDivisor", std::ref(renderResolutionDivisor)),
      S_NVO("renderResolutionDivisorActive", std::ref(renderResolutionDivisorActive)),
      S_NVO("uiScaleMultiplier", std::ref(uiScaleMultiplier)),
      S_NVO("uiScaleActive", std::ref(uiScaleActive)),
      S_NVO("muzzleFlashLight", std::ref(muzzleFlashLight)),
      S_NVO("lightingMode", std::ref(lightingMode)),
      S_NVO("lightingModeActive", std::ref(lightingModeActive)),
      S_NVO("glidosPack", std::ref(glidosPack)));
}
} // namespace render
