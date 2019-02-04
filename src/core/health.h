#pragma once

#include "intunit.h"

namespace core
{
namespace detail
{
struct HealthTag
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

using Health = IntUnit<detail::HealthTag>;

constexpr Health operator "" _hp(unsigned long long value) noexcept
{
    return Health{static_cast<Health::int_type>(value)};
}
}

using core::operator ""_hp;
