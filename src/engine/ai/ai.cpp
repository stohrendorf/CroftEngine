#include "ai.h"

#include "core/angle.h"
#include "core/boundingbox.h"
#include "core/genericvec.h"
#include "core/interval.h"
#include "core/magic.h"
#include "engine/engine.h"
#include "engine/items_tr1.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/objects/aiagent.h"
#include "engine/objects/laraobject.h"
#include "engine/objects/objectstate.h"
#include "engine/script/reflection.h"
#include "engine/script/scriptengine.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/box.h"
#include "engine/world/world.h"
#include "loader/file/animation.h"
#include "pathfinder.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "util/helpers.h"

#include <exception>
#include <map>
#include <optional>

namespace engine::ai
{
namespace
{
std::optional<ai::Mood>
  getNewViolentMood(const EnemyLocation& enemyLocation, const ai::CreatureInfo& creatureInfo, bool isHit) noexcept
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
  case Mood::Bored:
    [[fallthrough]];
  case Mood::Stalk:
    if(isHit && (util::rand15() < 2048 || !enemyLocation.canReachEnemyZone()))
    {
      return Mood::Escape;
    }
    else if(enemyLocation.canReachEnemyZone())
    {
      if(enemyLocation.enemyDistance >= util::square(3_sectors) && (creatureInfo.mood != Mood::Stalk || hasTargetBox))
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

void updateMood(const objects::AIAgent& aiAgent, const EnemyLocation& enemyLocation, const bool violent)
{
  if(aiAgent.getCreatureInfo() == nullptr)
    return;

  CreatureInfo& creatureInfo = *aiAgent.getCreatureInfo();
  auto newTargetBox = creatureInfo.pathFinder.getTargetBox();
  gsl_Assert(newTargetBox != nullptr);
  auto& lara = aiAgent.getWorld().getObjectManager().getLara();
  if(creatureInfo.pathFinder.isUnreachable(lara.m_state.getCurrentBox()))
  {
    // can't reach lara
    newTargetBox = nullptr;
  }

  if(creatureInfo.mood != Mood::Attack && newTargetBox != nullptr
     && !aiAgent.isInsideZoneButNotInBox(enemyLocation.zoneId, *newTargetBox))
  {
    // if we're not attacking, but we got a target, we're seeking for a box that is *not* the enemy's box, but we're too
    // close to the enemy now.
    if(enemyLocation.canReachEnemyZone())
    {
      creatureInfo.mood = Mood::Bored;
    }
    newTargetBox = nullptr;
  }
  const auto originalMood = creatureInfo.mood;
  if(lara.isDead())
  {
    creatureInfo.mood = Mood::Bored;
  }
  else if(auto newMood
          = getNewMood(enemyLocation, creatureInfo, aiAgent.m_state.is_hit, violent, newTargetBox != nullptr);
          newMood.has_value())
  {
    creatureInfo.mood = *newMood;
  }

  if(originalMood != creatureInfo.mood)
  {
    if(originalMood == Mood::Attack)
    {
      gsl_Assert(creatureInfo.pathFinder.getTargetBox() != nullptr);
      creatureInfo.pathFinder.setRandomSearchTarget(gsl::not_null{creatureInfo.pathFinder.getTargetBox()});
    }
    newTargetBox = nullptr;
  }

  switch(creatureInfo.mood)
  {
  case Mood::Attack:
    // when attacking, there's a chance we will update our own target location
    if(util::rand15() >= aiAgent.getWorld()
                           .getEngine()
                           .getScriptEngine()
                           .getGameflow()
                           .getObjectInfos()
                           .at(aiAgent.m_state.type.get_as<TR1ItemId>())
                           ->target_update_chance)
      break;

    {
      newTargetBox = lara.m_state.getCurrentBox();
      auto pos = lara.m_state.location.position;
      if(creatureInfo.pathFinder.isFlying() && lara.isOnLand())
      {
        // target the head
        pos.Y += lara.getSkeleton()->getInterpolationInfo().getNearestFrame()->bbox.toBBox().y.min;
      }

      creatureInfo.pathFinder.setTarget(pos);
    }

    break;
  case Mood::Bored:
  {
    // when bored, randomly walk around
    const auto box = creatureInfo.pathFinder.getRandomBox();
    if(!aiAgent.isInsideZoneButNotInBox(enemyLocation.zoneId, *box))
      break;

    if(aiAgent.m_state.isStalkBox(aiAgent.getWorld(), *box))
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
    // when stalking, seek a location behind lara
    if(newTargetBox != nullptr && aiAgent.m_state.isStalkBox(aiAgent.getWorld(), *newTargetBox))
      break;

    const auto box = creatureInfo.pathFinder.getRandomBox();
    if(!aiAgent.isInsideZoneButNotInBox(enemyLocation.zoneId, *box))
      break;

    if(aiAgent.m_state.isStalkBox(aiAgent.getWorld(), *box))
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
    // when escaping, seek a box behind lara which center is at least 5 sectors away
    const auto box = creatureInfo.pathFinder.getRandomBox();
    if(!aiAgent.isInsideZoneButNotInBox(enemyLocation.zoneId, *box) || newTargetBox != nullptr)
      break;

    if(aiAgent.m_state.isEscapeBox(aiAgent.getWorld(), *box))
    {
      newTargetBox = box;
      creatureInfo.pathFinder.setRandomSearchTarget(box);
    }
    else if(enemyLocation.canReachEnemyZone() && aiAgent.m_state.isStalkBox(aiAgent.getWorld(), *box))
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
    // init the pathfinder's target if it's not initialised yet
    newTargetBox = aiAgent.m_state.getCurrentBox();
    creatureInfo.pathFinder.setRandomSearchTarget(aiAgent.m_state.getCurrentBox());
  }

  if(newTargetBox != nullptr)
  {
    // we want to move to a different box
    creatureInfo.pathFinder.setTargetBox(gsl::not_null{newTargetBox});
  }

  creatureInfo.pathFinder.calculateTarget(
    aiAgent.getWorld(), creatureInfo.target, aiAgent.m_state.location.position, aiAgent.m_state.getCurrentBox());
}

std::unique_ptr<CreatureInfo> create(const serialization::TypeId<std::unique_ptr<CreatureInfo>>&,
                                     const serialization::Deserializer<world::World>& ser)
{
  if(ser.isNull())
    return nullptr;

  auto result = std::make_unique<CreatureInfo>();
  ser(S_NV("data", *result));
  return result;
}

void serialize(const std::unique_ptr<CreatureInfo>& data, const serialization::Serializer<world::World>& ser)
{
  if(data == nullptr)
  {
    ser.setNull();
  }
  else
  {
    ser(S_NV("data", *data));
  }
}

void deserialize(std::unique_ptr<CreatureInfo>& data, const serialization::Deserializer<world::World>& ser)
{
  data = create(serialization::TypeId<std::unique_ptr<CreatureInfo>>{}, ser);
}

EnemyLocation::EnemyLocation(objects::AIAgent& aiAgent)
{
  if(aiAgent.getCreatureInfo() == nullptr)
    return;

  const auto zoneRef = world::Box::getZoneRef(aiAgent.getWorld().roomsAreSwapped(),
                                              aiAgent.getCreatureInfo()->pathFinder.isFlying(),
                                              aiAgent.getCreatureInfo()->pathFinder.getStep());

  const auto aiAgentBox = aiAgent.m_state.getCurrentBox();
  zoneId = aiAgentBox.get()->*zoneRef;
  const auto& lara = aiAgent.getWorld().getObjectManager().getLara();
  const auto laraBox = lara.m_state.tryGetCurrentBox();
  enemyZoneId = laraBox == nullptr ? InvalidZone : laraBox->*zoneRef;
  if(laraBox == nullptr)
  {
    enemyUnreachable = true;
  }
  else
  {
    enemyUnreachable = !aiAgent.getCreatureInfo()->pathFinder.canVisit(*laraBox)
                       || aiAgent.getCreatureInfo()->pathFinder.isUnreachable(aiAgentBox);
  }

  const gsl::not_null objectInfo{aiAgent.getWorld().getEngine().getScriptEngine().getGameflow().getObjectInfos().at(
    aiAgent.m_state.type.get_as<TR1ItemId>())};
  const core::Length pivotLength{objectInfo->pivot_length};
  const auto pivotToLara = lara.m_state.location.position
                           - (aiAgent.m_state.location.position + util::pitch(pivotLength, aiAgent.m_state.rotation.Y));
  const auto anglePivotToLara = core::angleFromAtan(pivotToLara.X, pivotToLara.Z);
  enemyDistance = util::square(pivotToLara.X) + util::square(pivotToLara.Z);
  visualAngleToLara = anglePivotToLara - aiAgent.m_state.rotation.Y;
  visualLaraAngleToSelf = anglePivotToLara - 180_deg - lara.m_state.rotation.Y;
  laraInView = abs(visualAngleToLara) < 90_deg;
  if(laraInView)
  {
    const auto laraY = lara.m_state.location.position.Y;
    canAttackLara = laraY > aiAgent.m_state.location.position.Y - core::QuarterSectorSize
                    && laraY < aiAgent.m_state.location.position.Y + core::QuarterSectorSize;
  }
}

CreatureInfo::CreatureInfo(const world::World& world,
                           const core::TypeId& type,
                           const gsl::not_null<const world::Box*>& initialBox)
{
  const auto& objectInfo
    = *world.getEngine().getScriptEngine().getGameflow().getObjectInfos().at(type.get_as<TR1ItemId>());

  pathFinder.init(world, initialBox, objectInfo);
}

void CreatureInfo::serialize(const serialization::Serializer<world::World>& ser) const
{
  ser(S_NV("headRotation", headRotation),
      S_NV("neckRotation", neckRotation),
      S_NV("maxTurnSpeed", maxTurnSpeed),
      S_NV("mood", mood),
      S_NV("pathFinder", pathFinder),
      S_NV("target", target));
}

void CreatureInfo::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ser(S_NV("headRotation", headRotation),
      S_NV("neckRotation", neckRotation),
      S_NV("maxTurnSpeed", maxTurnSpeed),
      S_NV("mood", mood),
      S_NV("pathFinder", pathFinder),
      S_NV("target", target));
}
} // namespace engine::ai
