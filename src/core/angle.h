#pragma once

#include "qs/qs.h"
#include "serialization/serialization_fwd.h"
#include "units.h"

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
  return Angle{gsl::narrow_cast<Angle::type>(r / 2 / glm::pi<float>() * FullRotation * AngleStorageScale)};
}

[[nodiscard]] inline Angle angleFromAtan(const float dx, const float dz)
{
  return angleFromRad(std::atan2(dx, dz));
}

[[nodiscard]] inline Angle angleFromDegrees(const float value) noexcept
{
  return Angle{gsl::narrow_cast<Angle::type>(std::lround(value / 360 * FullRotation * AngleStorageScale))};
}

[[nodiscard]] inline Angle angleFromAtan(const Length& dx, const Length& dz)
{
  return angleFromRad(std::atan2(dx.get<float>(), dz.get<float>()));
}

[[nodiscard]] constexpr float toDegrees(const Angle& a) noexcept
{
  return a.get<float>() / AngleStorageScale * 360 / FullRotation;
}

[[nodiscard]] inline float toRad(const Angle& a) noexcept
{
  return a.get<float>() / AngleStorageScale * glm::pi<float>() * 2 / FullRotation;
}

[[nodiscard]] inline auto toAu(const Angle& a) noexcept
{
  return a.get() / AngleStorageScale;
}

[[nodiscard]] inline float sin(const Angle& a) noexcept
{
  return std::sin(toRad(a));
}

[[nodiscard]] inline float cos(const Angle& a) noexcept
{
  return std::cos(toRad(a));
}

[[nodiscard]] inline Angle abs(const Angle& a) noexcept
{
  const auto tmp = Angle{std::abs(a.get())};
  if(tmp >= 0_deg)
    return tmp;

  // abs(-180_deg) == -180_deg unfortunately
  return Angle{std::numeric_limits<Angle::type>::max()};
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

[[nodiscard]] inline std::optional<Axis> axisFromAngle(const Angle& angle, const Angle& margin)
{
  gsl_Expects(margin >= 0_deg && margin <= 45_deg);
  if(angle < +0_deg + margin && angle >= +0_deg - margin) // cppcheck-suppress knownConditionTrueFalse
    return Axis::Deg0;
  if(angle < +90_deg + margin && angle >= +90_deg - margin)
    return Axis::Right90;
  if(angle < -90_deg + margin && angle >= -90_deg - margin)
    return Axis::Left90;
  if(angle >= 180_deg - margin || angle <= -180_deg + margin)
    return Axis::Deg180;

  return std::nullopt;
}

[[nodiscard]] inline Axis axisFromAngle(Angle angle) noexcept
{
  angle += 45_deg;
  if(angle < -90_deg)
    return Axis::Deg180;
  else if(angle < 0_deg)
    return Axis::Left90;
  else if(angle < 90_deg)
    return Axis::Deg0;
  else
    return Axis::Right90;
}

[[nodiscard]] inline Angle snapRotation(const Axis& axis) noexcept
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

  TRRotation(const Angle& x, const Angle& y, const Angle& z) noexcept
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

  [[nodiscard]] TRRotation operator-(const TRRotation& rhs) const noexcept
  {
    return {X - rhs.X, Y - rhs.Y, Z - rhs.Z};
  }

  [[nodiscard]] glm::mat4 toMatrix() const;

  [[nodiscard]] TRRotation operator-() const noexcept
  {
    return TRRotation{-X, -Y, -Z};
  }

  void serialize(const serialization::Serializer<engine::world::World>& ser) const;
  void deserialize(const serialization::Deserializer<engine::world::World>& ser);
};

[[nodiscard]] extern glm::mat4 fromPackedAngles(const uint8_t* angleData);

struct TRRotationXY
{
  Angle X{0_deg};
  Angle Y{0_deg};

  [[nodiscard]] glm::mat4 toMatrix() const;

  void serialize(const serialization::Serializer<engine::world::World>& ser) const;
  void deserialize(const serialization::Deserializer<engine::world::World>& ser);
};

[[nodiscard]] extern TRRotationXY getVectorAngles(const Length& dx, const Length& dy, const Length& dz);
} // namespace core
