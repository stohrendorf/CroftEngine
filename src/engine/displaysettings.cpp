#include "displaysettings.h"

#include "serialization/optional_value.h"
#include "serialization/serialization.h"

#include <functional>

namespace engine
{
void DisplaySettings::serialize(const serialization::Serializer<engine::EngineConfig>& ser) const
{
  ser(S_NV("ghost", ghost), S_NV("showCoopNames", showCoopNames));
}

void DisplaySettings::deserialize(const serialization::Deserializer<engine::EngineConfig>& ser)
{
  ser(S_NVO("ghost", std::ref(ghost)), S_NVO("showCoopNames", std::ref(showCoopNames)));
}
} // namespace engine
