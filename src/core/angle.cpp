#include "angle.h"

#include "serialization/quantity.h"

namespace core
{
void TRRotationXY::serialize(const serialization::Serializer& ser)
{
  ser(S_NV("x", X), S_NV("y", Y));
}

void TRRotation::serialize(const serialization::Serializer& ser)
{
  ser(S_NV("x", X), S_NV("y", Y), S_NV("z", Z));
}
} // namespace core
