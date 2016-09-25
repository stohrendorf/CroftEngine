#pragma once

#include "gameplay.h"


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


    inline glm::quat xyzToYpr(const glm::vec3& rotation)
    {
        glm::quat v;
        v *= glm::quat(rotation.z, {0,0,1});
        v *= glm::quat(rotation.x, {1,0,0});
        v *= glm::quat(rotation.y, {0,1,0});
        return v;
    }
} // namespace util
