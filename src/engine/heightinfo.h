#pragma once

#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/floordata/types.h"
#include "qs/qs.h"

#include <cstdint>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <map>

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
enum class SlantClass : uint8_t
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
                              const std::map<uint16_t, gslu::nn_shared<objects::Object>>& objects);

  static HeightInfo fromCeiling(gsl::not_null<const world::Sector*> roomSector,
                                const core::TRVec& pos,
                                const std::map<uint16_t, gslu::nn_shared<objects::Object>>& objects);

  HeightInfo() = default;
};

struct RelativeHeightInfo
{
  core::Length dy = 0_len;
  SlantClass slantClass = SlantClass::None;
  const floordata::FloorDataValue* lastCommandSequenceOrDeath = nullptr;

  RelativeHeightInfo() = default;
};

struct VerticalDistances
{
  //! Floor distance relative to the object
  RelativeHeightInfo floor;
  //! Ceiling distance relative to the object's top
  RelativeHeightInfo ceiling;

  void init(const gsl::not_null<const world::Sector*>& roomSector,
            const core::TRVec& position,
            const std::map<uint16_t, gslu::nn_shared<objects::Object>>& objects,
            const core::Length& objectY,
            const core::Length& objectHeight)
  {
    const auto floorInfo = HeightInfo::fromFloor(roomSector, position, objects);
    floor.dy = floorInfo.y;
    floor.slantClass = floorInfo.slantClass;
    floor.lastCommandSequenceOrDeath = floorInfo.lastCommandSequenceOrDeath;
    if(floor.dy != core::InvalidHeight)
      floor.dy -= objectY;

    const auto ceilingInfo = HeightInfo::fromCeiling(roomSector, position, objects);
    ceiling.dy = ceilingInfo.y;
    ceiling.slantClass = ceilingInfo.slantClass;
    ceiling.lastCommandSequenceOrDeath = ceilingInfo.lastCommandSequenceOrDeath;
    if(ceiling.dy != core::InvalidHeight)
      ceiling.dy -= objectY - objectHeight;
  }
};
} // namespace engine
