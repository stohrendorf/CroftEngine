#pragma once

#include <gsl/gsl>

#define ENUM_TO_OSTREAM(name) \
    inline std::ostream& operator<<(std::ostream& str, name e) \
    { \
        return str << static_cast<int>(e); \
    }
#define DISABLE_COPY(classname) \
    classname(const classname&) = delete; \
    classname& operator=(const classname&) = delete

namespace util
{
template<typename T>
T clamp(const T& v, const T& min, const T& max)
{
    if( v < min )
    {
        return min;
    }
    if( max < v )
    {
        return max;
    }
    return v;
}

template<typename T>
constexpr T square(T v)
{
    return v * v;
}

inline int16_t rand15()
{
    return std::rand() & 0x7fff;
}

template<typename T>
inline T rand15(T max)
{
    return max * rand15() / 32768;
}

inline int16_t rand15s()
{
    return static_cast<int16_t>(rand15() - 16384);
}

template<typename T>
inline T rand15s(T max)
{
    return max * rand15s() / 32768;
}

} // namespace util
