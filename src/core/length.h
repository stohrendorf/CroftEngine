#pragma once

#include "intunit.h"

namespace core
{
namespace detail
{
struct LengthTag
{
    static const char* suffix()
    {
        return "u";
    }

    static const char* typeId()
    {
        return "length";
    }
};
}

using Length = IntUnit<detail::LengthTag>;
using Area = IntUnit<MultiTag<detail::LengthTag, 2>>;

inline Length sqrt(const Area& area)
{
    return Length{Length::int_type( std::sqrt( area.value ) )};
}

constexpr Length operator "" _len(unsigned long long value) noexcept
{
    return Length{static_cast<Length::int_type>(value)};
}

constexpr Length lerp(const Length& a, const Length& b, float bias)
{
    return Length{static_cast<Length::int_type>(a.value * (1 - bias) + b.value * bias)};
}

constexpr Length abs(const Length& v) noexcept
{
    return v >= 0_len ? v : -v;
}
}

using core::operator ""_len;
