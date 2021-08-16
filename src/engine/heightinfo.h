#pragma once

#include "loader/file/datatypes.h"

namespace engine::world
{
struct Sector;
}

namespace engine::objects
{
class Object;
}

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
  const floordata::FloorDataValue* lastCommandSequenceOrDeath = nullptr;

  static bool skipSteepSlants;

  static HeightInfo fromFloor(gsl::not_null<const world::Sector*> roomSector,
                              const core::TRVec& pos,
                              const std::map<uint16_t, gsl::not_null<std::shared_ptr<objects::Object>>>& objects);

  static HeightInfo fromCeiling(gsl::not_null<const world::Sector*> roomSector,
                                const core::TRVec& pos,
                                const std::map<uint16_t, gsl::not_null<std::shared_ptr<objects::Object>>>& objects);

  HeightInfo() = default;
};

struct VerticalDistances
{
  HeightInfo floor;
  HeightInfo ceiling;

  void init(const gsl::not_null<const world::Sector*>& roomSector,
            const core::TRVec& position,
            const std::map<uint16_t, gsl::not_null<std::shared_ptr<objects::Object>>>& objects,
            const core::Length& objectY,
            const core::Length& objectHeight)
  {
    floor = HeightInfo::fromFloor(roomSector, position, objects);
    if(floor.y != core::InvalidHeight)
      floor.y -= objectY;

    ceiling = HeightInfo::fromCeiling(roomSector, position, objects);
    if(ceiling.y != core::InvalidHeight)
      ceiling.y -= objectY - objectHeight;
  }
};
} // namespace engine
