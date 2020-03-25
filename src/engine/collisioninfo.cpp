#include "collisioninfo.h"

#include "core/magic.h"
#include "objects/laraobject.h"

namespace engine
{
namespace
{
core::Length reflectAtSectorBoundary(const core::Length& target, const core::Length& current)
{
  const auto targetSector = target / core::SectorSize;
  const auto currentSector = current / core::SectorSize;
  if(targetSector == currentSector)
    return 0_len;

  const auto targetInSector = target % core::SectorSize;
  if(currentSector <= targetSector)
    return -(targetInSector + 1_len);
  return core::SectorSize - (targetInSector - 1_len);
}
} // namespace

void CollisionInfo::initHeightInfo(const core::TRVec& laraPos, const Engine& engine, const core::Length& height)
{
  collisionType = AxisColl::None;
  shift = core::TRVec{};
  facingAxis = *axisFromAngle(facingAngle, 45_deg);

  auto room = engine.getObjectManager().getLara().m_state.position.room;
  const auto refTestPos = laraPos - core::TRVec(0_len, height + core::ScalpToHandsHeight, 0_len);
  const auto currentSector = findRealFloorSector(refTestPos, &room);

  mid.init(currentSector, refTestPos, engine.getObjectManager().getObjects(), laraPos.Y, height);

  std::tie(floorSlantX, floorSlantZ) = engine.getFloorSlantInfo(currentSector, laraPos);

  core::Length frontX = 0_len, frontZ = 0_len;
  core::Length frontLeftX = 0_len, frontLeftZ = 0_len;
  core::Length frontRightX = 0_len, frontRightZ = 0_len;

  switch(facingAxis)
  {
  case core::Axis::PosZ:
    frontX = util::sin(collisionRadius, facingAngle);
    frontZ = collisionRadius;
    frontLeftZ = collisionRadius;
    frontLeftX = -collisionRadius;
    frontRightX = collisionRadius;
    frontRightZ = collisionRadius;
    break;
  case core::Axis::PosX:
    frontX = collisionRadius;
    frontZ = util::cos(collisionRadius, facingAngle);
    frontLeftX = collisionRadius;
    frontLeftZ = collisionRadius;
    frontRightX = collisionRadius;
    frontRightZ = -collisionRadius;
    break;
  case core::Axis::NegZ:
    frontX = util::sin(collisionRadius, facingAngle);
    frontZ = -collisionRadius;
    frontLeftX = collisionRadius;
    frontLeftZ = -collisionRadius;
    frontRightX = -collisionRadius;
    frontRightZ = -collisionRadius;
    break;
  case core::Axis::NegX:
    frontX = -collisionRadius;
    frontZ = util::cos(collisionRadius, facingAngle);
    frontLeftX = -collisionRadius;
    frontLeftZ = -collisionRadius;
    frontRightX = -collisionRadius;
    frontRightZ = collisionRadius;
    break;
  }

  // Front
  auto testPos = refTestPos + core::TRVec(frontX, 0_len, frontZ);
  auto sector = findRealFloorSector(testPos, &room);
  front.init(sector, testPos, engine.getObjectManager().getObjects(), laraPos.Y, height);
  if(policyFlags.is_set(PolicyFlags::SlopesAreWalls) && front.floorSpace.slantClass == SlantClass::Steep
     && front.floorSpace.y < 0_len)
  {
    front.floorSpace.y = -32767_len; // This is not a typo, it is really -32767
  }
  else if(front.floorSpace.y > 0_len
          && ((policyFlags.is_set(PolicyFlags::SlopesArePits) && front.floorSpace.slantClass == SlantClass::Steep)
              || (policyFlags.is_set(PolicyFlags::LavaIsPit) && front.floorSpace.lastCommandSequenceOrDeath != nullptr
                  && floordata::FloorDataChunk::extractType(*front.floorSpace.lastCommandSequenceOrDeath)
                       == floordata::FloorDataChunkType::Death)))
  {
    front.floorSpace.y = 2 * core::QuarterSectorSize;
  }

  // Front left
  testPos = refTestPos + core::TRVec(frontLeftX, 0_len, frontLeftZ);
  sector = findRealFloorSector(testPos, &room);
  frontLeft.init(sector, testPos, engine.getObjectManager().getObjects(), laraPos.Y, height);

  if(policyFlags.is_set(PolicyFlags::SlopesAreWalls) && frontLeft.floorSpace.slantClass == SlantClass::Steep
     && frontLeft.floorSpace.y < 0_len)
  {
    frontLeft.floorSpace.y = -32767_len; // This is not a typo, it is really -32767
  }
  else if(frontLeft.floorSpace.y > 0_len
          && ((policyFlags.is_set(PolicyFlags::SlopesArePits) && frontLeft.floorSpace.slantClass == SlantClass::Steep)
              || (policyFlags.is_set(PolicyFlags::LavaIsPit)
                  && frontLeft.floorSpace.lastCommandSequenceOrDeath != nullptr
                  && floordata::FloorDataChunk::extractType(*frontLeft.floorSpace.lastCommandSequenceOrDeath)
                       == floordata::FloorDataChunkType::Death)))
  {
    frontLeft.floorSpace.y = 2 * core::QuarterSectorSize;
  }

  // Front right
  testPos = refTestPos + core::TRVec(frontRightX, 0_len, frontRightZ);
  sector = findRealFloorSector(testPos, &room);
  frontRight.init(sector, testPos, engine.getObjectManager().getObjects(), laraPos.Y, height);

  if(policyFlags.is_set(PolicyFlags::SlopesAreWalls) && frontRight.floorSpace.slantClass == SlantClass::Steep
     && frontRight.floorSpace.y < 0_len)
  {
    frontRight.floorSpace.y = -32767_len; // This is not a typo, it is really -32767
  }
  else if(frontRight.floorSpace.y > 0_len
          && ((policyFlags.is_set(PolicyFlags::SlopesArePits) && frontRight.floorSpace.slantClass == SlantClass::Steep)
              || (policyFlags.is_set(PolicyFlags::LavaIsPit)
                  && frontRight.floorSpace.lastCommandSequenceOrDeath != nullptr
                  && floordata::FloorDataChunk::extractType(*frontRight.floorSpace.lastCommandSequenceOrDeath)
                       == floordata::FloorDataChunkType::Death)))
  {
    frontRight.floorSpace.y = 2 * core::QuarterSectorSize;
  }

  checkStaticMeshCollisions(laraPos, height, engine);

  if(mid.floorSpace.y == -core::HeightLimit)
  {
    shift = oldPosition - laraPos;
    collisionType = AxisColl::Front;
    return;
  }

  if(mid.floorSpace.y <= mid.ceilingSpace.y)
  {
    collisionType = AxisColl::TopFront;
    shift = oldPosition - laraPos;
    return;
  }

  if(mid.ceilingSpace.y >= 0_len)
  {
    collisionType = AxisColl::Top;
    shift.Y = mid.ceilingSpace.y;
  }

  if(front.floorSpace.y > badPositiveDistance || front.floorSpace.y < badNegativeDistance
     || front.ceilingSpace.y > badCeilingDistance)
  {
    collisionType = AxisColl::Front;
    switch(facingAxis)
    {
    case core::Axis::PosZ:
    case core::Axis::NegZ:
      shift.X = oldPosition.X - laraPos.X;
      shift.Z = reflectAtSectorBoundary(frontZ + laraPos.Z, laraPos.Z);
      break;
    case core::Axis::PosX:
    case core::Axis::NegX:
      shift.X = reflectAtSectorBoundary(frontX + laraPos.X, laraPos.X);
      shift.Z = oldPosition.Z - laraPos.Z;
      break;
    }
    return;
  }

  if(front.ceilingSpace.y >= badCeilingDistance)
  {
    collisionType = AxisColl::TopBottom;
    shift = oldPosition - laraPos;
    return;
  }

  if(frontLeft.floorSpace.y > badPositiveDistance || frontLeft.floorSpace.y < badNegativeDistance)
  {
    collisionType = AxisColl::Left;
    switch(facingAxis)
    {
    case core::Axis::PosZ:
    case core::Axis::NegZ: shift.X = reflectAtSectorBoundary(frontLeftX + laraPos.X, frontX + laraPos.X); break;
    case core::Axis::PosX:
    case core::Axis::NegX: shift.Z = reflectAtSectorBoundary(frontLeftZ + laraPos.Z, frontZ + laraPos.Z); break;
    }
    return;
  }

  if(frontRight.floorSpace.y > badPositiveDistance || frontRight.floorSpace.y < badNegativeDistance)
  {
    collisionType = AxisColl::Right;
    switch(facingAxis)
    {
    case core::Axis::PosZ:
    case core::Axis::NegZ: shift.X = reflectAtSectorBoundary(frontRightX + laraPos.X, frontX + laraPos.X); break;
    case core::Axis::PosX:
    case core::Axis::NegX: shift.Z = reflectAtSectorBoundary(frontRightZ + laraPos.Z, frontZ + laraPos.Z); break;
    }
  }
}

std::set<gsl::not_null<const loader::file::Room*>> CollisionInfo::collectTouchingRooms(const core::TRVec& position,
                                                                                       const core::Length& radius,
                                                                                       const core::Length& height,
                                                                                       const Engine& engine)
{
  std::set<gsl::not_null<const loader::file::Room*>> result;
  auto room = engine.getObjectManager().getLara().m_state.position.room;
  result.emplace(room);

  const auto roomAt = [position, room](const core::Length& x, const core::Length& y, const core::Length& z) {
    auto tmp = room;
    findRealFloorSector(position + core::TRVec(x, y, z), &tmp);
    return tmp;
  };

  result.emplace(roomAt(radius, 0_len, radius));
  result.emplace(roomAt(-radius, 0_len, radius));
  result.emplace(roomAt(radius, 0_len, -radius));
  result.emplace(roomAt(-radius, 0_len, -radius));
  result.emplace(roomAt(radius, -height, radius));
  result.emplace(roomAt(-radius, -height, radius));
  result.emplace(roomAt(radius, -height, -radius));
  result.emplace(roomAt(-radius, -height, -radius));
  return result;
}

bool CollisionInfo::checkStaticMeshCollisions(const core::TRVec& position,
                                              const core::Length& height,
                                              const Engine& engine)
{
  const auto rooms = collectTouchingRooms(position, collisionRadius + 50_len, height + 50_len, engine);

  const core::BoundingBox inBox{{position.X - collisionRadius, position.Y - height, position.Z - collisionRadius},
                                {position.X + collisionRadius, position.Y, position.Z + collisionRadius}};

  hasStaticMeshCollision = false;

  for(const auto& room : rooms)
  {
    for(const loader::file::RoomStaticMesh& rsm : room->staticMeshes)
    {
      const auto sm = engine.findStaticMeshById(rsm.meshId);
      if(sm->doNotCollide())
        continue;

      const auto meshBox = sm->getCollisionBox(rsm.position, rsm.rotation);

      if(!meshBox.intersects(inBox))
        continue;

      core::Length dx = 0_len, dz = 0_len;
      {
        auto left = inBox.max.X - meshBox.min.X;
        auto right = meshBox.max.X - inBox.min.X;
        if(left < right)
          dx = -left;
        else
          dx = right;

        left = inBox.max.Z - meshBox.min.Z;
        right = meshBox.max.Z - inBox.min.Z;
        if(left < right)
          dz = -left;
        else
          dz = right;
      }

      switch(facingAxis)
      {
      case core::Axis::PosX:
        if(abs(dz) > collisionRadius)
        {
          shift.X = dx;
          shift.Z = this->oldPosition.Z - position.Z;
          collisionType = AxisColl::Front;
        }
        else if(dz > 0_len && dz <= collisionRadius)
        {
          shift.X = 0_len;
          shift.Z = dz;
          collisionType = AxisColl::Right;
        }
        else if(dz < 0_len && dz >= -collisionRadius)
        {
          shift.X = 0_len;
          shift.Z = dz;
          collisionType = AxisColl::Left;
        }
        break;
      case core::Axis::PosZ:
        if(abs(dx) > collisionRadius)
        {
          shift.X = this->oldPosition.X - position.X;
          shift.Z = dz;
          collisionType = AxisColl::Front;
        }
        else if(dx > 0_len && dx <= collisionRadius)
        {
          shift.X = dx;
          shift.Z = 0_len;
          collisionType = AxisColl::Left;
        }
        else if(dx < 0_len && dx >= -collisionRadius)
        {
          shift.X = dx;
          shift.Z = 0_len;
          collisionType = AxisColl::Right;
        }
        break;
      case core::Axis::NegX:
        if(abs(dz) > collisionRadius)
        {
          shift.X = dx;
          shift.Z = this->oldPosition.Z - position.Z;
          collisionType = AxisColl::Front;
        }
        else if(dz > 0_len && dz <= collisionRadius)
        {
          shift.X = 0_len;
          shift.Z = dz;
          collisionType = AxisColl::Left;
        }
        else if(dz < 0_len && dz >= -collisionRadius)
        {
          shift.X = 0_len;
          shift.Z = dz;
          collisionType = AxisColl::Right;
        }
        break;
      case core::Axis::NegZ:
        if(abs(dx) > collisionRadius)
        {
          shift.X = this->oldPosition.X - position.X;
          shift.Z = dz + 1_len;
          collisionType = AxisColl::Front;
        }
        else if(dx > 0_len && dx <= collisionRadius)
        {
          shift.X = dx;
          shift.Z = 0_len;
          collisionType = AxisColl::Right;
        }
        else if(dx < 0_len && dx >= -collisionRadius)
        {
          shift.X = dx;
          shift.Z = 0_len;
          collisionType = AxisColl::Left;
        }
        break;
      }

      hasStaticMeshCollision = true;
      return true;
    }
  }

  return false;
}
} // namespace engine
