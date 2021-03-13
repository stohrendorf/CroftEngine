#include "collisioninfo.h"

#include "core/magic.h"
#include "objects/laraobject.h"
#include "world/world.h"

namespace engine
{
namespace
{
[[nodiscard]] core::Length reflectAtSectorBoundary(const core::Length& target, const core::Length& current)
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

[[nodiscard]] core::BoundingBox
  rotateTranslate(const core::BoundingBox& bbox, const core::TRVec& pos, const core::Angle& angle)
{
  auto result = bbox;

  const auto axis = axisFromAngle(angle);
  switch(axis)
  {
  case core::Axis::Deg0:
    // nothing to do
    break;
  case core::Axis::Right90:
    result.min.X = bbox.min.Z;
    result.max.X = bbox.max.Z;
    result.min.Z = -bbox.max.X;
    result.max.Z = -bbox.min.X;
    break;
  case core::Axis::Deg180:
    result.min.X = -bbox.max.X;
    result.max.X = -bbox.min.X;
    result.min.Z = -bbox.max.Z;
    result.max.Z = -bbox.min.Z;
    break;
  case core::Axis::Left90:
    result.min.X = -bbox.max.Z;
    result.max.X = -bbox.min.Z;
    result.min.Z = bbox.min.X;
    result.max.Z = bbox.max.X;
    break;
  }

  result.min += pos;
  result.max += pos;
  return result;
}

[[nodiscard]] core::Length absMin(const core::Length& a, const core::Length& b)
{
  return abs(a) < abs(b) ? a : b;
}
} // namespace

void CollisionInfo::initHeightInfo(const core::TRVec& laraPos, const world::World& world, const core::Length& height)
{
  collisionType = AxisColl::None;
  shift = core::TRVec{0_len, 0_len, 0_len};
  facingAxis = axisFromAngle(facingAngle);

  auto room = world.getObjectManager().getLara().m_state.position.room;
  const auto refTestPos = laraPos - core::TRVec(0_len, height + core::ScalpToHandsHeight, 0_len);
  const auto currentSector = findRealFloorSector(refTestPos, &room);

  mid.init(currentSector, refTestPos, world.getObjectManager().getObjects(), laraPos.Y, height);

  std::tie(floorSlantX, floorSlantZ) = world::getFloorSlantInfo(
    currentSector, core::TRVec{laraPos.X, world.getObjectManager().getLara().m_state.position.position.Y, laraPos.Z});

  core::Length frontX = 0_len, frontZ = 0_len;
  core::Length frontLeftX = 0_len, frontLeftZ = 0_len;
  core::Length frontRightX = 0_len, frontRightZ = 0_len;

  switch(facingAxis)
  {
  case core::Axis::Deg0:
    frontX = util::sin(collisionRadius, facingAngle);
    frontZ = collisionRadius;
    frontLeftX = -collisionRadius;
    frontLeftZ = collisionRadius;
    frontRightX = collisionRadius;
    frontRightZ = collisionRadius;
    break;
  case core::Axis::Right90:
    frontX = collisionRadius;
    frontZ = util::cos(collisionRadius, facingAngle);
    frontLeftX = collisionRadius;
    frontLeftZ = collisionRadius;
    frontRightX = collisionRadius;
    frontRightZ = -collisionRadius;
    break;
  case core::Axis::Deg180:
    frontX = util::sin(collisionRadius, facingAngle);
    frontZ = -collisionRadius;
    frontLeftX = collisionRadius;
    frontLeftZ = -collisionRadius;
    frontRightX = -collisionRadius;
    frontRightZ = -collisionRadius;
    break;
  case core::Axis::Left90:
    frontX = -collisionRadius;
    frontZ = util::cos(collisionRadius, facingAngle);
    frontLeftX = -collisionRadius;
    frontLeftZ = -collisionRadius;
    frontRightX = -collisionRadius;
    frontRightZ = collisionRadius;
    break;
  }

  const auto initVSI = [&refTestPos, &world, &height, laraPosY = laraPos.Y, policyFlags = policies](
                         VerticalSpaceInfo& vsi,
                         core::Length dx,
                         core::Length dz,
                         const gsl::not_null<gsl::not_null<const loader::file::Room*>*>& room) {
    const auto testPos = refTestPos + core::TRVec(dx, 0_len, dz);
    const auto sector = findRealFloorSector(testPos, room);
    vsi.init(sector, testPos, world.getObjectManager().getObjects(), laraPosY, height);

    if(policyFlags.is_set(PolicyFlags::SlopesAreWalls) && vsi.floorSpace.slantClass == SlantClass::Steep
       && vsi.floorSpace.y < 0_len)
    {
      vsi.floorSpace.y = -32767_len; // This is not a typo, it is really -32767
    }
    else if(vsi.floorSpace.y > 0_len
            && ((policyFlags.is_set(PolicyFlags::SlopesArePits) && vsi.floorSpace.slantClass == SlantClass::Steep)
                || (policyFlags.is_set(PolicyFlags::LavaIsPit) && vsi.floorSpace.lastCommandSequenceOrDeath != nullptr
                    && floordata::FloorDataChunk::extractType(*vsi.floorSpace.lastCommandSequenceOrDeath)
                         == floordata::FloorDataChunkType::Death)))
    {
      vsi.floorSpace.y = core::SectorSize / 2;
    }
  };

  initVSI(front, frontX, frontZ, &room);
  initVSI(frontLeft, frontLeftX, frontLeftZ, &room);
  initVSI(frontRight, frontRightX, frontRightZ, &room);

  checkStaticMeshCollisions(laraPos, height, world);

  if(mid.floorSpace.y == -core::HeightLimit)
  {
    shift = initialPosition - laraPos;
    collisionType = AxisColl::Front;
    return;
  }

  if(mid.floorSpace.y <= mid.ceilingSpace.y)
  {
    collisionType = AxisColl::TopFront;
    shift = initialPosition - laraPos;
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
    case core::Axis::Deg0: [[fallthrough]];
    case core::Axis::Deg180:
      shift.X = initialPosition.X - laraPos.X;
      shift.Z = reflectAtSectorBoundary(frontZ + laraPos.Z, laraPos.Z);
      break;
    case core::Axis::Left90: [[fallthrough]];
    case core::Axis::Right90:
      shift.X = reflectAtSectorBoundary(frontX + laraPos.X, laraPos.X);
      shift.Z = initialPosition.Z - laraPos.Z;
      break;
    }
    return;
  }

  if(front.ceilingSpace.y >= badCeilingDistance)
  {
    collisionType = AxisColl::TopBottom;
    shift = initialPosition - laraPos;
    return;
  }

  if(frontLeft.floorSpace.y > badPositiveDistance || frontLeft.floorSpace.y < badNegativeDistance)
  {
    collisionType = AxisColl::Left;
    switch(facingAxis)
    {
    case core::Axis::Deg0: [[fallthrough]];
    case core::Axis::Deg180: shift.X = reflectAtSectorBoundary(frontLeftX + laraPos.X, frontX + laraPos.X); break;
    case core::Axis::Left90: [[fallthrough]];
    case core::Axis::Right90: shift.Z = reflectAtSectorBoundary(frontLeftZ + laraPos.Z, frontZ + laraPos.Z); break;
    }
    return;
  }

  if(frontRight.floorSpace.y > badPositiveDistance || frontRight.floorSpace.y < badNegativeDistance)
  {
    collisionType = AxisColl::Right;
    switch(facingAxis)
    {
    case core::Axis::Deg0: [[fallthrough]];
    case core::Axis::Deg180: shift.X = reflectAtSectorBoundary(frontRightX + laraPos.X, frontX + laraPos.X); break;
    case core::Axis::Left90: [[fallthrough]];
    case core::Axis::Right90: shift.Z = reflectAtSectorBoundary(frontRightZ + laraPos.Z, frontZ + laraPos.Z); break;
    }
  }
}

std::set<gsl::not_null<const loader::file::Room*>> CollisionInfo::collectTouchingRooms(const core::TRVec& position,
                                                                                       const core::Length& radius,
                                                                                       const core::Length& height,
                                                                                       const world::World& world)
{
  std::set<gsl::not_null<const loader::file::Room*>> result;
  auto room = world.getObjectManager().getLara().m_state.position.room;
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

bool CollisionInfo::checkStaticMeshCollisions(const core::TRVec& pokePosition,
                                              const core::Length& pokeHeight,
                                              const world::World& world)
{
  const auto rooms = collectTouchingRooms(pokePosition, collisionRadius + 50_len, pokeHeight + 50_len, world);

  // no need to worry about rotation here, this is a square box
  const core::BoundingBox pokeBox{
    {pokePosition.X - collisionRadius, pokePosition.Y - pokeHeight, pokePosition.Z - collisionRadius},
    {pokePosition.X + collisionRadius, pokePosition.Y, pokePosition.Z + collisionRadius}};

  hasStaticMeshCollision = false;

  for(const auto& room : rooms)
  {
    for(const loader::file::RoomStaticMesh& rsm : room->staticMeshes)
    {
      const auto mesh = world.findStaticMeshById(rsm.meshId);
      if(mesh->doNotCollide())
        continue;

      const auto meshBox = rotateTranslate(mesh->collision_box, rsm.position, rsm.rotation);
      if(!meshBox.intersects(pokeBox))
        continue;

      // both collision boxes are in world space
      shift.X = absMin(meshBox.min.X - pokeBox.max.X, meshBox.max.X - pokeBox.min.X);
      shift.Z = absMin(meshBox.min.Z - pokeBox.max.Z, meshBox.max.Z - pokeBox.min.Z);

      switch(facingAxis)
      {
      case core::Axis::Deg0:
        if(abs(shift.X) > collisionRadius)
        {
          shift.X = initialPosition.X - pokePosition.X;
          collisionType = AxisColl::Front;
        }
        else
        {
          collisionType = shift.X > 0_len ? AxisColl::Left : AxisColl::Right;
        }
        break;
      case core::Axis::Deg180:
        if(abs(shift.X) > collisionRadius)
        {
          shift.X = initialPosition.X - pokePosition.X;
          collisionType = AxisColl::Front;
        }
        else
        {
          shift.Z = 0_len;
          collisionType = shift.X > 0_len ? AxisColl::Right : AxisColl::Left;
        }
        break;
      case core::Axis::Right90:
        if(abs(shift.Z) > collisionRadius)
        {
          shift.Z = initialPosition.Z - pokePosition.Z;
          collisionType = AxisColl::Front;
        }
        else
        {
          shift.X = 0_len;
          collisionType = shift.Z > 0_len ? AxisColl::Right : AxisColl::Left;
        }
        break;
      case core::Axis::Left90:
        if(abs(shift.Z) > collisionRadius)
        {
          shift.Z = initialPosition.Z - pokePosition.Z;
          collisionType = AxisColl::Front;
        }
        else
        {
          shift.X = 0_len;
          collisionType = shift.Z > 0_len ? AxisColl::Left : AxisColl::Right;
        }
        break;
      }

      hasStaticMeshCollision = true;
      return hasStaticMeshCollision;
    }
  }

  return hasStaticMeshCollision;
}
} // namespace engine
