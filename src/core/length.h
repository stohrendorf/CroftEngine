#pragma once

#include "unit.h"

namespace core
{
namespace detail
{
struct LengthUnit
{
    static const char* suffix()
    {
        return "u";
    }
};
}

using Length = Quantity<detail::LengthUnit>;
using Area = Quantity<UnitExp<detail::LengthUnit, 2>>;

inline Length sqrt(const Area& area)
{
    return Length{static_cast<Length::type>( std::sqrt( area.value ) )};
}

constexpr Length operator "" _len(unsigned long long value) noexcept
{
    return Length{static_cast<Length::type>(value)};
}

constexpr Length lerp(const Length& a, const Length& b, float bias)
{
    return Length{static_cast<Length::type>(a.value * (1 - bias) + b.value * bias)};
}
}

using core::operator ""_len;
