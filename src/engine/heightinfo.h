#pragma once

#include "loader/datatypes.h"
#include "engine/floordata/floordata.h"

namespace engine
{
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
    const uint16_t* lastCommandSequenceOrDeath = nullptr;

    static bool skipSteepSlants;

    static HeightInfo fromFloor(gsl::not_null<const loader::Sector*> roomSector,
                                const core::TRCoordinates& pos,
                                const std::map<uint16_t, std::shared_ptr<engine::items::ItemNode>>& itemList);

    static HeightInfo fromCeiling(gsl::not_null<const loader::Sector*> roomSector,
                                  const core::TRCoordinates& pos,
                                  const std::map<uint16_t, std::shared_ptr<engine::items::ItemNode>>& itemList);

    HeightInfo() = default;
};


struct VerticalInfo
{
    HeightInfo floor;
    HeightInfo ceiling;

    void init(const gsl::not_null<const loader::Sector*>& roomSector,
              const core::TRCoordinates& position,
              const std::map<uint16_t, std::shared_ptr<engine::items::ItemNode>>& itemList,
              int floorHeight,
              int scalpHeight)
    {
        floor = HeightInfo::fromFloor( roomSector, position, itemList );
        if( floor.distance != -loader::HeightLimit )
            floor.distance -= floorHeight;

        ceiling = HeightInfo::fromCeiling( roomSector, position, itemList );
        if( ceiling.distance != -loader::HeightLimit )
            ceiling.distance -= floorHeight - scalpHeight;
    }
};
}
