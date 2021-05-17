#include "rendersettings.h"

#include "serialization/optional.h"
#include "serialization/optional_value.h"
#include "serialization/serialization.h"

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
      S_NVO("moreLights", moreLights),
      S_NVO("highQualityShadows", highQualityShadows),
      S_NVO("anisotropicFiltering", anisotropicFiltering),
      S_NVO("glidosPack", glidosPack));
}
} // namespace render
