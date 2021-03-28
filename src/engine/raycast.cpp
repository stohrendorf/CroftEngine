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
            core::RoomBoundPosition& goal,
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

std::pair<CollisionType, core::RoomBoundPosition> clampSteps(const core::RoomBoundPosition& from,
                                                             const core::TRVec& goal,
                                                             const ObjectManager& objectManager,
                                                             core::Length(core::TRVec::*stepAxis),
                                                             core::Length(core::TRVec::*secondaryAxis))
{
  core::RoomBoundPosition result{from.room, goal};
  if(goal.*stepAxis == from.position.*stepAxis)
  {
    return {CollisionType::None, result};
  }

  const auto delta = goal - from.position;
  const auto dir = delta.*stepAxis < 0_len ? -1 : 1;

  core::TRVec current;
  current.*stepAxis = (from.position.*stepAxis / core::SectorSize) * core::SectorSize;
  if(dir > 0)
    current.*stepAxis += core::SectorSize - 1_len;

  current.*secondaryAxis = from.position.*secondaryAxis
                           + (current.*stepAxis - from.position.*stepAxis) * delta.*secondaryAxis / delta.*stepAxis;
  current.Y = from.position.Y + (current.*stepAxis - from.position.*stepAxis) * delta.Y / delta.*stepAxis;

  core::TRVec step;
  step.*stepAxis = dir * core::SectorSize;
  step.*secondaryAxis = step.*stepAxis * delta.*secondaryAxis / delta.*stepAxis;
  step.Y = step.*stepAxis * delta.Y / delta.*stepAxis;

  auto testVerticalHit = [&result, &objectManager](const core::TRVec& pos) {
    const auto sector = findRealFloorSector(pos, &result.room);
    const auto floor = HeightInfo::fromFloor(sector, pos, objectManager.getObjects()).y;
    const auto ceiling = HeightInfo::fromCeiling(sector, pos, objectManager.getObjects()).y;
    return pos.Y > floor || pos.Y < ceiling;
  };

  while(true)
  {
    if(dir > 0 && current.*stepAxis >= result.position.*stepAxis)
    {
      return {CollisionType::None, result};
    }
    if(dir < 0 && current.*stepAxis <= result.position.*stepAxis)
    {
      return {CollisionType::None, result};
    }

    if(testVerticalHit(current))
    {
      result.position = current;
      return {CollisionType::Vertical, result};
    }

    auto nextSector = current;
    nextSector.*stepAxis += dir * 1_len;
    BOOST_ASSERT(current.*stepAxis / core::SectorSize != nextSector.*stepAxis / core::SectorSize);
    const auto oldResult = result;
    if(testVerticalHit(nextSector))
    {
      return {CollisionType::Wall, oldResult};
    }

    current += step;
  }
}

} // namespace

std::pair<bool, core::RoomBoundPosition>
  raycastLineOfSight(const core::RoomBoundPosition& start, const core::TRVec& goal, const ObjectManager& objectManager)
{
  auto collide =
    [&start, &goal, &objectManager](
      core::Length(core::TRVec::*firstStepAxis),
      core::Length(core::TRVec::*secondStepAxis)) -> std::tuple<CollisionType, CollisionType, core::RoomBoundPosition> {
    auto [firstType, firstPos] = clampSteps(start, goal, objectManager, firstStepAxis, secondStepAxis);
    auto [secondType, secondPos] = clampSteps(start, firstPos.position, objectManager, secondStepAxis, firstStepAxis);
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
