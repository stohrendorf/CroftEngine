#pragma once

#include <gsl/gsl>

#include <cmath>

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
    inline T clamp(const T& v, const T& min, const T& max)
    {
        if( v < min )
        {
            return min;
        }
        else if( max < v )
        {
            return max;
        }
        else
        {
            return v;
        }
    }


    template<typename T>
    constexpr T square(T v)
    {
        return v * v;
    }
} // namespace util
