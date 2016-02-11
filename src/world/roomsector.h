#pragma once

#include "object.h"

#include <boost/optional.hpp>
#include <irrlicht.h>

#include <cstdint>

namespace world
{

class Room;

enum class PenetrationConfig;
enum class DiagonalType;

struct RoomSector
{
    uint32_t                    trig_index; // Trigger function index.
    int32_t                     box_index;

    uint32_t                    flags;      // Climbability, death etc.
    uint32_t                    material;   // Footstep sound and footsteps.

    int32_t                     floor;
    int32_t                     ceiling;

    const RoomSector* sector_below;
    const RoomSector* sector_above;
    Room* owner_room;    // Room that contain this sector

    size_t index_x;
    size_t index_y;
    irr::core::vector3df position;

    irr::core::vector3df        ceiling_corners[4];
    DiagonalType                ceiling_diagonal_type;
    PenetrationConfig           ceiling_penetration_config;

    irr::core::vector3df        floor_corners[4];
    DiagonalType                floor_diagonal_type;
    PenetrationConfig           floor_penetration_config;

    boost::optional<ObjectId> portal_to_room;

    const RoomSector* getLowestSector() const;
    const RoomSector* getHighestSector() const;

    const RoomSector* checkFlip() const;
    const RoomSector* checkBaseRoom() const;
    const RoomSector* checkAlternateRoom() const;
    const RoomSector* checkPortalPointerRaw(const World& world);
    const RoomSector* checkPortalPointer(const World& world) const;
    bool is2SidePortals(const World& world, const RoomSector* s2) const;
    bool similarCeiling(const RoomSector* s2, bool ignore_doors) const;
    bool similarFloor(const RoomSector* s2, bool ignore_doors) const;
    irr::core::vector3df getFloorPoint() const;
    irr::core::vector3df getCeilingPoint() const;

    irr::core::vector3df getHighestFloorCorner() const
    {
        auto r1 = floor_corners[0].Z > floor_corners[1].Z ? floor_corners[0] : floor_corners[1];
        auto r2 = floor_corners[2].Z > floor_corners[3].Z ? floor_corners[2] : floor_corners[3];

        return r1.Z > r2.Z ? r1 : r2;
    }

    irr::core::vector3df getLowestCeilingCorner() const
    {
        auto r1 = ceiling_corners[0].Z > ceiling_corners[1].Z ? ceiling_corners[1] : ceiling_corners[0];
        auto r2 = ceiling_corners[2].Z > ceiling_corners[3].Z ? ceiling_corners[3] : ceiling_corners[2];

        return r1.Z > r2.Z ? r2 : r1;
    }
};

}
