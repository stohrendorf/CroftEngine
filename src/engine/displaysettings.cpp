#include "displaysettings.h"

#include "serialization/optional_value.h"
#include "serialization/serialization.h"

#include <exception>
#include <functional>

namespace engine
{
void DisplaySettings::serialize(const serialization::Serializer<engine::EngineConfig>& ser) const
{
  ser(S_NV("ghost", ghost));
}

void DisplaySettings::deserialize(const serialization::Deserializer<engine::EngineConfig>& ser)
{
  ser(S_NVO("ghost", std::ref(ghost)));
}
} // namespace engine
