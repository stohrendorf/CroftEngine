#include "raycast.h"

#include "heightinfo.h"
#include "loader/file/datatypes.h"
#include "objectmanager.h"

#include <algorithm>

namespace engine
{
namespace
{
bool clampY(const core::TRVec& start,
            core::TRVec& goal,
            const gsl::not_null<const loader::file::Sector*>& sector,
            const ObjectManager& objectManager)
{
  const auto delta = goal - start;

  const HeightInfo floor = HeightInfo::fromFloor(sector, goal, objectManager.getObjects());
  if(floor.y < goal.Y && floor.y > start.Y)
  {
    goal.Y = floor.y;
    const auto dy = floor.y - start.Y;
    goal.X = delta.X * dy / delta.Y + start.X;
    goal.Z = delta.Z * dy / delta.Y + start.Z;
    return false;
  }

  const HeightInfo ceiling = HeightInfo::fromCeiling(sector, goal, objectManager.getObjects());
  if(ceiling.y > goal.Y && ceiling.y < start.Y)
  {
    goal.Y = ceiling.y;
    const auto dy = ceiling.y - start.Y;
    goal.X = delta.X * dy / delta.Y + start.X;
    goal.Z = delta.Z * dy / delta.Y + start.Z;
    return false;
  }

  return true;
}

enum class CollisionType
{
  Ceiling,
  Wall,
  None
};

std::pair<CollisionType, core::RoomBoundPosition> clampAlongMajorMinor(const core::RoomBoundPosition& from,
                                                                       const core::TRVec& goal,
                                                                       const ObjectManager& objectManager,
                                                                       core::Length(core::TRVec::*major),
                                                                       core::Length(core::TRVec::*minor))
{
  core::RoomBoundPosition result{from.room, goal};
  if(goal.*major == from.position.*major)
  {
    return {CollisionType::None, result};
  }

  const auto delta = goal - from.position;
  const auto dir = delta.*major < 0_len ? -1 : 1;

  core::TRVec current;
  current.*major = (from.position.*major / core::SectorSize) * core::SectorSize;
  if(dir > 0)
    current.*major += core::SectorSize - 1_len;

  current.*minor = from.position.*minor + (current.*major - from.position.*major) * delta.*minor / delta.*major;
  current.Y = from.position.Y + (current.*major - from.position.*major) * delta.Y / delta.*major;

  core::TRVec step;
  step.*major = dir * core::SectorSize;
  step.*minor = step.*major * delta.*minor / delta.*major;
  step.Y = step.*major * delta.Y / delta.*major;

  auto testHit = [&result, &objectManager](const core::TRVec& pos) {
    const auto sector = findRealFloorSector(pos, &result.room);
    const auto floor = HeightInfo::fromFloor(sector, pos, objectManager.getObjects()).y;
    const auto ceiling = HeightInfo::fromCeiling(sector, pos, objectManager.getObjects()).y;
    return pos.Y > floor || pos.Y < ceiling;
  };

  while(true)
  {
    if(dir > 0 && current.*major >= result.position.*major)
    {
      return {CollisionType::None, result};
    }
    if(dir < 0 && current.*major <= result.position.*major)
    {
      return {CollisionType::None, result};
    }

    if(testHit(current))
    {
      result.position = current;
      return {CollisionType::Ceiling, result};
    }

    auto nextSector = current;
    nextSector.*major += dir * 1_len;
    BOOST_ASSERT(current.*major / core::SectorSize != nextSector.*major / core::SectorSize);
    if(testHit(nextSector))
    {
      result.position = current;
      return {CollisionType::Wall, result};
    }

    current += step;
  }
}

} // namespace

std::pair<bool, core::RoomBoundPosition>
  raycastLineOfSight(const core::RoomBoundPosition& start, const core::TRVec& goal, const ObjectManager& objectManager)
{
  auto collide = [&start, &goal, &objectManager](core::Length(core::TRVec::*first), core::Length(core::TRVec::*second))
    -> std::tuple<CollisionType, CollisionType, core::RoomBoundPosition> {
    auto [firstType, firstPos] = clampAlongMajorMinor(start, goal, objectManager, first, second);
    auto [secondType, secondPos] = clampAlongMajorMinor(start, firstPos.position, objectManager, second, first);
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

  const auto sector = loader::file::findRealFloorSector(result);
  bool success = clampY(start.position, result.position, sector, objectManager) && firstCollision == CollisionType::None
                 && secondCollision == CollisionType::None;
  return {success, result};
}
} // namespace engine
