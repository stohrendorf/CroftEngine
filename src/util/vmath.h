#pragma once

#include "gameplay.h"

#include <glm/gtc/matrix_transform.hpp>

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
        v *= glm::angleAxis(rotation.z, glm::vec3{0,0,1});
        v *= glm::angleAxis(rotation.x, glm::vec3{1,0,0});
        v *= glm::angleAxis(rotation.y, glm::vec3{0,1,0});
        return v;
    }


    inline glm::quat xyzToYpr(const int16_t* animData)
    {
        const uint32_t angle = *reinterpret_cast<const uint32_t*>(animData);
        glm::quat v;
        v *= glm::angleAxis(auToRad(((angle >> 00) & 0x3ff) * 64), glm::vec3{0,0,1});
        v *= glm::angleAxis(auToRad(((angle >> 20) & 0x3ff) * 64), glm::vec3{1,0,0});
        v *= glm::angleAxis(auToRad(((angle >> 10) & 0x3ff) * 64), glm::vec3{0,1,0});
        return v;
    }


    inline glm::mat4 xyzToYprMatrix(const int16_t* animData)
    {
        const uint32_t angle = *reinterpret_cast<const uint32_t*>(animData);
        glm::mat4 m{1.0f};
        m = glm::rotate(m, auToRad(((angle >> 10) & 0x3ff) * 64), {0,1,0});
        m = glm::rotate(m, auToRad(((angle >> 20) & 0x3ff) * 64), {1,0,0});
        m = glm::rotate(m, auToRad(((angle >> 00) & 0x3ff) * 64), {0,0,1});
        return m;
    }
} // namespace util
