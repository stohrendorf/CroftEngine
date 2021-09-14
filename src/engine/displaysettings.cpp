#include "displaysettings.h"

#include "serialization/optional_value.h"
#include "serialization/serialization.h"

#include <exception>
#include <gsl/gsl-lite.hpp>

namespace engine
{
void DisplaySettings::serialize(const serialization::Serializer<engine::EngineConfig>& ser)
{
  ser(S_NVO("performanceMeter", performanceMeter));
}
} // namespace engine
