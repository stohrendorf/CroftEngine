#include "ai.h"

#include "engine/engine.h"
#include "engine/objects/laraobject.h"
#include "engine/script/reflection.h"
#include "engine/world/world.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"

namespace engine::ai
{
namespace
{
std::optional<ai::Mood>
  getNewViolentMood(const EnemyLocation& enemyLocation, const ai::CreatureInfo& creatureInfo, bool isHit)
{
  switch(creatureInfo.mood)
  {
  case Mood::Bored:
  case Mood::Stalk:
    if(enemyLocation.canReachEnemyZone())
      return Mood::Attack;
    else if(isHit)
      return Mood::Escape;
    break;
  case Mood::Attack:
    if(!enemyLocation.canReachEnemyZone())
      return Mood::Bored;
    break;
  case Mood::Escape:
    if(enemyLocation.canReachEnemyZone())
      return Mood::Attack;
    break;
  }

  return std::nullopt;
}

std::optional<ai::Mood> getNewNonViolentMood(const EnemyLocation& enemyLocation,
                                             const ai::CreatureInfo& creatureInfo,
                                             bool isHit,
                                             bool hasTargetBox)
{
  switch(creatureInfo.mood)
  {
  case Mood::Bored: [[fallthrough]];
  case Mood::Stalk:
    if(isHit && (util::rand15() < 2048 || !enemyLocation.canReachEnemyZone()))
    {
      return Mood::Escape;
    }
    else if(enemyLocation.canReachEnemyZone())
    {
      if(enemyLocation.enemyDistance >= util::square(3 * core::SectorSize)
         && (creatureInfo.mood != Mood::Stalk || hasTargetBox))
      {
        return Mood::Stalk;
      }
      else
      {
        return Mood::Attack;
      }
    }
    break;
  case Mood::Attack:
    if(isHit && (util::rand15() < 2048 || !enemyLocation.canReachEnemyZone()))
    {
      return Mood::Escape;
    }
    else if(!enemyLocation.canReachEnemyZone())
    {
      return Mood::Bored;
    }
    break;
  case Mood::Escape:
    if(enemyLocation.canReachEnemyZone() && util::rand15() < 256)
    {
      return Mood::Stalk;
    }
    break;
  }

  return std::nullopt;
}

std::optional<ai::Mood> getNewMood(
  const EnemyLocation& enemyLocation, const ai::CreatureInfo& creatureInfo, bool isHit, bool violent, bool hasTargetBox)
{
  if(violent)
  {
    return getNewViolentMood(enemyLocation, creatureInfo, isHit);
  }
  else
  {
    return getNewNonViolentMood(enemyLocation, creatureInfo, isHit, hasTargetBox);
  }
}
} // namespace

void updateMood(const world::World& world,
                const objects::ObjectState& objectState,
                const EnemyLocation& enemyLocation,
                const bool violent)
{
  if(objectState.creatureInfo == nullptr)
    return;

  CreatureInfo& creatureInfo = *objectState.creatureInfo;
  auto newTargetBox = creatureInfo.pathFinder.getTargetBox();
  if(creatureInfo.pathFinder.isUnreachable(objectState.getCurrentBox()))
  {
    newTargetBox = nullptr;
  }

  if(creatureInfo.mood != Mood::Attack && newTargetBox != nullptr
     && !objectState.isInsideZoneButNotInBox(world, enemyLocation.zoneId, *creatureInfo.pathFinder.getTargetBox()))
  {
    if(enemyLocation.canReachEnemyZone())
    {
      creatureInfo.mood = Mood::Bored;
    }
    newTargetBox = nullptr;
  }
  const auto originalMood = creatureInfo.mood;
  if(world.getObjectManager().getLara().isDead())
    creatureInfo.mood = Mood::Bored;
  else if(auto newMood = getNewMood(enemyLocation, creatureInfo, objectState.is_hit, violent, newTargetBox != nullptr))
    creatureInfo.mood = newMood.value();

  if(originalMood != creatureInfo.mood)
  {
    if(originalMood == Mood::Attack)
    {
      Expects(creatureInfo.pathFinder.getTargetBox() != nullptr);
      creatureInfo.pathFinder.setRandomSearchTarget(creatureInfo.pathFinder.getTargetBox());
    }
    newTargetBox = nullptr;
  }

  switch(creatureInfo.mood)
  {
  case Mood::Attack:
    if(util::rand15()
       >= pybind11::globals()["getObjectInfo"](objectState.type.get()).cast<script::ObjectInfo>().target_update_chance)
      break;

    creatureInfo.pathFinder.target = world.getObjectManager().getLara().m_state.location.position;
    newTargetBox = world.getObjectManager().getLara().m_state.getCurrentBox();
    creatureInfo.pathFinder.target.X
      = std::clamp(creatureInfo.pathFinder.target.X, newTargetBox->xmin, newTargetBox->xmax);
    creatureInfo.pathFinder.target.Z
      = std::clamp(creatureInfo.pathFinder.target.Z, newTargetBox->zmin, newTargetBox->zmax);
    if(creatureInfo.pathFinder.isFlying() && world.getObjectManager().getLara().isOnLand())
      creatureInfo.pathFinder.target.Y += world.getObjectManager()
                                            .getLara()
                                            .getSkeleton()
                                            ->getInterpolationInfo()
                                            .getNearestFrame()
                                            ->bbox.toBBox()
                                            .minY;

    break;
  case Mood::Bored:
  {
    const auto box = creatureInfo.pathFinder.getRandomBox();
    if(!objectState.isInsideZoneButNotInBox(world, enemyLocation.zoneId, *box))
      break;

    if(objectState.isStalkBox(world, *box))
    {
      newTargetBox = box;
      creatureInfo.pathFinder.setRandomSearchTarget(box);
      creatureInfo.mood = Mood::Stalk;
    }
    else if(newTargetBox == nullptr)
    {
      newTargetBox = box;
      creatureInfo.pathFinder.setRandomSearchTarget(box);
    }
    break;
  }
  case Mood::Stalk:
  {
    if(newTargetBox != nullptr && objectState.isStalkBox(world, *newTargetBox))
      break;

    const auto box = creatureInfo.pathFinder.getRandomBox();
    if(!objectState.isInsideZoneButNotInBox(world, enemyLocation.zoneId, *box))
      break;

    if(objectState.isStalkBox(world, *box))
    {
      newTargetBox = box;
      creatureInfo.pathFinder.setRandomSearchTarget(box);
    }
    else if(newTargetBox == nullptr)
    {
      newTargetBox = box;
      creatureInfo.pathFinder.setRandomSearchTarget(box);
      if(!enemyLocation.canReachEnemyZone())
      {
        creatureInfo.mood = Mood::Bored;
      }
    }
    break;
  }
  case Mood::Escape:
  {
    const auto box = creatureInfo.pathFinder.getRandomBox();
    if(!objectState.isInsideZoneButNotInBox(world, enemyLocation.zoneId, *box) || newTargetBox != nullptr)
      break;

    if(objectState.isEscapeBox(world, *box))
    {
      newTargetBox = box;
      creatureInfo.pathFinder.setRandomSearchTarget(box);
    }
    else if(enemyLocation.canReachEnemyZone() && objectState.isStalkBox(world, *box))
    {
      newTargetBox = box;
      creatureInfo.pathFinder.setRandomSearchTarget(box);
      creatureInfo.mood = Mood::Stalk;
    }
    break;
  }
  }

  if(creatureInfo.pathFinder.getTargetBox() == nullptr)
  {
    newTargetBox = objectState.getCurrentBox();
    creatureInfo.pathFinder.setRandomSearchTarget(objectState.getCurrentBox());
  }
  if(newTargetBox != nullptr)
    creatureInfo.pathFinder.setTargetBox(newTargetBox);
  creatureInfo.pathFinder.calculateTarget(
    world, creatureInfo.target, objectState.location.position, objectState.getCurrentBox());
}

std::shared_ptr<CreatureInfo> create(const serialization::TypeId<std::shared_ptr<CreatureInfo>>&,
                                     const serialization::Serializer<world::World>& ser)
{
  if(!ser.node.has_val())
    return nullptr;

  auto result = std::make_shared<CreatureInfo>(ser.context);
  ser(S_NV("data", *result));
  return result;
}

void serialize(std::shared_ptr<CreatureInfo>& data, const serialization::Serializer<world::World>& ser)
{
  if(ser.loading)
  {
    data = create(serialization::TypeId<std::shared_ptr<CreatureInfo>>{}, ser);
  }
  else
  {
    if(data == nullptr)
    {
      ser.node |= ryml::MAP;
    }
    else
    {
      ser(S_NV("data", *data));
    }
  }
}

EnemyLocation::EnemyLocation(world::World& world, objects::ObjectState& objectState)
{
  if(objectState.creatureInfo == nullptr)
    return;

  const auto zoneRef = world::Box::getZoneRef(world.roomsAreSwapped(),
                                              objectState.creatureInfo->pathFinder.isFlying(),
                                              objectState.creatureInfo->pathFinder.step);

  zoneId = objectState.getCurrentBox().get()->*zoneRef;
  world.getObjectManager().getLara().m_state.getCurrentBox()
    = world.getObjectManager().getLara().m_state.getCurrentBox();
  enemyZoneId = world.getObjectManager().getLara().m_state.getCurrentBox().get()->*zoneRef;
  enemyUnreachable
    = !objectState.creatureInfo->pathFinder.canVisit(*world.getObjectManager().getLara().m_state.getCurrentBox())
      || objectState.creatureInfo->pathFinder.isUnreachable(objectState.getCurrentBox());

  auto objectInfo = pybind11::globals()["getObjectInfo"](objectState.type.get()).cast<script::ObjectInfo>();
  const core::Length pivotLength{objectInfo.pivot_length};
  const auto toLara = world.getObjectManager().getLara().m_state.location.position
                      - (objectState.location.position + util::pitch(pivotLength, objectState.rotation.Y));
  const auto angleToLara = core::angleFromAtan(toLara.X, toLara.Z);
  enemyDistance = util::square(toLara.X) + util::square(toLara.Z);
  angleToEnemy = angleToLara - objectState.rotation.Y;
  enemyAngleToSelf = angleToLara - 180_deg - world.getObjectManager().getLara().m_state.rotation.Y;
  enemyAhead = angleToEnemy > -90_deg && angleToEnemy < 90_deg;
  if(enemyAhead)
  {
    const auto laraY = world.getObjectManager().getLara().m_state.location.position.Y;
    canAttackForward = objectState.location.position.Y - core::QuarterSectorSize < laraY
                       && objectState.location.position.Y + core::QuarterSectorSize > laraY;
  }
}

CreatureInfo::CreatureInfo(const world::World& world)
    : pathFinder{world}
{
}

CreatureInfo::CreatureInfo(const world::World& world,
                           const core::TypeId type,
                           const gsl::not_null<const world::Box*>& initialBox)
    : CreatureInfo{world}
{
  auto objectInfo = pybind11::globals()["getObjectInfo"](type.get()).cast<script::ObjectInfo>();
  pathFinder.step = core::Length{objectInfo.step_limit};
  pathFinder.drop = core::Length{objectInfo.drop_limit};
  pathFinder.fly = core::Length{objectInfo.fly_limit};
  pathFinder.cannotVisitBlockable = objectInfo.cannot_visit_blocked;
  pathFinder.cannotVisitBlocked = objectInfo.cannot_visit_blockable;

  pathFinder.collectBoxes(world, initialBox);
}

void CreatureInfo::serialize(const serialization::Serializer<world::World>& ser)
{
  ser(S_NV("headRotation", headRotation),
      S_NV("neckRotation", neckRotation),
      S_NV("maxTurnSpeed", maxTurnSpeed),
      S_NV("mood", mood),
      S_NV("pathFinder", pathFinder),
      S_NV("target", target));
}
} // namespace engine::ai
