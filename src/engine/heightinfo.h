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
    int y = 0;
    SlantClass slantClass = SlantClass::None;
    const uint16_t* lastCommandSequenceOrDeath = nullptr;

    static bool skipSteepSlants;

    static HeightInfo fromFloor(gsl::not_null<const loader::Sector*> roomSector,
                                const core::TRVec& pos,
                                const std::map<uint16_t, gsl::not_null<std::shared_ptr<engine::items::ItemNode>>>& itemList);

    static HeightInfo fromCeiling(gsl::not_null<const loader::Sector*> roomSector,
                                  const core::TRVec& pos,
                                  const std::map<uint16_t, gsl::not_null<std::shared_ptr<engine::items::ItemNode>>>& itemList);

    HeightInfo() = default;
};


struct VerticalInfo
{
    HeightInfo floor;
    HeightInfo ceiling;

    void init(const gsl::not_null<const loader::Sector*>& roomSector,
              const core::TRVec& position,
              const std::map<uint16_t, gsl::not_null<std::shared_ptr<engine::items::ItemNode>>>& itemList,
              int floorHeight,
              int scalpHeight)
    {
        floor = HeightInfo::fromFloor( roomSector, position, itemList );
        if( floor.y != -loader::HeightLimit )
            floor.y -= floorHeight;

        ceiling = HeightInfo::fromCeiling( roomSector, position, itemList );
        if( ceiling.y != -loader::HeightLimit )
            ceiling.y -= floorHeight - scalpHeight;
    }
};
}
