#pragma once

#include "loader/datatypes.h"
#include "util/vmath.h"

#include <array>
#include <numeric>

namespace world
{
class Room;

struct Portal
{
    Portal(const loader::Portal& portal, Room* src, Room* dest, const irr::core::matrix4& transform)
        : vertices{ {} }
        , normal{ util::convert(portal.normal) }
        , center{}
        , source{ src }
        , destination{ dest }
    {
        for(int j = 0; j < 4; ++j)
        {
            vertices[j] = util::convert(portal.vertices[j]) + transform.getTranslation();
        }
        center = std::accumulate(vertices.begin(), vertices.end(), irr::core::vector3df(0, 0, 0)) / static_cast<irr::f32>(vertices.size());
    }

    std::array<irr::core::vector3df, 4> vertices;
    irr::core::vector3df normal;
    irr::core::vector3df center = { 0,0,0 };
    Room* source = nullptr;
    Room* destination = nullptr;
};
} // namespace world
