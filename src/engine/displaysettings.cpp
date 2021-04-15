#include "displaysettings.h"

#include "serialization/default.h"
#include "serialization/serialization.h"

namespace engine
{
void DisplaySettings::serialize(const serialization::Serializer<engine::EngineConfig>& ser)
{
  ser(S_NVD("performanceMeter", performanceMeter, false));
}
} // namespace engine
