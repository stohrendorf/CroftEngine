#pragma once

#include "unit.h"

namespace core
{
namespace detail
{
struct FrameUnit
{
    static const char* suffix()
    {
        return "frame";
    }
};
}

using Frame = Quantity<detail::FrameUnit>;

constexpr Frame operator "" _frame(unsigned long long value) noexcept
{
    return Frame{static_cast<Frame::type>(value)};
}
}

using core::operator ""_frame;
