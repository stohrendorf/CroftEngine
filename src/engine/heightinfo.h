#pragma once

#include "loader/file/datatypes.h"

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

  static HeightInfo fromFloor(gsl::not_null<const loader::file::Sector*> roomSector,
                              const core::TRVec& pos,
                              const std::map<uint16_t, gsl::not_null<std::shared_ptr<items::ItemNode>>>& itemList);

  static HeightInfo fromCeiling(gsl::not_null<const loader::file::Sector*> roomSector,
                                const core::TRVec& pos,
                                const std::map<uint16_t, gsl::not_null<std::shared_ptr<items::ItemNode>>>& itemList);

  HeightInfo() = default;
};

struct VerticalSpaceInfo
{
  HeightInfo floorSpace;
  HeightInfo ceilingSpace;

  void init(const gsl::not_null<const loader::file::Sector*>& roomSector,
            const core::TRVec& position,
            const std::map<uint16_t, gsl::not_null<std::shared_ptr<items::ItemNode>>>& itemList,
            const core::Length& itemY,
            const core::Length& itemHeight)
  {
    floorSpace = HeightInfo::fromFloor(roomSector, position, itemList);
    if(floorSpace.y != -core::HeightLimit)
      floorSpace.y -= itemY;

    ceilingSpace = HeightInfo::fromCeiling(roomSector, position, itemList);
    if(ceilingSpace.y != -core::HeightLimit)
      ceilingSpace.y -= itemY - itemHeight;
  }
};
} // namespace engine
