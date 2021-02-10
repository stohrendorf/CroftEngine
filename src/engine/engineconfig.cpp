#include "engineconfig.h"

#include "serialization/default.h"
#include "serialization/serialization.h"

namespace engine
{
void EngineConfig::serialize(const serialization::Serializer<EngineConfig>& ser)
{
  ser(S_NVD("renderSettings", renderSettings, render::RenderSettings{}));
}
} // namespace engine
