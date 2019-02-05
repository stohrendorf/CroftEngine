#pragma once

#include "intunit.h"

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

    static const char* typeId()
    {
        return "health";
    }
};
}

using Health = IntQuantity<detail::HealthUnit>;

constexpr Health operator "" _hp(unsigned long long value) noexcept
{
    return Health{static_cast<Health::int_type>(value)};
}
}

using core::operator ""_hp;
