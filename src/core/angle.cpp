#include "angle.h"

namespace core
{
void TRRotationXY::serialize(const serialization::Serializer& ser)
{
  ser(S_NVP(X), S_NVP(Y));
}

void TRRotation::serialize(const serialization::Serializer& ser)
{
  ser(S_NVP(X), S_NVP(Y), S_NVP(Z));
}
} // namespace core
