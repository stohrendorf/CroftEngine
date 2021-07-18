#include "raycast.h"

#include "heightinfo.h"
#include "objectmanager.h"
#include "world/room.h"

#include <algorithm>

namespace engine
{
namespace
{
bool clampY(const core::TRVec& start,
            RoomBoundPosition& goal,
            const gsl::not_null<const world::Sector*>& sector,
            const ObjectManager& objectManager)
{
  const auto delta = goal.position - start;

  const auto goalFloor = HeightInfo::fromFloor(sector, goal.position, objectManager.getObjects()).y;
  if(goalFloor < goal.position.Y && goalFloor > start.Y)
  {
    goal.position.Y = goalFloor;
    const auto dy = goalFloor - start.Y;
    goal.position.X = delta.X * dy / delta.Y + start.X;
    goal.position.Z = delta.Z * dy / delta.Y + start.Z;
    world::findRealFloorSector(goal);
    return false;
  }

  const auto goalCeiling = HeightInfo::fromCeiling(sector, goal.position, objectManager.getObjects()).y;
  if(goalCeiling > goal.position.Y && goalCeiling < start.Y)
  {
    goal.position.Y = goalCeiling;
    const auto dy = goalCeiling - start.Y;
    goal.position.X = delta.X * dy / delta.Y + start.X;
    goal.position.Z = delta.Z * dy / delta.Y + start.Z;
    world::findRealFloorSector(goal);
    return false;
  }

  return true;
}

enum class CollisionType
{
  Vertical, // resulting position collides with ceiling or floor
  Wall,     // resulting position is valid but did not reach the goal
  None      // resulting position is valid and needs no further adjustment
};

std::pair<CollisionType, RoomBoundPosition> clampSteps(const RoomBoundPosition& start,
                                                       const core::TRVec& goal,
                                                       const ObjectManager& objectManager,
                                                       core::Length(core::TRVec::*stepAxis),
                                                       core::Length(core::TRVec::*secondaryAxis))
{
  const auto delta = goal - start.position;
  if(delta.*stepAxis == 0_len)
  {
    return {CollisionType::None, RoomBoundPosition{start.room, goal}};
  }

  const auto dir = delta.*stepAxis < 0_len ? -1 : 1;
  core::TRVec sectorStep;
  sectorStep.*stepAxis = dir * core::SectorSize;
  sectorStep.*secondaryAxis = delta.*secondaryAxis * sectorStep.*stepAxis / delta.*stepAxis;
  sectorStep.Y = delta.Y * sectorStep.*stepAxis / delta.*stepAxis;

  auto result = start;
  // align the result to the sector boundary, adjust other axes as necessary
  result.position.*stepAxis = (result.position.*stepAxis / core::SectorSize) * core::SectorSize;
  if(dir > 0)
    result.position.*stepAxis += core::SectorSize - 1_len;

  const auto deltaStep = result.position.*stepAxis - start.position.*stepAxis;
  result.position.*secondaryAxis += sectorStep.*secondaryAxis * deltaStep / sectorStep.*stepAxis;
  result.position.Y += sectorStep.Y * deltaStep / sectorStep.*stepAxis;

  auto testVerticalHit = [&objectManager](RoomBoundPosition& pos)
  {
    const auto sector = world::findRealFloorSector(pos);
    const auto floor = HeightInfo::fromFloor(sector, pos.position, objectManager.getObjects()).y;
    const auto ceiling = HeightInfo::fromCeiling(sector, pos.position, objectManager.getObjects()).y;
    return pos.position.Y > floor || pos.position.Y < ceiling;
  };

  while(true)
  {
    if(dir > 0 && result.position.*stepAxis >= goal.*stepAxis)
    {
      return {CollisionType::None, RoomBoundPosition{result.room, goal}};
    }
    if(dir < 0 && result.position.*stepAxis <= goal.*stepAxis)
    {
      return {CollisionType::None, RoomBoundPosition{result.room, goal}};
    }

    if(testVerticalHit(result))
    {
      return {CollisionType::Vertical, result};
    }

    auto nextSector = result;
    nextSector.position.*stepAxis += dir * 1_len;
    BOOST_ASSERT(result.position.*stepAxis / core::SectorSize != nextSector.position.*stepAxis / core::SectorSize);
    const auto oldResult = result;
    if(testVerticalHit(nextSector))
    {
      return {CollisionType::Wall, oldResult};
    }

    result.room = nextSector.room;
    result.position += sectorStep;
  }
}

} // namespace

std::pair<bool, RoomBoundPosition>
  raycastLineOfSight(const RoomBoundPosition& start, const core::TRVec& goal, const ObjectManager& objectManager)
{
  auto collide
    = [&start, &goal, &objectManager](
        core::Length(core::TRVec::*firstStepAxis),
        core::Length(core::TRVec::*secondStepAxis)) -> std::tuple<CollisionType, CollisionType, RoomBoundPosition>
  {
    auto [firstType, firstPos] = clampSteps(start, goal, objectManager, firstStepAxis, secondStepAxis);
    auto [secondType, secondPos] = clampSteps(start, firstPos.position, objectManager, secondStepAxis, firstStepAxis);
    BOOST_ASSERT(secondPos.room->getSectorByAbsolutePosition(secondPos.position) != nullptr);
    return {firstType, secondType, secondPos};
  };

  auto [firstCollision, secondCollision, result] = abs(goal.Z - start.position.Z) <= abs(goal.X - start.position.X)
                                                     ? collide(&core::TRVec::Z, &core::TRVec::X)
                                                     : collide(&core::TRVec::X, &core::TRVec::Z);
  const auto invariantCheck = gsl::finally(
    [&result = result]() { Ensures(result.room->getSectorByAbsolutePosition(result.position) != nullptr); });

  if(secondCollision == CollisionType::Wall)
  {
    return {false, result};
  }

  const auto sector = world::findRealFloorSector(result);
  bool success = clampY(start.position, result, sector, objectManager) && firstCollision == CollisionType::None
                 && secondCollision == CollisionType::None;
  return {success, result};
}
} // namespace engine
