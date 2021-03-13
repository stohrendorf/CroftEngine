#include "angle.h"

#include "serialization/quantity.h"
#include "serialization/serialization.h"

namespace core
{
void TRRotationXY::serialize(const serialization::Serializer<engine::world::World>& ser)
{
  ser(S_NV("x", X), S_NV("y", Y));
}

void TRRotation::serialize(const serialization::Serializer<engine::world::World>& ser)
{
  ser(S_NV("x", X), S_NV("y", Y), S_NV("z", Z));
}
} // namespace core
