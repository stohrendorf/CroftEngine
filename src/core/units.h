#pragma once

#include "qs/qs.h"

#include <cmath>

namespace core
{
QS_DECLARE_QUANTITY(Frame, int32_t, "frame");

QS_LITERAL_OP_ULL(Frame, _frame)

QS_DECLARE_QUANTITY(Health, int32_t, "hp");

QS_LITERAL_OP_ULL(Health, _hp)

QS_DECLARE_QUANTITY(Length, int32_t, "u");

QS_LITERAL_OP_ULL(Length, _len)

using Area = QS_COMBINE_UNITS(Length, *, Length);

inline Length sqrt(const Area& area)
{
  return Length{static_cast<Length::type>(std::sqrt(area.get()))};
}

constexpr Length lerp(const Length& a, const Length& b, float bias)
{
  return Length{static_cast<Length::type>(a.get() * (1 - bias) + b.get() * bias)};
}

using LengthF = Length::with_type<float>;

QS_LITERAL_OP_LD(LengthF, _len)

QS_DECLARE_QUANTITY(Seconds, int32_t, "s");

QS_LITERAL_OP_ULL(Seconds, _sec)

using Speed = QS_COMBINE_UNITS(Length, /, Frame);

QS_LITERAL_OP_ULL(Speed, _spd)

using Acceleration = QS_COMBINE_UNITS(Speed, /, Frame);

inline auto sqrt(QS_COMBINE_UNITS(Acceleration, *, Length) value)
{
  return Speed{static_cast<Speed::type>(std::sqrt(value.get()))};
}
} // namespace core

using core::operator""_frame;
using core::operator""_hp;
using core::operator""_len;
using core::operator""_sec;
using core::operator""_spd;
