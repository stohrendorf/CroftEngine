#pragma once

#include <cstdint>
#include <cmath>
#include <chrono>

#define ENUM_TO_OSTREAM(name) \
    inline std::ostream& operator<<(std::ostream& str, name e) \
    { \
        return str << static_cast<int>(e); \
    } \

#define DISABLE_COPY(classname) \
    classname(const classname&) = delete; \
    classname& operator=(const classname&) = delete

namespace util
{
template<typename T>
inline bool fuzzyZero(T value) noexcept
{
    return std::abs(value) <= std::numeric_limits<T>::epsilon();
}

template<typename T>
inline bool fuzzyEqual(T a, T b) noexcept
{
    return fuzzyZero(a - b);
}

template<typename T>
inline bool fuzzyOne(T value) noexcept
{
    return fuzzyEqual(value, static_cast<T>(1));
}

using ClockType = std::chrono::high_resolution_clock;

using FloatDuration = float;
using Duration = std::chrono::duration<FloatDuration, std::chrono::nanoseconds::period>;
using Seconds = std::chrono::duration<FloatDuration, std::chrono::seconds::period>;
using MilliSeconds = std::chrono::duration<FloatDuration, std::chrono::milliseconds::period>;

using TimePoint = ClockType::time_point;

constexpr inline FloatDuration toSeconds(Duration d) noexcept
{
    return d.count() * static_cast<FloatDuration>(Duration::period::num) / static_cast<FloatDuration>(Duration::period::den);
}

constexpr inline util::Duration fromSeconds(FloatDuration d) noexcept
{
    return Duration(d * static_cast<FloatDuration>(Duration::period::den) / static_cast<FloatDuration>(Duration::period::num));
}

inline TimePoint now() noexcept
{
    return ClockType::now();
}

} // namespace util
