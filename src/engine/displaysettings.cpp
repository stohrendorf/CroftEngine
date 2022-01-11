#include "displaysettings.h"

#include "serialization/optional_value.h"
#include "serialization/serialization.h"

namespace engine
{
void DisplaySettings::serialize(const serialization::Serializer<engine::EngineConfig>& ser)
{
  ser(S_NVO("performanceMeter", performanceMeter), S_NVO("ghost", ghost));
}
} // namespace engine
