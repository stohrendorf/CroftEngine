#pragma once

#include <gsl/gsl>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    return std::rand() & ((1 << 15) - 1);
}

template<typename T>
inline T rand15(T max)
{
    return max * rand15() / (1 << 15);
}

inline int16_t rand15s()
{
    return static_cast<int16_t>(rand15() - (1 << 14));
}

template<typename T>
inline T rand15s(T max)
{
    return max * rand15s() / 32768;
}

constexpr float auToDeg(const int16_t au)
{
    return au / 65536.0f * 360;
}

inline float auToRad(const int16_t au)
{
    return au / 65536.0f * 2 * glm::pi<float>();
}

inline glm::mat4 mix(const glm::mat4& a, const glm::mat4& b, const float bias)
{
    glm::mat4 result{0.0f};
    const auto ap = value_ptr( a );
    const auto bp = value_ptr( b );
    const auto rp = value_ptr( result );
    for( int i = 0; i < 16; ++i )
        rp[i] = ap[i] * (1 - bias) + bp[i] * bias;
    return result;
}
}
