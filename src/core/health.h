#pragma once

#include "unit.h"

namespace core
{
namespace detail
{
struct HealthUnit
{
    static const char* suffix()
    {
        return "hp";
    }
};
}

using Health = Quantity<detail::HealthUnit>;

constexpr Health operator "" _hp(unsigned long long value) noexcept
{
    return Health{static_cast<Health::type>(value)};
}
}

using core::operator ""_hp;
