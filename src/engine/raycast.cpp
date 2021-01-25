#include "raycast.h"

#include "heightinfo.h"
#include "loader/file/datatypes.h"
#include "objectmanager.h"

namespace engine
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

enum class ClampType
{
  Ceiling,
  Wall,
  None
};

ClampType clampAlongMajorMinor(const core::RoomBoundPosition& from,
                               core::RoomBoundPosition& goal,
                               const ObjectManager& objectManager,
                               core::Length(core::TRVec::*major),
                               core::Length(core::TRVec::*minor))
{
  if(goal.position.*major == from.position.*major)
  {
    return ClampType::None;
  }

  const auto delta = goal.position - from.position;
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

  goal.room = from.room;

  auto testHit = [&goal, &objectManager](const core::TRVec& pos) {
    const auto sector = findRealFloorSector(pos, &goal.room);
    const auto floor = HeightInfo::fromFloor(sector, pos, objectManager.getObjects()).y;
    const auto ceiling = HeightInfo::fromCeiling(sector, pos, objectManager.getObjects()).y;
    return pos.Y > floor || pos.Y < ceiling;
  };

  while(true)
  {
    if(dir > 0 && current.*major >= goal.position.*major)
    {
      return ClampType::None;
    }
    if(dir < 0 && current.*major <= goal.position.*major)
    {
      return ClampType::None;
    }

    if(testHit(current))
    {
      goal.position = current;
      return ClampType::Ceiling;
    }

    auto nextSector = current;
    nextSector.*major += dir * 1_len;
    BOOST_ASSERT(current.*major / core::SectorSize != nextSector.*major / core::SectorSize);
    if(testHit(nextSector))
    {
      goal.position = current;
      return ClampType::Wall;
    }

    current += step;
  }
}

ClampType
  clampAlongZ(const core::RoomBoundPosition& from, core::RoomBoundPosition& goal, const ObjectManager& objectManager)
{
  return clampAlongMajorMinor(from, goal, objectManager, &core::TRVec::Z, &core::TRVec::X);
}

ClampType
  clampAlongX(const core::RoomBoundPosition& from, core::RoomBoundPosition& goal, const ObjectManager& objectManager)
{
  return clampAlongMajorMinor(from, goal, objectManager, &core::TRVec::X, &core::TRVec::Z);
}

std::pair<bool, core::RoomBoundPosition>
  raycastLineOfSight(const core::RoomBoundPosition& start, const core::TRVec& goal, const ObjectManager& objectManager)
{
  core::RoomBoundPosition result{start.room, goal};
  bool firstUnclamped;
  ClampType secondClamp;
  if(abs(goal.Z - start.position.Z) <= abs(goal.X - start.position.X))
  {
    firstUnclamped = clampAlongZ(start, result, objectManager) == ClampType::None;
    secondClamp = clampAlongX(start, result, objectManager);
  }
  else
  {
    firstUnclamped = clampAlongX(start, result, objectManager) == ClampType::None;
    secondClamp = clampAlongZ(start, result, objectManager);
  }
  const auto invariantCheck
    = gsl::finally([&result]() { Ensures(result.room->getSectorByAbsolutePosition(result.position) != nullptr); });

  if(secondClamp == ClampType::Wall)
  {
    return {false, result};
  }

  const auto sector = loader::file::findRealFloorSector(result);
  bool success = clampY(start.position, result.position, sector, objectManager) && firstUnclamped
                 && secondClamp == ClampType::None;
  return {success, result};
}
} // namespace engine
