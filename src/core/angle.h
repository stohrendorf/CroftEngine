#pragma once

#include "core/units.h"
#include "gsl-lite.hpp"
#include "serialization/serialization.h"

#include <cmath>
#include <glm/gtx/euler_angles.hpp>
#include <optional>
#include <sol/sol.hpp>

namespace core
{
constexpr int32_t FullRotation = 1u << 16u;
constexpr int32_t AngleStorageScale = 1u << 16u;

QS_DECLARE_QUANTITY(Angle, int32_t, "au");

constexpr Angle auToAngle(int16_t value) noexcept;

constexpr Angle operator"" _deg(unsigned long long value) noexcept;

constexpr Angle operator"" _deg(long double value) noexcept;

inline Angle angleFromRad(const float r)
{
  return Angle{gsl::narrow_cast<Angle::type>(r / 2 / glm::pi<float>() * FullRotation * AngleStorageScale)};
}

inline Angle angleFromAtan(const float dx, const float dz)
{
  return angleFromRad(std::atan2(dx, dz));
}

inline Angle angleFromDegrees(const float value)
{
  return Angle{gsl::narrow_cast<Angle::type>(std::lround(value / 360 * FullRotation * AngleStorageScale))};
}

inline Angle angleFromAtan(const Length& dx, const Length& dz)
{
  return angleFromRad(std::atan2(dx.get_as<float>(), dz.get_as<float>()));
}

constexpr float toDegrees(const Angle& a) noexcept
{
  return a.get_as<float>() / AngleStorageScale * 360 / FullRotation;
}

inline float toRad(const Angle& a) noexcept
{
  return a.get_as<float>() / AngleStorageScale * glm::pi<float>() * 2 / FullRotation;
}

inline float sin(const Angle& a) noexcept
{
  return glm::sin(toRad(a));
}

inline float cos(const Angle& a) noexcept
{
  return glm::cos(toRad(a));
}

inline Angle abs(const Angle& a)
{
  return Angle{glm::abs(a.get())};
}

enum class Axis
{
  PosZ,
  PosX,
  NegZ,
  NegX
};

inline std::optional<Axis> axisFromAngle(const Angle& angle, const Angle& margin)
{
  Expects(margin >= 0_deg && margin <= 45_deg);
  if(angle <= +0_deg + margin && angle >= +0_deg - margin)
    return Axis::PosZ;
  if(angle <= +90_deg + margin && angle >= +90_deg - margin)
    return Axis::PosX;
  if(angle <= -90_deg + margin && angle >= -90_deg - margin)
    return Axis::NegX;
  if(angle >= 180_deg - margin || angle <= -180_deg + margin)
    return Axis::NegZ;

  return {};
}

inline Angle alignRotation(const Axis& axis)
{
  switch(axis)
  {
  case Axis::PosZ: return 0_deg;
  case Axis::PosX: return 90_deg;
  case Axis::NegZ: return -180_deg;
  case Axis::NegX: return -90_deg;
  default: return 0_deg;
  }
}

inline std::optional<Angle> alignRotation(const Angle& angle, const Angle& margin)
{
  auto axis = axisFromAngle(angle, margin);
  if(!axis)
    return {};

  return alignRotation(*axis);
}

class TRRotation final
{
public:
  Angle X;

  Angle Y;

  Angle Z;

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

  TRRotation operator-(const TRRotation& rhs) const
  {
    return {X - rhs.X, Y - rhs.Y, Z - rhs.Z};
  }

  [[nodiscard]] glm::mat4 toMatrix() const
  {
    return glm::yawPitchRoll(-toRad(Y), toRad(X), -toRad(Z));
  }

  TRRotation operator-() const
  {
    return TRRotation{-X, -Y, -Z};
  }

  void serialize(const serialization::Serializer& ser);
};

inline glm::mat4 fromPackedAngles(uint32_t angleData)
{
  const auto getAngle = [angleData](const uint8_t n) -> Angle {
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

  void serialize(const serialization::Serializer& ser);
};

inline TRRotationXY getVectorAngles(const Length& dx, const Length& dy, const Length& dz)
{
  const auto y = angleFromAtan(dx, dz);
  const auto dxz = sqrt(dx * dx + dz * dz);
  auto x = angleFromAtan(dy, dxz);
  if((dy < 0_len) == std::signbit(toRad(x)))
    x = -x;

  return TRRotationXY{x, y};
}

constexpr Angle auToAngle(int16_t value) noexcept
{
  return Angle{static_cast<Angle::type>(value) * AngleStorageScale};
}

constexpr Angle operator"" _au(const unsigned long long value) noexcept
{
  return auToAngle(static_cast<int16_t>(value));
}

constexpr Angle operator"" _deg(const unsigned long long value) noexcept
{
  return Angle{static_cast<Angle::type>(value * FullRotation / 360 * AngleStorageScale)};
}

constexpr Angle operator"" _deg(const long double value) noexcept
{
  return Angle{static_cast<Angle::type>(value * FullRotation / 360 * AngleStorageScale)};
}
} // namespace core

using core::operator""_au;
using core::operator""_deg;
