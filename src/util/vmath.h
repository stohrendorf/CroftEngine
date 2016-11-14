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
        v *= glm::quat(rotation.z, {0,0,1});
        v *= glm::quat(rotation.x, {1,0,0});
        v *= glm::quat(rotation.y, {0,1,0});
        return v;
    }


    inline glm::quat xyzToYpr(const int16_t* animData)
    {
        const uint32_t angle = *reinterpret_cast<const uint32_t*>(animData);
        glm::quat v;
        v *= glm::quat(auToRad(((angle >> 00) & 0x3ff) * 64), {0,0,1});
        v *= glm::quat(auToRad(((angle >> 20) & 0x3ff) * 64), {1,0,0});
        v *= glm::quat(auToRad(((angle >> 10) & 0x3ff) * 64), {0,1,0});
        return v;
    }


    inline glm::mat4& rotateCompressed(glm::mat4& matrix, const int16_t* animData)
    {
        const uint32_t angle = *reinterpret_cast<const uint32_t*>(animData);
        matrix = glm::rotate(matrix, util::auToRad(((angle >> 10) & 0x3ff) * 64), { 0, 1.0f, 0 });
        matrix = glm::rotate(matrix, util::auToRad(((angle >> 20) & 0x3ff) * 64), { 1.0f, 0, 0 });
        matrix = glm::rotate(matrix, util::auToRad(((angle >> 00) & 0x3ff) * 64), { 0, 0, 1.0f });
        return matrix;
    }


    inline glm::mat4 rotateCompressed(const int16_t* animData)
    {
        const uint32_t angle = *reinterpret_cast<const uint32_t*>(animData);
        glm::mat4 matrix{ 1.0f };
        matrix = glm::rotate(matrix, util::auToRad(((angle >> 10) & 0x3ff) * 64), { 0, 1.0f, 0 });
        matrix = glm::rotate(matrix, util::auToRad(((angle >> 20) & 0x3ff) * 64), { 1.0f, 0, 0 });
        matrix = glm::rotate(matrix, util::auToRad(((angle >> 00) & 0x3ff) * 64), { 0, 0, 1.0f });
        return matrix;
    }
} // namespace util
