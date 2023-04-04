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
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/log/trivial.hpp>
#include <boost/range/adaptor/map.hpp>
#include <exception>
#include <gl/renderstate.h>
#include <iosfwd>
#include <map>

namespace engine::objects
{
namespace
{
bool canMoveTo(const world::Room& room,
               const ai::PathFinder& pathFinder,
               const world::Box& currentBox,
               const core::Length& currentSectorBoxFloor,
               const core::Length& nextPathFloor,
               const core::TRVec& testPos)
{
  const auto testBox = Location{gsl::not_null{&room}, testPos}.updateRoom()->box;
  if(testBox == nullptr || !pathFinder.canVisit(*testBox, currentBox.blocked, currentBox.blockable))
  {
    return false;
  }

  const auto dy = testBox->floor - currentSectorBoxFloor;

  if(dy < -pathFinder.getStep() || dy > -pathFinder.getDrop())
  {
    // height difference doesn't allow stepping up or dropping down
    return false;
  }

  if(dy > pathFinder.getStep() && testBox->floor > nextPathFloor)
  {
    // height difference would allow stepping down, but the test position is already below the wanted path
    return false;
  }

  if(!pathFinder.isFlying())
  {
    return true;
  }

  // true if the entity is flying, but the test position is outside the maximum vertical flying speed that would
  // allow recovery after penetrating the floor
  return testPos.Y <= testBox->floor + pathFinder.getFly();
}
} // namespace

core::Angle AIAgent::rotateTowardsTarget(core::RotationSpeed maxRotationSpeed)
{
  if(m_state.speed == 0_spd || maxRotationSpeed == 0_au / 1_frame)
  {
    return 0_au;
  }

  const auto dx = m_creatureInfo->target.X - m_state.location.position.X;
  const auto dz = m_creatureInfo->target.Z - m_state.location.position.Z;
  const auto turnAngle = angleFromAtan(dx, dz) - m_state.rotation.Y;
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

bool AIAgent::anyMovingEnabledObjectInReach() const
{
  for(const auto& object : getWorld().getObjectManager().getObjects() | boost::adaptors::map_values)
  {
    if(object.get().get() == this)
      break;

    if(!object->isActive() || object.get().get() == &getWorld().getObjectManager().getLara())
      continue;

    if(object->m_state.triggerState == TriggerState::Active && object->m_state.speed != 0_spd
       && distanceTo(object->m_state.location.position, m_state.location.position) < m_collisionRadius)
    {
      return true;
    }
  }
  return false;
}

void AIAgent::animateCreature(const core::Angle& collisionRotationY, const core::Angle& tilt)
{
  if(m_creatureInfo == nullptr)
    return;

  const auto invariantCheck = gsl::finally(
    [this]()
    {
      const auto sector = m_state.getCurrentSector();
      gsl_Assert(sector != nullptr && sector->box != nullptr);
      BOOST_ASSERT(m_state.location.isValid());
    });

  const auto& pathFinder = m_creatureInfo->pathFinder;

  const auto oldLocation = m_state.location;
  const auto oldBox = gsl::not_null{oldLocation.getCurrentSector()->box};
  const auto zoneRef
    = world::Box::getZoneRef(getWorld().roomsAreSwapped(), pathFinder.isFlying(), pathFinder.getStep());

#ifndef NDEBUG
  const auto invariantCheck2 = gsl::finally(
    [this, oldPos = m_state.location.position]()
    {
      gsl_Assert(abs(m_state.location.position.Y - oldPos.Y) < 2_sectors);
    });
#endif

  // this can move the entity into an invalid position
  ModelObject::update();

  // Moving to a different sector is basically a wall glitch, as the collision check always moves the entity away
  // from the sector boundaries if it detects a collision. As the max norm is always greater than or equal to the
  // euclidean norm, checking that the max norm of movement is less than or equal to the collision radius means that
  // the movement did not cause a wall glitch, assuming that the position before applying the movement was valid.
  if(abs(m_state.speed) > m_collisionRadius / 1_frame)
  {
    BOOST_LOG_TRIVIAL(warning) << m_skeleton->getName() << " movement speed of " << m_state.speed
                               << " exceeds collision radius of " << m_collisionRadius;
  }

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
    return;
  }

  const auto bbox = getSkeleton()->getBoundingBox();

  auto currentSector = m_state.location.moved(0_len, bbox.y.max, 0_len).updateRoom();

  // fix location in case the entity moved to an invalid location, including checks for step/drop limits.
  // keep in mind that step/drop limits are negated, so they're subtracted here instead of being added.
  const bool isInvalidPosition = currentSector->box == nullptr || !pathFinder.canVisit(*currentSector->box)
                                 || currentSector->box->floor < oldBox->floor - pathFinder.getStep()
                                 || currentSector->box->floor > oldBox->floor - pathFinder.getDrop()
                                 || oldBox.get()->*zoneRef != currentSector->box->*zoneRef;
  if(isInvalidPosition)
  {
    const auto toMin = [this](const core::Length& l)
    {
      return snappedSector(l) + m_collisionRadius;
    };
    const auto toMax = [this](const core::Length& l)
    {
      return snappedSector(l) + 1_sectors - 1_len - m_collisionRadius;
    };

    const auto oldSectorX = sectorOf(oldLocation.position.X);
    const auto newSectorX = sectorOf(m_state.location.position.X);
    if(newSectorX < oldSectorX)
      m_state.location.position.X = toMin(oldLocation.position.X);
    else if(newSectorX > oldSectorX)
      m_state.location.position.X = toMax(oldLocation.position.X);

    const auto oldSectorZ = sectorOf(oldLocation.position.Z);
    const auto newSectorZ = sectorOf(m_state.location.position.Z);
    if(newSectorZ < oldSectorZ)
      m_state.location.position.Z = toMin(oldLocation.position.Z);
    else if(newSectorZ > oldSectorZ)
      m_state.location.position.Z = toMax(oldLocation.position.Z);

    currentSector = m_state.location.moved(0_len, bbox.y.max, 0_len).updateRoom();
  }

  gsl_Assert(currentSector->box != nullptr);

  core::Length nextPathFloor;
  if(const auto& exitBox = pathFinder.getNextPathBox(gsl::not_null{currentSector->box}); exitBox != nullptr)
  {
    nextPathFloor = exitBox->floor;
  }
  else
  {
    nextPathFloor = currentSector->box->floor;
  }

  const auto collisionFreeRangeX
    = snappedSector(m_state.location.position.X) + core::Interval{0_len, 1_sectors - 1_len}.narrowed(m_collisionRadius);
  gsl_Assert(collisionFreeRangeX.isValid());
  const auto collisionFreeRangeZ
    = snappedSector(m_state.location.position.Z) + core::Interval{0_len, 1_sectors - 1_len}.narrowed(m_collisionRadius);
  gsl_Assert(collisionFreeRangeZ.isValid());
  const auto bottom = m_state.location.position + core::TRVec{0_len, bbox.y.max, 0_len};
  // relative bounding box collision test coordinates
  const core::TRVec testDx{m_collisionRadius, 0_len, 0_len};
  const core::TRVec testDz{0_len, 0_len, m_collisionRadius};

  const auto isFeasiblePosition
    = [this, currentSector = currentSector, nextPathFloor = nextPathFloor, &pathFinder](const core::TRVec& position)
  {
    return canMoveTo(*m_state.location.room,
                     pathFinder,
                     *gsl::not_null{currentSector->box},
                     currentSector->box->floor,
                     nextPathFloor,
                     position);
  };

  // bats temporarily penetrate then floor when dying, so this will trigger
  // BOOST_ASSERT(isFeasiblePosition(m_state.location.position));

  core::Length nextX = m_state.location.position.X;
  core::Length nextZ = m_state.location.position.Z;

  auto moveAwayFromCornerSplit45 = [this, &nextX, &nextZ](const core::Length& x, const core::Length& z) noexcept
  {
    switch(axisFromAngle(m_state.rotation.Y))
    {
    case core::Axis::Right90:
    case core::Axis::Deg180:
      // move to corner at +X/-Z
      nextX = x;
      break;
    case core::Axis::Deg0:
    case core::Axis::Left90:
      // move to corner at -X/+Z
      nextZ = z;
      break;
    }
  };

  auto moveAwayFromCornerSplitNeg45 = [this, &nextX, &nextZ](const core::Length& x, const core::Length& z) noexcept
  {
    switch(axisFromAngle(m_state.rotation.Y))
    {
    case core::Axis::Left90:
    case core::Axis::Deg180:
      // move to corner at -X/-Z
      nextX = x;
      break;
    case core::Axis::Deg0:
    case core::Axis::Right90:
      // move to corner at +X/+Z
      nextZ = z;
      break;
    }
  };

  if(bottom.Z < collisionFreeRangeZ.min)
  {
    const auto firstCollision = !isFeasiblePosition(bottom - testDz);
    if(firstCollision)
    {
      nextZ = collisionFreeRangeZ.min;
    }

    if(bottom.X < collisionFreeRangeX.min)
    {
      if(!isFeasiblePosition(bottom - testDx))
      {
        nextX = collisionFreeRangeX.min;
      }
      else if(firstCollision && !isFeasiblePosition(bottom - testDz - testDx))
      {
        // can move aligned to -X and -Z, which means the collider is in the corner
        moveAwayFromCornerSplit45(collisionFreeRangeX.min, collisionFreeRangeZ.min);
      }
    }
    else if(bottom.X > collisionFreeRangeX.max)
    {
      if(!isFeasiblePosition(bottom + testDx))
      {
        nextX = collisionFreeRangeX.max;
      }
      else if(firstCollision && !isFeasiblePosition(bottom - testDz + testDx))
      {
        // can move aligned to +X and -Z, which means the collider is in the corner
        moveAwayFromCornerSplitNeg45(collisionFreeRangeX.max, collisionFreeRangeZ.min);
      }
    }
  }
  else if(bottom.Z > collisionFreeRangeZ.max)
  {
    const auto firstCollision = !isFeasiblePosition(bottom + testDz);
    if(firstCollision)
    {
      nextZ = collisionFreeRangeZ.max;
    }

    if(bottom.X < collisionFreeRangeX.min)
    {
      if(!isFeasiblePosition(bottom - testDx))
      {
        nextX = collisionFreeRangeX.min;
      }
      else if(firstCollision && !isFeasiblePosition(bottom + testDz - testDx))
      {
        // can move aligned to -X and +Z, which means the collider is in the corner
        moveAwayFromCornerSplitNeg45(collisionFreeRangeX.min, collisionFreeRangeZ.max);
      }
    }
    else if(bottom.X > collisionFreeRangeX.max)
    {
      if(!isFeasiblePosition(bottom + testDx))
      {
        nextX = collisionFreeRangeX.max;
      }
      else if(firstCollision && !isFeasiblePosition(bottom + testDz + testDx))
      {
        // can move aligned to +X and +Z, which means the collider is in the corner
        moveAwayFromCornerSplit45(collisionFreeRangeX.max, collisionFreeRangeZ.max);
      }
    }
  }
  else
  {
    // in this case, Z is completely valid, so only X needs to be checked

    if(m_state.location.position.X < collisionFreeRangeX.min)
    {
      if(!isFeasiblePosition(bottom - testDx))
      {
        nextX = collisionFreeRangeX.min;
      }
    }
    else if(m_state.location.position.X > collisionFreeRangeX.max)
    {
      if(!isFeasiblePosition(bottom + testDx))
      {
        nextX = collisionFreeRangeX.max;
      }
    }
  }

  if(m_state.location.position.X != nextX || m_state.location.position.Z != nextZ)
  {
    m_state.location.position.X = nextX;
    m_state.location.position.Z = nextZ;

    currentSector = m_state.location.moved(0_len, bbox.y.max, 0_len).updateRoom();

    m_state.rotation.Y += collisionRotationY;
    m_state.rotation.Z += std::clamp(8 * tilt - m_state.rotation.Z, -3_deg, +3_deg);
  }

  // bats temporarily penetrate then floor when dying, so this will trigger
  // BOOST_ASSERT(isFeasiblePosition(m_state.location.position + core::TRVec{0_len, bbox.y.max, 0_len}));

  if(anyMovingEnabledObjectInReach())
  {
    // we would end up colliding with another object
    m_state.location = oldLocation;
    return;
  }

  if(!pathFinder.isFlying())
  {
    static constexpr auto FallSpeed = 64_len;

    if(m_state.location.position.Y > m_state.floor)
    {
      m_state.location.position.Y = m_state.floor;
    }
    else if(m_state.floor > m_state.location.position.Y + FallSpeed)
    {
      m_state.location.position.Y += FallSpeed;
    }
    else if(m_state.location.position.Y < m_state.floor)
    {
      m_state.location.position.Y = m_state.floor;
    }

    m_state.rotation.X = 0_au;

    currentSector = m_state.location.updateRoom();
    BOOST_ASSERT(m_state.location.isValid());
    setCurrentRoom(m_state.location.room);
    m_state.floor
      = HeightInfo::fromFloor(currentSector, m_state.location.position, getWorld().getObjectManager().getObjects()).y;

    return;
  }

  auto moveY
    = std::clamp(m_creatureInfo->target.Y - m_state.location.position.Y, -pathFinder.getFly(), pathFinder.getFly());

  const auto currentFloor = HeightInfo::fromFloor(currentSector,
                                                  m_state.location.position + core::TRVec{0_len, bbox.y.max, 0_len},
                                                  getWorld().getObjectManager().getObjects())
                              .y;

  if(m_state.location.position.Y + moveY > currentFloor)
  {
    // fly movements ended up penetrating the floor

    if(m_state.location.position.Y > currentFloor)
    {
      // current and previous positions penetrate the floor, fly up from the floor
      m_state.location.position.X = oldLocation.position.X;
      m_state.location.position.Z = oldLocation.position.Z;
      moveY = -pathFinder.getFly();
    }
    else
    {
      // transitioning between "above floor" and "penetrating floor", so place on floor
      m_state.location.position.Y = currentFloor;
      moveY = 0_len;
    }
  }
  else
  {
    const auto currentCeiling
      = HeightInfo::fromCeiling(currentSector,
                                m_state.location.position + core::TRVec{0_len, bbox.y.max, 0_len},
                                getWorld().getObjectManager().getObjects())
          .y;

    // TODO nah... why, core, why?
    const auto bboxTopAdjusted = m_state.type == TR1ItemId::CrocodileInWater ? 0_len : bbox.y.min;

    if(m_state.location.position.Y + bboxTopAdjusted + moveY < currentCeiling)
    {
      // fly movements ended up penetrating the ceiling

      if(m_state.location.position.Y + bboxTopAdjusted < currentCeiling)
      {
        // current and previous positions penetrate the floor, fly up from the floor
        m_state.location.position.X = oldLocation.position.X;
        m_state.location.position.Z = oldLocation.position.Z;
        moveY = pathFinder.getFly();
      }
      else
      {
        // transitioning between "below ceiling" and "penetrating ceiling", so place below ceiling
        m_state.location.position.Y = currentCeiling - bboxTopAdjusted;
        moveY = 0_len;
      }
    }
  }

  m_state.location.position.Y += moveY;
  auto bboxBottomLocation = m_state.location.moved(0_len, bbox.y.max, 0_len);
  currentSector = bboxBottomLocation.updateRoom();
  m_state.floor
    = HeightInfo::fromFloor(currentSector, bboxBottomLocation.position, getWorld().getObjectManager().getObjects()).y;

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
  getSkeleton()->getRenderState().setScissorTest(false);
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
  if(!enemyLocation.laraInView || enemyLocation.distance >= util::square(7_sectors))
  {
    return false;
  }

  return raycastLineOfSight(m_state.location.moved(0_len, -768_len, 0_len),
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
  if(distance <= util::square(7_sectors))
  {
    if(util::rand15() < (util::square(7_sectors) - distance) / util::square(40_len) - 8192)
    {
      isHit = true;
    }
  }

  if(isHit)
  {
    lara.emitParticle(core::TRVec{}, util::rand15(lara.getSkeleton()->getBoneCount()), &createBloodSplat);

    if(!lara.isInWater())
      lara.playSoundEffect(TR1SoundEffect::BulletHitsLara);
  }
  else
  {
    auto location = lara.m_state.location;
    location.position.X += util::rand15s(1_sectors / 2);
    location.position.Y = lara.m_state.floor;
    location.position.Z += util::rand15s(1_sectors / 2);
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

void AIAgent::serialize(const serialization::Serializer<world::World>& ser) const
{
  ModelObject::serialize(ser);
  ser(S_NV("collisionRadius", m_collisionRadius), S_NV("creatureInfo", m_creatureInfo));
}

void AIAgent::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ModelObject::deserialize(ser);
  ser(S_NV("collisionRadius", m_collisionRadius), S_NV("creatureInfo", m_creatureInfo));
  getSkeleton()->getRenderState().setScissorTest(false);
}

void AIAgent::initCreatureInfo()
{
  if(m_creatureInfo != nullptr)
    return;

  m_creatureInfo = std::make_unique<ai::CreatureInfo>(getWorld(), m_state.type, m_state.getCurrentBox());
}

bool AIAgent::isInsideZoneButNotInBox(const uint32_t zoneId, const world::Box& targetBox) const
{
  gsl_Expects(m_creatureInfo != nullptr);

  const auto zoneRef = world::Box::getZoneRef(
    getWorld().roomsAreSwapped(), m_creatureInfo->pathFinder.isFlying(), m_creatureInfo->pathFinder.getStep());

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
