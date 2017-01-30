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
    inline bool fuzzyEqual(T a, T b, T margin) noexcept
    {
        Expects(margin > 0);
        return std::abs(a - b) <= margin;
    }


    template<typename T>
    inline bool fuzzyOne(T value) noexcept
    {
        return fuzzyEqual(value, static_cast<T>(1));
    }


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
