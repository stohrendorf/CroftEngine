#pragma once

#include "qs/qs.h"
#include "serialization/serialization_fwd.h" // IWYU pragma: keep
#include "units.h"

#include <boost/assert.hpp>
#include <cstdint>
#include <glm/gtx/euler_angles.hpp>
#include <glm/mat4x4.hpp>
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

[[nodiscard]] inline float sin(const Angle& a) noexcept
{
  return glm::sin(toRad(a));
}

[[nodiscard]] inline float cos(const Angle& a) noexcept
{
  return glm::cos(toRad(a));
}

[[nodiscard]] inline Angle abs(const Angle& a)
{
  return Angle{glm::abs(a.get())};
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
  case Axis::Deg0: return 0_deg;
  case Axis::Right90: return 90_deg;
  case Axis::Deg180: return -180_deg;
  case Axis::Left90: return -90_deg;
  default: return 0_deg;
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

  [[nodiscard]] glm::mat4 toMatrix() const
  {
    return glm::yawPitchRoll(-toRad(Y), toRad(X), -toRad(Z));
  }

  [[nodiscard]] TRRotation operator-() const
  {
    return TRRotation{-X, -Y, -Z};
  }

  void serialize(const serialization::Serializer<engine::world::World>& ser);
};

inline glm::mat4 fromPackedAngles(uint32_t angleData)
{
  const auto getAngle = [angleData](const uint8_t n) -> Angle
  {
    BOOST_ASSERT(n < 3);
    return auToAngle(((angleData >> (10u * n)) & 0x3ffu) * 64);
  };

  const TRRotation r{getAngle(2), getAngle(1), getAngle(0)};

  return r.toMatrix();
}

struct TRRotationXY
{
  Angle X{0_deg};
  Angle Y{0_deg};

  [[nodiscard]] glm::mat4 toMatrix() const
  {
    return glm::yawPitchRoll(-toRad(Y), toRad(X), 0.0f);
  }

  void serialize(const serialization::Serializer<engine::world::World>& ser);
};

[[nodiscard]] inline TRRotationXY getVectorAngles(const Length& dx, const Length& dy, const Length& dz)
{
  const auto y = angleFromAtan(dx, dz);
  const auto dxz = sqrt(dx * dx + dz * dz);
  auto x = angleFromAtan(dy, dxz);
  if((dy < 0_len) == (toRad(x) < 0))
    x = -x;

  return TRRotationXY{x, y};
}

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
