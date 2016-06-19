#pragma once

#include "loader/datatypes.h"

class CameraController;

enum class SlantClass
{
    None,
    Max512,
    Steep
};

struct HeightInfo
{
    int distance = 0;
    SlantClass slantClass = SlantClass::None;
    const uint16_t* lastTriggerOrKill = nullptr;

    static bool skipSteepSlants;

    static HeightInfo fromFloor(gsl::not_null<const loader::Sector*>roomSector, const loader::TRCoordinates& pos, const CameraController* camera);

    static HeightInfo fromCeiling(gsl::not_null<const loader::Sector*>roomSector, const loader::TRCoordinates& pos, const CameraController* camera);

    HeightInfo() = default;
};

struct VerticalInfo
{
    HeightInfo floor;
    HeightInfo ceiling;

    void init(const loader::Sector* roomSector, const loader::TRCoordinates& position, const CameraController* camera, int scalpHeight)
    {
        floor = HeightInfo::fromFloor(roomSector, position, camera);
        if(floor.distance != -loader::HeightLimit)
            floor.distance -= position.Y;

        ceiling = HeightInfo::fromCeiling(roomSector, position, camera);
        if(ceiling.distance != -loader::HeightLimit)
            ceiling.distance -= position.Y - scalpHeight;
    }
};

