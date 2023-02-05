#include "displaysettings.h"

#include "serialization/optional_value.h"
#include "serialization/serialization.h"

#include <exception>

namespace engine
{
void DisplaySettings::serialize(const serialization::Serializer<engine::EngineConfig>& ser) const
{
  ser(S_NVO("ghost", ghost));
}

void DisplaySettings::deserialize(const serialization::Deserializer<engine::EngineConfig>& ser)
{
  ser(S_NVO("ghost", ghost));
}
} // namespace engine
