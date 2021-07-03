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
std::optional<ai::Mood> getNewViolentMood(const AiInfo& aiInfo, const ai::CreatureInfo& creatureInfo, bool isHit)
{
  switch(creatureInfo.mood)
  {
  case Mood::Bored:
  case Mood::Stalk:
    if(aiInfo.canReachEnemyZone())
      return Mood::Attack;
    else if(isHit)
      return Mood::Escape;
    break;
  case Mood::Attack:
    if(!aiInfo.canReachEnemyZone())
      return Mood::Bored;
    break;
  case Mood::Escape:
    if(aiInfo.canReachEnemyZone())
      return Mood::Attack;
    break;
  }

  return std::nullopt;
}

std::optional<ai::Mood>
  getNewNonViolentMood(const AiInfo& aiInfo, const ai::CreatureInfo& creatureInfo, bool isHit, bool hasTargetBox)
{
  switch(creatureInfo.mood)
  {
  case Mood::Bored: [[fallthrough]];
  case Mood::Stalk:
    if(isHit && (util::rand15() < 2048 || !aiInfo.canReachEnemyZone()))
    {
      return Mood::Escape;
    }
    else if(aiInfo.canReachEnemyZone())
    {
      if(aiInfo.distance >= util::square(3 * core::SectorSize) && (creatureInfo.mood != Mood::Stalk || hasTargetBox))
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
    if(isHit && (util::rand15() < 2048 || !aiInfo.canReachEnemyZone()))
    {
      return Mood::Escape;
    }
    else if(!aiInfo.canReachEnemyZone())
    {
      return Mood::Bored;
    }
    break;
  case Mood::Escape:
    if(aiInfo.canReachEnemyZone() && util::rand15() < 256)
    {
      return Mood::Stalk;
    }
    break;
  }

  return std::nullopt;
}

std::optional<ai::Mood>
  getNewMood(const AiInfo& aiInfo, const ai::CreatureInfo& creatureInfo, bool isHit, bool violent, bool hasTargetBox)
{
  if(violent)
  {
    return getNewViolentMood(aiInfo, creatureInfo, isHit);
  }
  else
  {
    return getNewNonViolentMood(aiInfo, creatureInfo, isHit, hasTargetBox);
  }
}
} // namespace

void updateMood(const world::World& world,
                const objects::ObjectState& objectState,
                const AiInfo& aiInfo,
                const bool violent)
{
  if(objectState.creatureInfo == nullptr)
    return;

  CreatureInfo& creatureInfo = *objectState.creatureInfo;
  auto newTargetBox = creatureInfo.pathFinder.getTargetBox();
  if(creatureInfo.pathFinder.isUnreachable(objectState.box))
  {
    newTargetBox = nullptr;
  }

  if(creatureInfo.mood != Mood::Attack && newTargetBox != nullptr
     && !objectState.isInsideZoneButNotInBox(world, aiInfo.zone_number, *creatureInfo.pathFinder.getTargetBox()))
  {
    if(aiInfo.canReachEnemyZone())
    {
      creatureInfo.mood = Mood::Bored;
    }
    newTargetBox = nullptr;
  }
  const auto originalMood = creatureInfo.mood;
  if(world.getObjectManager().getLara().isDead())
    creatureInfo.mood = Mood::Bored;
  else if(auto newMood = getNewMood(aiInfo, creatureInfo, objectState.is_hit, violent, newTargetBox != nullptr))
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

    creatureInfo.pathFinder.target = world.getObjectManager().getLara().m_state.position.position;
    newTargetBox = world.getObjectManager().getLara().m_state.box;
    creatureInfo.pathFinder.target.X
      = std::clamp(creatureInfo.pathFinder.target.X, newTargetBox->xmin, newTargetBox->xmax);
    creatureInfo.pathFinder.target.Z
      = std::clamp(creatureInfo.pathFinder.target.Z, newTargetBox->zmin, newTargetBox->zmax);
    if(creatureInfo.pathFinder.fly != 0_len && world.getObjectManager().getLara().isOnLand())
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
    if(!objectState.isInsideZoneButNotInBox(world, aiInfo.zone_number, *box))
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
    if(!objectState.isInsideZoneButNotInBox(world, aiInfo.zone_number, *box))
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
      if(!aiInfo.canReachEnemyZone())
      {
        creatureInfo.mood = Mood::Bored;
      }
    }
    break;
  }
  case Mood::Escape:
  {
    const auto box = creatureInfo.pathFinder.getRandomBox();
    if(!objectState.isInsideZoneButNotInBox(world, aiInfo.zone_number, *box) || newTargetBox != nullptr)
      break;

    if(objectState.isEscapeBox(world, *box))
    {
      newTargetBox = box;
      creatureInfo.pathFinder.setRandomSearchTarget(box);
    }
    else if(aiInfo.canReachEnemyZone() && objectState.isStalkBox(world, *box))
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
    newTargetBox = objectState.box;
    creatureInfo.pathFinder.setRandomSearchTarget(objectState.box);
  }
  if(newTargetBox != nullptr)
    creatureInfo.pathFinder.setTargetBox(newTargetBox);
  creatureInfo.pathFinder.calculateTarget(world, creatureInfo.target, objectState);
}

std::shared_ptr<CreatureInfo> create(const serialization::TypeId<std::shared_ptr<CreatureInfo>>&,
                                     const serialization::Serializer<world::World>& ser)
{
  if(!ser.node.has_val())
    return nullptr;

  auto result = std::make_shared<CreatureInfo>(ser.context, core::TypeId::create(ser["type"]));
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
      ser(S_NV("type", data->type), S_NV("data", *data));
    }
  }
}

AiInfo::AiInfo(world::World& world, objects::ObjectState& objectState)
{
  if(objectState.creatureInfo == nullptr)
    return;

  const auto zoneRef = world::Box::getZoneRef(
    world.roomsAreSwapped(), objectState.creatureInfo->pathFinder.fly, objectState.creatureInfo->pathFinder.step);

  objectState.box = objectState.getCurrentSector()->box;
  zone_number = objectState.box->*zoneRef;
  world.getObjectManager().getLara().m_state.box = world.getObjectManager().getLara().m_state.getCurrentSector()->box;
  enemy_zone = world.getObjectManager().getLara().m_state.box->*zoneRef;
  enemy_unreachable = !objectState.creatureInfo->pathFinder.canVisit(*world.getObjectManager().getLara().m_state.box)
                      || objectState.creatureInfo->pathFinder.isUnreachable(objectState.box);

  auto objectInfo = pybind11::globals()["getObjectInfo"](objectState.type.get()).cast<script::ObjectInfo>();
  const core::Length pivotLength{objectInfo.pivot_length};
  const auto d = world.getObjectManager().getLara().m_state.position.position
                 - (objectState.position.position + util::pitch(pivotLength, objectState.rotation.Y));
  const auto pivotAngle = core::angleFromAtan(d.X, d.Z);
  distance = util::square(d.X) + util::square(d.Z);
  angle = pivotAngle - objectState.rotation.Y;
  enemy_facing = pivotAngle - 180_deg - world.getObjectManager().getLara().m_state.rotation.Y;
  ahead = angle > -90_deg && angle < 90_deg;
  if(ahead)
  {
    const auto laraY = world.getObjectManager().getLara().m_state.position.position.Y;
    if(objectState.position.position.Y - core::QuarterSectorSize < laraY
       && objectState.position.position.Y + core::QuarterSectorSize > laraY)
    {
      bite = true;
    }
  }
}

CreatureInfo::CreatureInfo(const world::World& world, const core::TypeId type)
    : type{type}
    , pathFinder{world}
{
  switch(type.get_as<TR1ItemId>())
  {
  case TR1ItemId::Wolf:
  case TR1ItemId::LionMale:
  case TR1ItemId::LionFemale:
  case TR1ItemId::Panther: pathFinder.drop = -core::SectorSize; break;

  case TR1ItemId::Bat:
  case TR1ItemId::CrocodileInWater:
  case TR1ItemId::Fish:
    pathFinder.step = 20 * core::SectorSize;
    pathFinder.drop = -20 * core::SectorSize;
    pathFinder.fly = 16_len;
    break;

  case TR1ItemId::Gorilla:
    pathFinder.step = core::SectorSize / 2;
    pathFinder.drop = -core::SectorSize;
    break;

  case TR1ItemId::TRex:
  case TR1ItemId::FlyingMutant:
  case TR1ItemId::CentaurMutant:
    pathFinder.cannotVisitBlockable = true;
    pathFinder.cannotVisitBlocked = false;
    break;

  default:
    // silence compiler
    break;
  }
}

void CreatureInfo::serialize(const serialization::Serializer<world::World>& ser)
{
  ser(S_NV("headRotation", head_rotation),
      S_NV("neckRotation", neck_rotation),
      S_NV("maximumTurn", maximum_turn),
      S_NV("mood", mood),
      S_NV("pathFinder", pathFinder),
      S_NV("target", target));
}
} // namespace engine::ai
