#pragma once

#include "loader/datatypes.h"

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
    core::Length y = 0_len;
    SlantClass slantClass = SlantClass::None;
    const engine::floordata::FloorDataValue* lastCommandSequenceOrDeath = nullptr;

    static bool skipSteepSlants;

    static HeightInfo fromFloor(gsl::not_null<const loader::Sector*> roomSector,
                                const core::TRVec& pos,
                                const std::map<uint16_t, gsl::not_null<std::shared_ptr<items::ItemNode>>>& itemList);

    static HeightInfo fromCeiling(gsl::not_null<const loader::Sector*> roomSector,
                                  const core::TRVec& pos,
                                  const std::map<uint16_t, gsl::not_null<std::shared_ptr<items::ItemNode>>>& itemList);

    HeightInfo() = default;
};


struct VerticalInfo
{
    HeightInfo floor;
    HeightInfo ceiling;

    void init(const gsl::not_null<const loader::Sector*>& roomSector,
              const core::TRVec& position,
              const std::map<uint16_t, gsl::not_null<std::shared_ptr<items::ItemNode>>>& itemList,
              const core::Length floorHeight,
              const core::Length scalpHeight)
    {
        floor = HeightInfo::fromFloor( roomSector, position, itemList );
        if( floor.y != -core::HeightLimit )
            floor.y -= floorHeight;

        ceiling = HeightInfo::fromCeiling( roomSector, position, itemList );
        if( ceiling.y != -core::HeightLimit )
            ceiling.y -= floorHeight - scalpHeight;
    }
};
}
