#include "collisioninfo.h"

#include "core/angle.h"
#include "core/boundingbox.h"
#include "core/genericvec.h"
#include "core/interval.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/floordata/floordata.h"
#include "engine/floordata/types.h"
#include "engine/heightinfo.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/objects/objectstate.h"
#include "engine/world/room.h"
#include "engine/world/sector.h"
#include "engine/world/staticmesh.h"
#include "objects/laraobject.h"
#include "qs/qs.h"
#include "type_safe/integer.hpp"
#include "util/helpers.h" // IWYU pragma: keep
#include "world/world.h"

#include <cstdint>
#include <gsl/gsl-lite.hpp>
#include <set>
#include <tuple>

namespace engine
{
namespace
{
[[nodiscard]] core::Length reflectAtSectorBoundary(const core::Length& target, const core::Length& current)
{
  const auto targetSector = sectorOf(target);
  const auto currentSector = sectorOf(current);
  if(targetSector == currentSector)
    return 0_len;

  const auto targetInSector = toSectorLocal(target);
  if(currentSector < targetSector)
    return -(targetInSector + 1_len);
  return 1_sectors - (targetInSector - 1_len);
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
    result.x = {bbox.z.min, bbox.z.max};
    result.z = {-bbox.x.max, -bbox.x.min};
    break;
  case core::Axis::Deg180:
    result.x = {-bbox.x.max, -bbox.x.min};
    result.z = {-bbox.z.max, -bbox.z.min};
    break;
  case core::Axis::Left90:
    result.x = {-bbox.z.max, -bbox.z.min};
    result.z = {bbox.x.min, bbox.x.max};
    break;
  }

  result.x += pos.X;
  result.y += pos.Y;
  result.z += pos.Z;
  return result;
}

[[nodiscard]] core::Length minShift(const core::Length& min, const core::Length& max) noexcept
{
  return min < max ? -min : max;
}

std::tuple<int8_t, int8_t> getFloorSlantInfo(gsl::not_null<const world::Sector*> sector, const core::TRVec& position)
{
  while(sector->roomBelow != nullptr)
  {
    sector = gsl::not_null{sector->roomBelow->getSectorByAbsolutePosition(position)};
  }

  static const auto zero = std::make_tuple(int8_t{0}, int8_t{0});

  if(position.Y + core::QuarterSectorSize * 2 < sector->floorHeight)
    return zero;
  if(sector->floorData == nullptr)
    return zero;
  if(floordata::FloorDataChunk{*sector->floorData}.type != floordata::FloorDataChunkType::FloorSlant)
    return zero;

  const auto fd = sector->floorData[1];
  return std::make_tuple(gsl::narrow_cast<int8_t>(fd.get() & 0xffu), gsl::narrow_cast<int8_t>(fd.get() >> 8u));
}
} // namespace

void CollisionInfo::initHeightInfo(const core::TRVec& laraPos, const world::World& world, const core::Length& height)
{
  collisionType = AxisColl::None;
  shift = core::TRVec{0_len, 0_len, 0_len};
  facingAxis = axisFromAngle(facingAngle);

  Location refTestLocation{world.getObjectManager().getLara().m_state.location.room,
                           laraPos - core::TRVec{0_len, height + core::ScalpToHandsHeight, 0_len}};
  const auto currentSector = refTestLocation.updateRoom();

  std::tie(floorSlantX, floorSlantZ) = getFloorSlantInfo(
    currentSector, core::TRVec{laraPos.X, world.getObjectManager().getLara().m_state.location.position.Y, laraPos.Z});

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

  const auto initVD = [&refTestLocation, &world, &height, laraPosY = laraPos.Y, policyFlags = policies](
                        VerticalDistances& vd, const core::Length& dx, const core::Length& dz)
  {
    auto testLocation = refTestLocation.moved(dx, 0_len, dz);
    const auto sector = testLocation.updateRoom();
    vd.init(sector, testLocation.position, world.getObjectManager().getObjects(), laraPosY, height);

    if(policyFlags.is_set(PolicyFlags::SlopesAreWalls) && vd.floor.slantClass == SlantClass::Steep
       && vd.floor.dy < 0_len)
    {
      // This is not a typo, it is really -32767. It must be different from core::InvalidHeight to not mix it up with
      // walls.
      vd.floor.dy = -32767_len;
    }
    else if(vd.floor.dy > 0_len)
    {
      if((policyFlags.is_set(PolicyFlags::SlopesArePits) && vd.floor.slantClass == SlantClass::Steep))
      {
        vd.floor.dy = 1_sectors / 2;
      }
      else if(policyFlags.is_set(PolicyFlags::LavaIsPit) && vd.floor.lastCommandSequenceOrDeath != nullptr
              && floordata::FloorDataChunk::extractType(*vd.floor.lastCommandSequenceOrDeath)
                   == floordata::FloorDataChunkType::Death)
      {
        vd.floor.dy = 1_sectors / 2;
      }
    }
  };

  mid.init(currentSector, refTestLocation.position, world.getObjectManager().getObjects(), laraPos.Y, height);
  initVD(front, frontX, frontZ);
  initVD(frontLeft, frontLeftX, frontLeftZ);
  initVD(frontRight, frontRightX, frontRightZ);

  checkStaticMeshCollisions(laraPos, height, world);

  if(mid.floor.dy == core::InvalidHeight)
  {
    collisionType = AxisColl::Front;
    shift = initialPosition - laraPos;
    return;
  }

  if(mid.floor.dy - mid.ceiling.dy <= 0_len)
  {
    collisionType = AxisColl::Jammed;
    shift = initialPosition - laraPos;
    return;
  }

  if(mid.ceiling.dy >= 0_len)
  {
    collisionType = AxisColl::Top;
    shift.Y = mid.ceiling.dy;
  }

  if(!validFloorHeight.contains(front.floor.dy) || front.ceiling.dy > validCeilingHeightMin)
  {
    collisionType = AxisColl::Front;
    switch(facingAxis)
    {
    case core::Axis::Deg0:
      [[fallthrough]];
    case core::Axis::Deg180:
      shift.X = initialPosition.X - laraPos.X;
      shift.Z = reflectAtSectorBoundary(frontZ + laraPos.Z, laraPos.Z);
      break;
    case core::Axis::Left90:
      [[fallthrough]];
    case core::Axis::Right90:
      shift.X = reflectAtSectorBoundary(frontX + laraPos.X, laraPos.X);
      shift.Z = initialPosition.Z - laraPos.Z;
      break;
    }
    return;
  }

  if(front.ceiling.dy >= validCeilingHeightMin)
  {
    collisionType = AxisColl::FrontTop;
    shift = initialPosition - laraPos;
    return;
  }

  if(!validFloorHeight.contains(frontLeft.floor.dy))
  {
    collisionType = AxisColl::FrontLeft;
    switch(facingAxis)
    {
    case core::Axis::Deg0:
      [[fallthrough]];
    case core::Axis::Deg180:
      shift.X = reflectAtSectorBoundary(frontLeftX + laraPos.X, frontX + laraPos.X);
      break;
    case core::Axis::Left90:
      [[fallthrough]];
    case core::Axis::Right90:
      shift.Z = reflectAtSectorBoundary(frontLeftZ + laraPos.Z, frontZ + laraPos.Z);
      break;
    }
    return;
  }

  if(!validFloorHeight.contains(frontRight.floor.dy))
  {
    collisionType = AxisColl::FrontRight;
    switch(facingAxis)
    {
    case core::Axis::Deg0:
      [[fallthrough]];
    case core::Axis::Deg180:
      shift.X = reflectAtSectorBoundary(frontRightX + laraPos.X, frontX + laraPos.X);
      break;
    case core::Axis::Left90:
      [[fallthrough]];
    case core::Axis::Right90:
      shift.Z = reflectAtSectorBoundary(frontRightZ + laraPos.Z, frontZ + laraPos.Z);
      break;
    }
  }
}

std::set<gsl::not_null<const world::Room*>> CollisionInfo::collectTouchingRooms(const core::TRVec& position,
                                                                                const core::Length& radius,
                                                                                const core::Length& height,
                                                                                const world::World& world)
{
  std::set<gsl::not_null<const world::Room*>> result;
  auto room = world.getObjectManager().getLara().m_state.location.room;
  result.emplace(room);

  const auto roomAt = [position, room](const core::Length& x, const core::Length& y, const core::Length& z)
  {
    Location tmp{room, position + core::TRVec{x, y, z}};
    tmp.updateRoom();
    return tmp.room;
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

bool CollisionInfo::checkStaticMeshCollisions(const core::TRVec& objectPos,
                                              const core::Length& objectHeight,
                                              const world::World& world)
{
  const auto rooms = collectTouchingRooms(objectPos, collisionRadius + 50_len, objectHeight + 50_len, world);

  // no need to worry about rotation here, this is a square box
  const core::BoundingBox objectBox{
    {objectPos.X - collisionRadius, objectPos.Y - objectHeight, objectPos.Z - collisionRadius},
    {objectPos.X + collisionRadius, objectPos.Y, objectPos.Z + collisionRadius}};

  hasStaticMeshCollision = false;

  for(const auto& room : rooms)
  {
    for(const world::RoomStaticMesh& rsm : room->staticMeshes)
    {
      if(rsm.staticMesh->doNotCollide)
        continue;

      const auto meshBox = rotateTranslate(rsm.staticMesh->collisionBox, rsm.position, rsm.rotation);
      if(!meshBox.intersectsExclusive(objectBox))
        continue;

      // both collision boxes are in world space
      shift.X = minShift(objectBox.x.max - meshBox.x.min, meshBox.x.max - objectBox.x.min);
      shift.Z = minShift(objectBox.z.max - meshBox.z.min, meshBox.z.max - objectBox.z.min);

      switch(facingAxis)
      {
      case core::Axis::Deg0:
        if(abs(shift.X) > collisionRadius)
        {
          shift.X = initialPosition.X - objectPos.X;
          collisionType = AxisColl::Front;
        }
        else
        {
          shift.Z = 0_len;
          collisionType = shift.X > 0_len ? AxisColl::FrontLeft : AxisColl::FrontRight;
        }
        break;
      case core::Axis::Deg180:
        if(abs(shift.X) > collisionRadius)
        {
          shift.X = initialPosition.X - objectPos.X;
          collisionType = AxisColl::Front;
        }
        else
        {
          shift.Z = 0_len;
          collisionType = shift.X > 0_len ? AxisColl::FrontRight : AxisColl::FrontLeft;
        }
        break;
      case core::Axis::Right90:
        if(abs(shift.Z) > collisionRadius)
        {
          shift.Z = initialPosition.Z - objectPos.Z;
          collisionType = AxisColl::Front;
        }
        else
        {
          shift.X = 0_len;
          collisionType = shift.Z > 0_len ? AxisColl::FrontRight : AxisColl::FrontLeft;
        }
        break;
      case core::Axis::Left90:
        if(abs(shift.Z) > collisionRadius)
        {
          shift.Z = initialPosition.Z - objectPos.Z;
          collisionType = AxisColl::Front;
        }
        else
        {
          shift.X = 0_len;
          collisionType = shift.Z > 0_len ? AxisColl::FrontLeft : AxisColl::FrontRight;
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
