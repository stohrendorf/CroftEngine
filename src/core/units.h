#pragma once

#include "qs/qs.h"

#include <cmath>
#include <cstdint>

namespace core
{
QS_DECLARE_QUANTITY(Frame, int32_t, "frame");
QS_LITERAL_OP_ULL(Frame, _frame)

QS_DECLARE_QUANTITY(Health, int32_t, "hp");
QS_LITERAL_OP_ULL(Health, _hp)

QS_DECLARE_QUANTITY(Length, int32_t, "u");
QS_LITERAL_OP_ULL(Length, _len)

using Area = QS_COMBINE_UNITS(Length, *, Length);

[[nodiscard]] inline Length sqrt(const Area& area)
{
  return Length{static_cast<Length::type>(std::sqrt(area.get()))};
}

[[nodiscard]] constexpr Length lerp(const Length& a, const Length& b, float bias)
{
  return Length{static_cast<Length::type>(a.get() * (1 - bias) + b.get() * bias)};
}

using LengthF = Length::as_type<float>;

QS_LITERAL_OP_LD(LengthF, _len)

QS_DECLARE_QUANTITY(Seconds, int32_t, "s");
QS_LITERAL_OP_ULL(Seconds, _sec)

using Speed = QS_COMBINE_UNITS(Length, /, Frame);
QS_LITERAL_OP_ULL(Speed, _spd)

using Acceleration = QS_COMBINE_UNITS(Speed, /, Frame);

// NOLINTNEXTLINE(performance-unnecessary-value-param)
[[nodiscard]] inline auto sqrt(QS_COMBINE_UNITS(Acceleration, *, Length) value)
{
  return Speed{static_cast<Speed::type>(std::sqrt(value.get()))};
}

QS_DECLARE_QUANTITY(Shade, int16_t, "shade");
QS_DECLARE_QUANTITY(Intensity, int16_t, "intensity");
QS_DECLARE_QUANTITY(Brightness, float, "brightness");

[[nodiscard]] inline constexpr Brightness toBrightness(const Shade& shade)
{
  return Brightness{2.0f - shade.get<float>() / 4096.0f};
}

[[nodiscard]] inline constexpr Brightness toBrightness(const Intensity& intensity)
{
  return Brightness{intensity.get<float>() / 4096.0f};
}

QS_DECLARE_QUANTITY(Angle, int32_t, "au");
using RotationSpeed = QS_COMBINE_UNITS(Angle, /, Frame);
using RotationAcceleration = QS_COMBINE_UNITS(RotationSpeed, /, Frame);

constexpr int32_t FullRotation = 1u << 16u;
constexpr int32_t AngleStorageScale = 1u << 16u;

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

using core::operator""_frame;
using core::operator""_hp;
using core::operator""_len;
using core::operator""_sec;
using core::operator""_spd;
using core::operator""_au;
using core::operator""_deg;
