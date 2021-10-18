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
#include <optional>
#include <type_traits>

namespace engine::world
{
class World;
}

namespace core
{
constexpr int32_t FullRotation = 1u << 16u;
constexpr int32_t AngleStorageScale = 1u << 16u;

QS_DECLARE_QUANTITY(Angle, int32_t, "au");
using RotationSpeed = QS_COMBINE_UNITS(Angle, /, Frame);

[[nodiscard]] constexpr Angle auToAngle(int16_t value) noexcept;

[[nodiscard]] constexpr Angle operator"" _deg(unsigned long long value) noexcept;

[[nodiscard]] constexpr Angle operator"" _deg(long double value) noexcept;

[[nodiscard]] inline Angle angleFromRad(const float r)
{
  return Angle{gsl::narrow_cast<Angle::type>(r / 2 / glm::pi<float>() * FullRotation * AngleStorageScale)};
}

[[nodiscard]] inline Angle angleFromAtan(const float dx, const float dz)
{
  return angleFromRad(std::atan2(dx, dz));
}

[[nodiscard]] inline Angle angleFromDegrees(const float value)
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

[[nodiscard]] inline Angle abs(const Angle& a)
{
  return Angle{std::abs(a.get())};
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
  Expects(margin >= 0_deg && margin <= 45_deg);
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

[[nodiscard]] inline Axis axisFromAngle(Angle angle)
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

[[nodiscard]] constexpr Angle auToAngle(int16_t value) noexcept
{
  return Angle{static_cast<Angle::type>(value) * AngleStorageScale};
}

[[nodiscard]] constexpr Angle operator"" _au(const unsigned long long value) noexcept
{
  return auToAngle(static_cast<int16_t>(value));
}

[[nodiscard]] constexpr Angle operator"" _deg(const unsigned long long value) noexcept
{
  return Angle{static_cast<Angle::type>(value * FullRotation / 360 * AngleStorageScale)};
}

[[nodiscard]] constexpr Angle operator"" _deg(const long double value) noexcept
{
  return Angle{static_cast<Angle::type>(value * FullRotation / 360 * AngleStorageScale)};
}
} // namespace core

using core::operator""_au;
using core::operator""_deg;
