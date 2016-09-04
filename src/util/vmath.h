#pragma once

#include "gameplay.h"


namespace util
{
    constexpr float auToDeg(int16_t au)
    {
        return au / 65536.0f * 360;
    }


    constexpr float auToRad(int16_t au)
    {
        return au / 65536.0f * 2 * MATH_PI;
    }


    inline gameplay::Quaternion xyzToYpr(const gameplay::Vector3& rotation)
    {
        gameplay::Quaternion v;
        v *= gameplay::Quaternion({0,0,1}, rotation.z);
        v *= gameplay::Quaternion({1,0,0}, rotation.x);
        v *= gameplay::Quaternion({0,1,0}, rotation.y);
        return v;
    }
} // namespace util
