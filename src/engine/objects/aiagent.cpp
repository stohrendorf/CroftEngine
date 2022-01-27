#include "aiagent.h"

#include "core/boundingbox.h"
#include "core/genericvec.h"
#include "core/interval.h"
#include "core/magic.h"
#include "engine/ai/pathfinder.h"
#include "engine/collisioninfo.h"
#include "engine/engine.h"
#include "engine/heightinfo.h"
#include "engine/items_tr1.h"
#include "engine/objectmanager.h"
#include "engine/objects/objectstate.h"
#include "engine/particle.h"
#include "engine/raycast.h"
#include "engine/script/reflection.h"
#include "engine/script/scriptengine.h"
#include "engine/skeletalmodelnode.h"
#include "engine/soundeffects_tr1.h"
#include "engine/world/box.h"
#include "engine/world/sector.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "modelobject.h"
#include "object.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "util/helpers.h"

#include <boost/assert.hpp>
#include <boost/range/adaptor/map.hpp>
#include <exception>
#include <map>

namespace engine::objects
{
core::Angle AIAgent::rotateTowardsTarget(core::RotationSpeed maxRotationSpeed)
{
  if(m_state.speed == 0_spd || maxRotationSpeed == 0_au / 1_frame)
  {
    return 0_au;
  }

  const auto dx = m_creatureInfo->target.X - m_state.location.position.X;
  const auto dz = m_creatureInfo->target.Z - m_state.location.position.Z;
  auto turnAngle = angleFromAtan(dx, dz) - m_state.rotation.Y;
  if(abs(turnAngle) > 90_deg)
  {
    // the target is behind the current object, so we need a U-turn
    const auto relativeSpeed = m_state.speed.cast<float>() * 90_deg / maxRotationSpeed;
    if(util::square(dx) + util::square(dz) < util::square(relativeSpeed))
    {
      maxRotationSpeed /= 2;
    }
  }

  const auto turnSpeed = std::clamp(turnAngle / 1_frame, -maxRotationSpeed, maxRotationSpeed);

  m_state.rotation.Y += turnSpeed * 1_frame;
  return turnSpeed * 1_frame;
}

bool AIAgent::isPositionOutOfReach(const core::TRVec& testPosition,
                                   const core::Length& currentBoxFloor,
                                   const core::Length& nextBoxFloor,
                                   const ai::PathFinder& pathFinder) const
{
  const auto sectorBox = Location{m_state.location.room, testPosition}.updateRoom()->box;
  if(sectorBox == nullptr)
    return true;

  if(!pathFinder.canVisit(*sectorBox))
    return true;

  const auto stepHeight = currentBoxFloor - sectorBox->floor;

  if(stepHeight > pathFinder.step || stepHeight < pathFinder.drop)
    return true;

  if(stepHeight < -pathFinder.step && sectorBox->floor > nextBoxFloor)
    return true;

  return pathFinder.isFlying() && testPosition.Y > pathFinder.fly + sectorBox->floor;
}

bool AIAgent::anyMovingEnabledObjectInReach() const
{
  for(const auto& object : getWorld().getObjectManager().getObjects() | boost::adaptors::map_values)
  {
    if(object.get().get() == this)
      break;

    if(!object->m_isActive || object.get().get() == &getWorld().getObjectManager().getLara())
      continue;

    if(object->m_state.triggerState == TriggerState::Active && object->m_state.speed != 0_spd
       && distanceTo(object->m_state.location.position, m_state.location.position) < m_collisionRadius)
    {
      return true;
    }
  }
  return false;
}

bool AIAgent::animateCreature(const core::Angle& deltaRotationY, const core::Angle& tilt)
{
  if(m_creatureInfo == nullptr)
    return false;

  const auto invariantCheck = gsl::finally(
    [this]()
    {
      const auto sector = m_state.getCurrentSector();
      Ensures(sector != nullptr && sector->box != nullptr);
      BOOST_ASSERT(m_state.location.isValid());
    });

  const auto& pathFinder = m_creatureInfo->pathFinder;

  const auto oldLocation = m_state.location;

  const auto boxFloor = m_state.getCurrentBox()->floor;
  const auto zoneRef = world::Box::getZoneRef(
    getWorld().roomsAreSwapped(), m_creatureInfo->pathFinder.isFlying(), m_creatureInfo->pathFinder.step);
  ModelObject::update();

  // Moving to a different sector is basically a wall glitch, as the collision check always moves the entity away
  // from the sector boundaries if it detects a collision. As the max norm is always greater than or equal to the
  // euclidean norm, checking that the max norm of movement is less than or equal to the collision radius means that
  // the movement did not cause a wall glitch, assuming that the position before applying the movement was valid.
  BOOST_ASSERT(abs(m_skeleton->calculateFloorSpeed()) <= m_collisionRadius / 1_frame);

  if(m_state.triggerState == TriggerState::Deactivated)
  {
    if(!m_state.location.isValid())
    {
      m_state.location = oldLocation;
      setCurrentRoom(m_state.location.room);
    }
    m_state.health = core::DeadHealth;
    m_state.collidable = false;
    m_creatureInfo.reset();
    deactivate();
    return false;
  }

  const auto bbox = getSkeleton()->getBoundingBox();
  const auto bboxMaxY = m_state.location.position.Y + bbox.y.max;

  auto sector = m_state.location.moved(0_len, bbox.y.max, 0_len).updateRoom();

  if(sector->box == nullptr || boxFloor - sector->box->floor > pathFinder.step
     || boxFloor - sector->box->floor < pathFinder.drop
     || m_state.getCurrentBox().get()->*zoneRef != sector->box->*zoneRef)
  {
    static const auto shoveMin = [this](const core::Length& l)
    {
      return l / core::SectorSize * core::SectorSize + m_collisionRadius;
    };
    static const auto shoveMax = [this](const core::Length& l)
    {
      return shoveMin(l) + core::SectorSize - 1_len - m_collisionRadius;
    };

    const auto oldSectorX = oldLocation.position.X / core::SectorSize;
    const auto newSectorX = m_state.location.position.X / core::SectorSize;
    if(newSectorX < oldSectorX)
      m_state.location.position.X = shoveMin(oldLocation.position.X);
    else if(newSectorX > oldSectorX)
      m_state.location.position.X = shoveMax(oldLocation.position.X);

    const auto oldSectorZ = oldLocation.position.Z / core::SectorSize;
    const auto newSectorZ = m_state.location.position.Z / core::SectorSize;
    if(newSectorZ < oldSectorZ)
      m_state.location.position.Z = shoveMin(oldLocation.position.Z);
    else if(newSectorZ > oldSectorZ)
      m_state.location.position.Z = shoveMax(oldLocation.position.Z);

    sector = m_state.location.moved(0_len, bbox.y.max, 0_len).updateRoom();
  }

  Expects(sector->box != nullptr);

  core::Length nextFloor;
  if(const auto& exitBox = pathFinder.getNextPathBox(gsl::not_null{sector->box}); exitBox != nullptr)
  {
    nextFloor = exitBox->floor;
  }
  else
  {
    nextFloor = sector->box->floor;
  }

  const auto inSectorX = m_state.location.position.X % core::SectorSize;
  const auto inSectorZ = m_state.location.position.Z % core::SectorSize;

  // in-sector coordinate bounds incorporating collision radius
  const auto collisionBounds = core::Interval{0_len, core::SectorSize - 1_len}.narrowed(m_collisionRadius);
  gsl_Assert(collisionBounds.isValid());
  const core::TRVec origin{m_state.location.position.X, bboxMaxY, m_state.location.position.Z};
  // relative bounding box collision test coordinates
  const core::TRVec testDx{m_collisionRadius, 0_len, 0_len};
  const core::TRVec testDz{0_len, 0_len, m_collisionRadius};

  const auto cannotMoveTo
    = [this, floor = sector->box->floor, nextFloor = nextFloor, &pathFinder](const core::TRVec& pos)
  {
    return isPositionOutOfReach(pos, floor, nextFloor, pathFinder);
  };

  core::Length moveX = 0_len;
  core::Length moveZ = 0_len;

  if(inSectorZ < collisionBounds.min)
  {
    if(cannotMoveTo(origin - testDz))
    {
      moveZ = collisionBounds.min - inSectorZ;
    }

    if(inSectorX < collisionBounds.min)
    {
      if(cannotMoveTo(origin - testDx))
      {
        moveX = collisionBounds.min - inSectorX;
      }
      else if(moveZ == 0_len && cannotMoveTo(origin - testDz - testDx))
      {
        switch(axisFromAngle(m_state.rotation.Y))
        {
        case core::Axis::Deg180:
          [[fallthrough]];
        case core::Axis::Right90:
          moveX = collisionBounds.min - inSectorX;
          break;
        case core::Axis::Deg0:
          [[fallthrough]];
        case core::Axis::Left90:
          moveZ = collisionBounds.min - inSectorZ;
          break;
        }
      }
    }
    else if(inSectorX > collisionBounds.max)
    {
      if(cannotMoveTo(origin + testDx))
      {
        moveX = collisionBounds.max - inSectorX;
      }
      else if(moveZ == 0_len && cannotMoveTo(origin - testDz + testDx))
      {
        switch(axisFromAngle(m_state.rotation.Y))
        {
        case core::Axis::Deg180:
          [[fallthrough]];
        case core::Axis::Left90:
          moveX = collisionBounds.max - inSectorX;
          break;
        case core::Axis::Deg0:
          [[fallthrough]];
        case core::Axis::Right90:
          moveZ = collisionBounds.min - inSectorZ;
          break;
        }
      }
    }
  }
  else if(inSectorZ > collisionBounds.max)
  {
    if(cannotMoveTo(origin + testDz))
    {
      moveZ = collisionBounds.max - inSectorZ;
    }

    if(inSectorX < collisionBounds.min)
    {
      if(cannotMoveTo(origin - testDx))
      {
        moveX = collisionBounds.min - inSectorX;
      }
      else if(moveZ == 0_len && cannotMoveTo(origin + testDz - testDx))
      {
        switch(axisFromAngle(m_state.rotation.Y))
        {
        case core::Axis::Right90:
          [[fallthrough]];
        case core::Axis::Deg0:
          moveX = collisionBounds.min - inSectorX;
          break;
        case core::Axis::Left90:
          [[fallthrough]];
        case core::Axis::Deg180:
          moveZ = collisionBounds.max - inSectorZ;
          break;
        }
      }
    }
    else if(inSectorX > collisionBounds.max)
    {
      if(cannotMoveTo(origin + testDx))
      {
        moveX = collisionBounds.max - inSectorX;
      }
      else if(moveZ == 0_len && cannotMoveTo(origin + testDz + testDx))
      {
        switch(axisFromAngle(m_state.rotation.Y))
        {
        case core::Axis::Deg0:
          [[fallthrough]];
        case core::Axis::Left90:
          moveX = collisionBounds.max - inSectorX;
          break;
        case core::Axis::Deg180:
          [[fallthrough]];
        case core::Axis::Right90:
          moveZ = collisionBounds.max - inSectorZ;
          break;
        }
      }
    }
  }
  else
  {
    if(inSectorX < collisionBounds.min)
    {
      if(cannotMoveTo(origin - testDx))
      {
        moveX = collisionBounds.min - inSectorX;
      }
    }
    else if(inSectorX > collisionBounds.max)
    {
      if(cannotMoveTo(origin + testDx))
      {
        moveX = collisionBounds.max - inSectorX;
      }
    }
  }

  m_state.location.move(moveX, 0_len, moveZ);

  if(moveX != 0_len || moveZ != 0_len)
  {
    auto bboxMaxYLocation = m_state.location;
    bboxMaxYLocation.position.Y = bboxMaxY;
    sector = bboxMaxYLocation.updateRoom();

    m_state.rotation.Y += deltaRotationY;
    m_state.rotation.Z += std::clamp(8 * tilt - m_state.rotation.Z, -3_deg, +3_deg);
  }

  if(anyMovingEnabledObjectInReach())
  {
    m_state.location = oldLocation;
    return true;
  }

  if(pathFinder.isFlying())
  {
    auto moveY = std::clamp(m_creatureInfo->target.Y - m_state.location.position.Y, -pathFinder.fly, pathFinder.fly);

    const auto currentFloor
      = HeightInfo::fromFloor(sector,
                              core::TRVec{m_state.location.position.X, bboxMaxY, m_state.location.position.Z},
                              getWorld().getObjectManager().getObjects())
          .y;

    if(m_state.location.position.Y + moveY > currentFloor)
    {
      // fly target is below floor

      if(m_state.location.position.Y > currentFloor)
      {
        // we're already below the floor or inside a wall, so fix it
        m_state.location.position.X = oldLocation.position.X;
        m_state.location.position.Z = oldLocation.position.Z;
        // TODO this causes bats to "flicker" when falling and dying, should better be placed exactly on floor here
        moveY = -pathFinder.fly;
      }
      else
      {
        m_state.location.position.Y = currentFloor;
        moveY = 0_len;
      }
    }
    else
    {
      const auto ceiling
        = HeightInfo::fromCeiling(sector,
                                  core::TRVec{m_state.location.position.X, bboxMaxY, m_state.location.position.Z},
                                  getWorld().getObjectManager().getObjects())
            .y;

      // TODO nah... why, core, why?
      const auto y = m_state.type == TR1ItemId::CrocodileInWater ? 0_len : bbox.y.max;

      if(m_state.location.position.Y + y + moveY < ceiling)
      {
        if(m_state.location.position.Y + y < ceiling)
        {
          m_state.location.position.X = oldLocation.position.X;
          m_state.location.position.Z = oldLocation.position.Z;
          moveY = pathFinder.fly;
        }
        else
          moveY = 0_len;
      }
    }

    m_state.location.position.Y += moveY;
    auto bboxMaxYLocation = m_state.location;
    bboxMaxYLocation.position.Y = bboxMaxY;
    sector = bboxMaxYLocation.updateRoom();
    m_state.floor
      = HeightInfo::fromFloor(sector,
                              core::TRVec{m_state.location.position.X, bboxMaxY, m_state.location.position.Z},
                              getWorld().getObjectManager().getObjects())
          .y;

    core::Angle yaw{0_deg};
    if(m_state.speed != 0_spd)
      yaw = angleFromAtan(-moveY, m_state.speed * 1_frame);

    if(yaw < m_state.rotation.X - 1_deg)
      m_state.rotation.X -= 1_deg;
    else if(yaw > m_state.rotation.X + 1_deg)
      m_state.rotation.X += 1_deg;
    else
      m_state.rotation.X = yaw;

    m_state.location.updateRoom();
    BOOST_ASSERT(m_state.location.isValid());
    setCurrentRoom(m_state.location.room);

    return true;
  }

  if(m_state.location.position.Y > m_state.floor)
  {
    m_state.location.position.Y = m_state.floor;
  }
  else if(m_state.floor - m_state.location.position.Y > 64_len)
  {
    m_state.location.position.Y += 64_len;
  }
  else if(m_state.location.position.Y < m_state.floor)
  {
    m_state.location.position.Y = m_state.floor;
  }

  m_state.rotation.X = 0_au;

  sector = m_state.location.updateRoom();
  BOOST_ASSERT(m_state.location.isValid());
  setCurrentRoom(m_state.location.room);
  m_state.floor
    = HeightInfo::fromFloor(sector, m_state.location.position, getWorld().getObjectManager().getObjects()).y;

  return true;
}

AIAgent::AIAgent(const std::string& name,
                 const gsl::not_null<world::World*>& world,
                 const gsl::not_null<const world::Room*>& room,
                 const loader::file::Item& item,
                 const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel, true}
{
  m_state.collidable = true;
  const core::Angle v = util::rand15s(90_deg);
  m_state.rotation.Y += v;

  loadObjectInfo(false);
}

void AIAgent::collide(CollisionInfo& collisionInfo)
{
  if(!isNear(getWorld().getObjectManager().getLara(), collisionInfo.collisionRadius))
    return;

  if(!testBoneCollision(getWorld().getObjectManager().getLara()))
    return;

  if(!collisionInfo.policies.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  const bool enableSpaz = !m_state.isDead() && collisionInfo.policies.is_set(CollisionInfo::PolicyFlags::EnableSpaz);
  enemyPush(collisionInfo, enableSpaz, false);
}

bool AIAgent::canShootAtLara(const ai::EnemyLocation& enemyLocation) const
{
  if(!enemyLocation.enemyAhead || enemyLocation.enemyDistance >= util::square(7 * core::SectorSize))
  {
    return false;
  }

  return raycastLineOfSight(m_state.location,
                            getWorld().getObjectManager().getLara().m_state.location.position
                              - core::TRVec{0_len, 768_len, 0_len},
                            getWorld().getObjectManager())
    .first;
}

bool AIAgent::tryShootAtLara(ModelObject& object,
                             const core::Area& distance,
                             const core::TRVec& bonePos,
                             size_t boneIndex,
                             const core::Angle& muzzleFlashAngle)
{
  auto& lara = getWorld().getObjectManager().getLara();
  bool isHit = false;
  if(distance <= util::square(7 * core::SectorSize))
  {
    if(util::rand15() < (util::square(7 * core::SectorSize) - distance) / util::square(40_len) - 8192)
    {
      isHit = true;

      lara.emitParticle(core::TRVec{}, util::rand15(lara.getSkeleton()->getBoneCount()), &createBloodSplat);

      if(!lara.isInWater())
        lara.playSoundEffect(TR1SoundEffect::BulletHitsLara);
    }
  }

  if(!isHit)
  {
    auto location = lara.m_state.location;
    location.position.X += util::rand15s(core::SectorSize / 2);
    location.position.Y = lara.m_state.floor;
    location.position.Z += util::rand15s(core::SectorSize / 2);
    lara.emitRicochet(location);
  }

  auto p = object.emitParticle(bonePos, boneIndex, &createMuzzleFlash);
  p->angle.Y += muzzleFlashAngle;

  return isHit;
}

void AIAgent::loadObjectInfo(bool withoutGameState)
{
  m_collisionRadius = core::Length{getWorld()
                                     .getEngine()
                                     .getScriptEngine()
                                     .getGameflow()
                                     .getObjectInfos()
                                     .at(m_state.type.get_as<TR1ItemId>())
                                     ->radius};

  if(!withoutGameState)
    m_state.loadObjectInfo(getWorld().getEngine().getScriptEngine().getGameflow());
}

void AIAgent::hitLara(const core::Health& strength)
{
  getWorld().getObjectManager().getLara().m_state.is_hit = true;
  getWorld().getObjectManager().getLara().m_state.health -= strength;
}

void AIAgent::serialize(const serialization::Serializer<world::World>& ser)
{
  ModelObject::serialize(ser);
  ser(S_NV("collisionRadius", m_collisionRadius), S_NV("creatureInfo", m_creatureInfo));
}

void AIAgent::initCreatureInfo()
{
  if(m_creatureInfo != nullptr)
    return;

  m_creatureInfo = std::make_unique<ai::CreatureInfo>(getWorld(), m_state.type, m_state.getCurrentBox());
}

bool AIAgent::isInsideZoneButNotInBox(const uint32_t zoneId, const world::Box& targetBox) const
{
  Expects(m_creatureInfo != nullptr);

  const auto zoneRef = world::Box::getZoneRef(
    getWorld().roomsAreSwapped(), m_creatureInfo->pathFinder.isFlying(), m_creatureInfo->pathFinder.step);

  if(zoneId != targetBox.*zoneRef)
  {
    return false;
  }

  if(!m_creatureInfo->pathFinder.canVisit(targetBox))
  {
    return false;
  }

  return !targetBox.xInterval.contains(m_state.location.position.X)
         || !targetBox.zInterval.contains(m_state.location.position.Z);
}
} // namespace engine::objects
