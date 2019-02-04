#pragma once

#include "intunit.h"

namespace core
{
namespace detail
{
struct FrameTag
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

using Frame = IntUnit<detail::FrameTag>;

constexpr Frame operator "" _frame(unsigned long long value) noexcept
{
    return Frame{static_cast<Frame::int_type>(value)};
}
}

using core::operator ""_frame;
