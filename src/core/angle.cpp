#include "angle.h"

#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "units.h"
#include "util/memaccess.h"

#include <boost/assert.hpp>
#include <cstdint>
#include <glm/gtx/euler_angles.hpp>
#include <glm/mat4x4.hpp>
#include <gsl/gsl-lite.hpp>

namespace core
{
void TRRotationXY::serialize(const serialization::Serializer<engine::world::World>& ser) const
{
  ser(S_NV("x", X), S_NV("y", Y));
}

void TRRotationXY::deserialize(const serialization::Deserializer<engine::world::World>& ser)
{
  ser(S_NV("x", X), S_NV("y", Y));
}

glm::mat4 TRRotationXY::toMatrix() const
{
  return glm::yawPitchRoll(-toRad(Y), toRad(X), 0.0f);
}

void TRRotation::serialize(const serialization::Serializer<engine::world::World>& ser) const
{
  ser(S_NV("x", X), S_NV("y", Y), S_NV("z", Z));
}

void TRRotation::deserialize(const serialization::Deserializer<engine::world::World>& ser)
{
  ser(S_NV("x", X), S_NV("y", Y), S_NV("z", Z));
}

glm::mat4 TRRotation::toMatrix() const
{
  return glm::yawPitchRoll(-toRad(Y), toRad(X), -toRad(Z));
}

TRRotationXY getVectorAngles(const Length& dx, const Length& dy, const Length& dz)
{
  const auto y = angleFromAtan(dx, dz);
  const auto dxz = sqrt(dx * dx + dz * dz);
  auto x = angleFromAtan(dy, dxz);
  if((dy < 0_len) == (toRad(x) < 0))
    x = -x;

  return TRRotationXY{x, y};
}

glm::mat4 fromPackedAngles(const uint8_t* angleData)
{
  const auto getAngle = [value = util::readUnaligned32LE(angleData)](const uint8_t n) -> Angle
  {
    BOOST_ASSERT(n < 3);
    return auToAngle(gsl::narrow_cast<int16_t>(((value >> (10u * n)) & 0x3ffu) * 64));
  };

  const TRRotation r{getAngle(2), getAngle(1), getAngle(0)};

  return r.toMatrix();
}
} // namespace core
