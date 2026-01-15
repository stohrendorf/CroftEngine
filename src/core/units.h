#pragma once

#include "qs/qs.h"

#include <cmath>
#include <cstdint>
#include <gsl-lite/gsl-lite.hpp>

namespace core
{
// Frame represents a game logic tick at fixed 30 Hz (not a render frame), but at the same time also an animation frame.
// The engine runs game logic (physics, AI, input) at 30 FPS. Rendering can be uncapped and uses interpolation between logic frames.
QS_DECLARE_QUANTITY(Frame, int32_t, "frame");
QS_LITERAL_OP_ULL(Frame, _frame)

QS_DECLARE_QUANTITY(MenuFrame, int32_t, "mframe");
QS_LITERAL_OP_ULL(MenuFrame, _mframe)

QS_DECLARE_QUANTITY(Tick, int32_t, "tick");
QS_LITERAL_OP_ULL(Tick, _tick)

QS_DECLARE_QUANTITY(Health, int32_t, "hp");
QS_LITERAL_OP_ULL(Health, _hp)

QS_DECLARE_QUANTITY(Length, int32_t, "u");
QS_LITERAL_OP_ULL(Length, _len)
QS_LITERAL_OP_LD(Length, _len)

QS_DECLARE_QUANTITY(Degrees, float, "deg");
QS_LITERAL_OP_ULL(Degrees, _fdeg)
QS_LITERAL_OP_LD(Degrees, _fdeg)

QS_DECLARE_QUANTITY(Radians, float, "rad");
QS_LITERAL_OP_ULL(Radians, _rad)
QS_LITERAL_OP_LD(Radians, _rad)

using Area = QS_COMBINE_UNITS(Length, *, Length);

[[nodiscard]] inline Length sqrt(const Area& area) noexcept
{
  return Length{static_cast<Length::type>(std::sqrt(area.get()))};
}

QS_DECLARE_QUANTITY(Seconds, int32_t, "s");
QS_LITERAL_OP_ULL(Seconds, _sec)

using Speed = QS_COMBINE_UNITS(Length, /, Frame);
QS_LITERAL_OP_ULL(Speed, _spd)

using Acceleration = QS_COMBINE_UNITS(Speed, /, Frame);

// NOLINTNEXTLINE(performance-unnecessary-value-param)
[[nodiscard]] inline auto sqrt(QS_COMBINE_UNITS(Acceleration, *, Length) value) noexcept
{
  return Speed{static_cast<Speed::type>(std::sqrt(value.get()))};
}

QS_DECLARE_QUANTITY(Shade, int16_t, "shade");
QS_DECLARE_QUANTITY(Intensity, int16_t, "intensity");
QS_DECLARE_QUANTITY(Brightness, float, "brightness");

[[nodiscard]] constexpr Brightness toBrightness(const Shade& shade)
{
  return Brightness{2.0f - shade.get<float>() / 4096.0f};
}

[[nodiscard]] constexpr Brightness toBrightness(const Intensity& intensity)
{
  return Brightness{intensity.get<float>() / 4096.0f};
}

QS_DECLARE_QUANTITY(Angle, int32_t, "au");
using RotationSpeed = QS_COMBINE_UNITS(Angle, /, Frame);
using RotationAcceleration = QS_COMBINE_UNITS(RotationSpeed, /, Frame);

constexpr int32_t FullRotation = 1u << 16u;
constexpr int32_t AngleStorageScale = 1u << 16u;

[[nodiscard]] constexpr Angle auToAngle(const int16_t value) noexcept
{
  return Angle{static_cast<Angle::type>(value) * AngleStorageScale};
}

[[nodiscard]] constexpr Angle operator"" _au(const unsigned long long value) noexcept
{
  return auToAngle(gsl_lite::narrow_cast<int16_t>(value));
}

[[nodiscard]] constexpr Angle operator"" _deg(const unsigned long long value) noexcept
{
  return Angle{gsl_lite::narrow_cast<Angle::type>(value * FullRotation / 360 * AngleStorageScale)};
}

[[nodiscard]] constexpr Angle operator"" _deg(const long double value) noexcept
{
  return Angle{static_cast<Angle::type>(value * FullRotation / 360 * AngleStorageScale)};
}
} // namespace core

using core::operator""_frame;
using core::operator""_mframe;
using core::operator""_tick;
using core::operator""_hp;
using core::operator""_len;
using core::operator""_sec;
using core::operator""_spd;
using core::operator""_au;
using core::operator""_deg;
using core::operator""_fdeg;
using core::operator""_rad;