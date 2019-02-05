#pragma once

#include "intunit.h"

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

    static const char* typeId()
    {
        return "frame";
    }
};
}

using Frame = IntQuantity<detail::FrameUnit>;

constexpr Frame operator "" _frame(unsigned long long value) noexcept
{
    return Frame{static_cast<Frame::int_type>(value)};
}
}

using core::operator ""_frame;
