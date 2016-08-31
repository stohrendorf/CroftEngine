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
} // namespace util
