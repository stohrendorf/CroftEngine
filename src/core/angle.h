#pragma once

#include "qs/qs.h"
#include "serialization/serialization_fwd.h"
#include "units.h"

#include <boost/assert.hpp>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <glm/ext/scalar_constants.hpp>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <limits>
#include <optional>
#include <type_traits>

namespace engine::world
{
class World;
}

namespace core
{
[[nodiscard]] inline Angle angleFromRad(const float r)
{
  return Angle{gsl::narrow_cast<Angle::type>(r / 2 / glm::pi<float>() * FullRotation)};
}

[[nodiscard]] inline Angle angleFromAtan(const float dx, const float dz)
{
  return angleFromRad(std::atan2(dx, dz));
}

[[nodiscard]] inline Angle angleFromDegrees(const float value)
{
  return Angle{gsl::narrow_cast<Angle::type>(std::lround(value / 360 * FullRotation))};
}

[[nodiscard]] inline Angle angleFromAtan(const Length& dx, const Length& dz)
{
  return angleFromRad(std::atan2(dx.get<float>(), dz.get<float>()));
}

[[nodiscard]] constexpr float toDegrees(const Angle& a) noexcept
{
  return a.get<float>() * 360 / FullRotation;
}

[[nodiscard]] inline float toRad(const Angle& a) noexcept
{
  return a.get<float>() * glm::pi<float>() * 2 / FullRotation;
}

[[nodiscard]] inline auto toAu(const Angle& a) noexcept
{
  return a.get();
}

[[nodiscard]] inline float sin(const Angle& a) noexcept
{
  return std::sin(toRad(a));
}

[[nodiscard]] inline float cos(const Angle& a) noexcept
{
  return std::cos(toRad(a));
}

enum class Axis
{
  Deg0,
  PosZ = Deg0,
  Right90,
  PosX = Right90,
  Deg180,
  NegZ = Deg180,
  Left90,
  NegX = Left90
};

/**
 * Normalize an angle to be in range -180°..180°.
 */
[[nodiscard]] constexpr inline Angle normalizeAngle(Angle angle)
{
  angle = angle % 360_deg;
  if(angle < -180_deg)
    return angle + 360_deg;
  else if(angle > 180_deg)
    return angle - 360_deg;
  else
  {
    BOOST_ASSERT(angle >= -180_deg && angle <= 180_deg);
    return angle;
  }
}

[[nodiscard]] constexpr inline std::optional<Axis> axisFromAngle(Angle angle, const Angle& margin)
{
  Expects(margin >= 0_deg && margin <= 45_deg);
  angle = normalizeAngle(angle);
  if(angle < +0_deg + margin && angle >= +0_deg - margin)
    return Axis::Deg0;
  if(angle < +90_deg + margin && angle >= +90_deg - margin)
    return Axis::Right90;
  if(angle < -90_deg + margin && angle >= -90_deg - margin)
    return Axis::Left90;
  if(angle >= 180_deg - margin || angle <= -180_deg + margin)
    return Axis::Deg180;

  return std::nullopt;
}

[[nodiscard]] constexpr inline Axis axisFromAngle(Angle angle)
{
  angle = normalizeAngle(angle + 45_deg);
  if(angle < -90_deg)
    return Axis::Deg180;
  else if(angle < 0_deg)
    return Axis::Left90;
  else if(angle < 90_deg)
    return Axis::Deg0;
  else
    return Axis::Right90;
}

[[nodiscard]] inline Angle snapRotation(const Axis& axis)
{
  switch(axis)
  {
  case Axis::Deg0:
    return 0_deg;
  case Axis::Right90:
    return 90_deg;
  case Axis::Deg180:
    return -180_deg;
  case Axis::Left90:
    return -90_deg;
  default:
    return 0_deg;
  }
}

[[nodiscard]] inline std::optional<Angle> snapRotation(const Angle& angle, const Angle& margin)
{
  auto axis = axisFromAngle(angle, margin);
  if(!axis)
    return {};

  return snapRotation(*axis);
}

class TRRotation final
{
public:
  Angle X{0_deg};
  Angle Y{0_deg};
  Angle Z{0_deg};

  TRRotation() = default;

  TRRotation(const Angle& x, const Angle& y, const Angle& z)
      : X{x}
      , Y{y}
      , Z{z}
  {
  }

  [[nodiscard]] glm::vec3 toDegrees() const
  {
    return {core::toDegrees(X), core::toDegrees(Y), core::toDegrees(Z)};
  }

  [[nodiscard]] glm::vec3 toRenderSystem() const
  {
    return {toRad(X), -toRad(Y), -toRad(Z)};
  }

  [[nodiscard]] TRRotation operator-(const TRRotation& rhs) const
  {
    return {X - rhs.X, Y - rhs.Y, Z - rhs.Z};
  }

  [[nodiscard]] glm::mat4 toMatrix() const;

  [[nodiscard]] TRRotation operator-() const
  {
    return TRRotation{-X, -Y, -Z};
  }

  [[nodiscard]] auto normalized() const
  {
    return TRRotation{normalizeAngle(X), normalizeAngle(Y), normalizeAngle(Z)};
  }

  void serialize(const serialization::Serializer<engine::world::World>& ser);
};

[[nodiscard]] extern glm::mat4 fromPackedAngles(uint32_t angleData);

struct TRRotationXY
{
  Angle X{0_deg};
  Angle Y{0_deg};

  [[nodiscard]] glm::mat4 toMatrix() const;

  void serialize(const serialization::Serializer<engine::world::World>& ser);
};

[[nodiscard]] extern TRRotationXY getVectorAngles(const Length& dx, const Length& dy, const Length& dz);
} // namespace core
