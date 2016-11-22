#pragma once

#include <cstdint>

#include <glm/gtc/constants.hpp>


namespace util
{
    constexpr float auToDeg(int16_t au)
    {
        return au / 65536.0f * 360;
    }


    inline float auToRad(int16_t au)
    {
        return au / 65536.0f * 2 * glm::pi<float>();
    }
} // namespace util
