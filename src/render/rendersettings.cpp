#include "rendersettings.h"

#include "serialization/default.h"
#include "serialization/serialization.h"

namespace render
{
void RenderSettings::serialize(const serialization::Serializer<engine::EngineConfig>& ser)
{
  ser(S_NVD("crt", crt, true),
      S_NVD("dof", dof, true),
      S_NVD("lensDistortion", lensDistortion, true),
      S_NVD("filmGrain", filmGrain, true),
      S_NVD("fullscreen", fullscreen, false),
      S_NVD("bilinearFiltering", bilinearFiltering, false),
      S_NVD("waterDenoise", waterDenoise, true),
      S_NVD("hbao", hbao, true),
      S_NVD("moreLights", moreLights, true),
      S_NVD("highQualityShadows", highQualityShadows, true),
      S_NVD("anisotropicFiltering", anisotropicFiltering, true));
}
} // namespace render
