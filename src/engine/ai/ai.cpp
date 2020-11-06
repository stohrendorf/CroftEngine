#include "ai.h"

#include "engine/objects/laraobject.h"
#include "engine/script/reflection.h"
#include "serialization/quantity.h"

namespace engine::ai
{
void updateMood(const Engine& engine, const objects::ObjectState& objectState, const AiInfo& aiInfo, const bool violent)
{
  if(objectState.creatureInfo == nullptr)
    return;

  CreatureInfo& creatureInfo = *objectState.creatureInfo;
  if(!creatureInfo.pathFinder.nodes[objectState.box].traversable
     && creatureInfo.pathFinder.visited.count(objectState.box) != 0)
  {
    creatureInfo.pathFinder.required_box = nullptr;
  }

  if(creatureInfo.mood != Mood::Attack && creatureInfo.pathFinder.required_box != nullptr
     && !objectState.isInsideZoneButNotInBox(engine, aiInfo.zone_number, *creatureInfo.pathFinder.target_box))
  {
    if(aiInfo.canReachEnemyZone())
    {
      creatureInfo.mood = Mood::Bored;
    }
    creatureInfo.pathFinder.required_box = nullptr;
  }
  const auto originalMood = creatureInfo.mood;
  if(engine.getObjectManager().getLara().m_state.health <= 0_hp)
  {
    creatureInfo.mood = Mood::Bored;
  }
  else if(violent)
  {
    switch(creatureInfo.mood)
    {
    case Mood::Bored:
    case Mood::Stalk:
      if(aiInfo.canReachEnemyZone())
      {
        creatureInfo.mood = Mood::Attack;
      }
      else if(objectState.is_hit)
      {
        creatureInfo.mood = Mood::Escape;
      }
      break;
    case Mood::Attack:
      if(!aiInfo.canReachEnemyZone())
      {
        creatureInfo.mood = Mood::Bored;
      }
      break;
    case Mood::Escape:
      if(aiInfo.canReachEnemyZone())
      {
        creatureInfo.mood = Mood::Attack;
      }
      break;
    }
  }
  else
  {
    switch(creatureInfo.mood)
    {
    case Mood::Bored:
    case Mood::Stalk:
      if(objectState.is_hit && (util::rand15() < 2048 || !aiInfo.canReachEnemyZone()))
      {
        creatureInfo.mood = Mood::Escape;
      }
      else if(aiInfo.canReachEnemyZone())
      {
        if(aiInfo.distance >= util::square(3 * core::SectorSize)
           && (creatureInfo.mood != Mood::Stalk || creatureInfo.pathFinder.required_box != nullptr))
        {
          creatureInfo.mood = Mood::Stalk;
        }
        else
        {
          creatureInfo.mood = Mood::Attack;
        }
      }
      break;
    case Mood::Attack:
      if(objectState.is_hit && (util::rand15() < 2048 || !aiInfo.canReachEnemyZone()))
      {
        creatureInfo.mood = Mood::Escape;
      }
      else if(!aiInfo.canReachEnemyZone())
      {
        creatureInfo.mood = Mood::Bored;
      }
      break;
    case Mood::Escape:
      if(aiInfo.canReachEnemyZone() && util::rand15() < 256)
      {
        creatureInfo.mood = Mood::Stalk;
      }
      break;
    }
  }

  if(originalMood != creatureInfo.mood)
  {
    if(originalMood == Mood::Attack)
    {
      Expects(creatureInfo.pathFinder.target_box != nullptr);
      creatureInfo.pathFinder.setRandomSearchTarget(creatureInfo.pathFinder.target_box);
    }
    creatureInfo.pathFinder.required_box = nullptr;
  }

  switch(creatureInfo.mood)
  {
  case Mood::Attack:
    if(util::rand15()
       >= pybind11::globals()["getObjectInfo"](objectState.type.get()).cast<script::ObjectInfo>().target_update_chance)
      break;

    creatureInfo.pathFinder.target = engine.getObjectManager().getLara().m_state.position.position;
    creatureInfo.pathFinder.required_box = engine.getObjectManager().getLara().m_state.box;
    if(creatureInfo.pathFinder.fly != 0_len && engine.getObjectManager().getLara().isOnLand())
      creatureInfo.pathFinder.target.Y += engine.getObjectManager()
                                            .getLara()
                                            .getSkeleton()
                                            ->getInterpolationInfo()
                                            .getNearestFrame()
                                            ->bbox.toBBox()
                                            .minY;

    break;
  case Mood::Bored:
  {
    const auto box = creatureInfo.pathFinder.boxes[util::rand15(creatureInfo.pathFinder.boxes.size())];
    if(!objectState.isInsideZoneButNotInBox(engine, aiInfo.zone_number, *box))
      break;

    if(objectState.stalkBox(engine, *box))
    {
      creatureInfo.pathFinder.setRandomSearchTarget(box);
      creatureInfo.mood = Mood::Stalk;
    }
    else if(creatureInfo.pathFinder.required_box == nullptr)
    {
      creatureInfo.pathFinder.setRandomSearchTarget(box);
    }
    break;
  }
  case Mood::Stalk:
  {
    if(creatureInfo.pathFinder.required_box != nullptr
       && objectState.stalkBox(engine, *creatureInfo.pathFinder.required_box))
      break;

    const auto box = creatureInfo.pathFinder.boxes[util::rand15(creatureInfo.pathFinder.boxes.size())];
    if(!objectState.isInsideZoneButNotInBox(engine, aiInfo.zone_number, *box))
      break;

    if(objectState.stalkBox(engine, *box))
    {
      creatureInfo.pathFinder.setRandomSearchTarget(box);
    }
    else if(creatureInfo.pathFinder.required_box == nullptr)
    {
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
    const auto box = creatureInfo.pathFinder.boxes[util::rand15(creatureInfo.pathFinder.boxes.size())];
    if(!objectState.isInsideZoneButNotInBox(engine, aiInfo.zone_number, *box)
       || creatureInfo.pathFinder.required_box != nullptr)
      break;

    if(objectState.inSameQuadrantAsBoxRelativeToLara(engine, *box))
    {
      creatureInfo.pathFinder.setRandomSearchTarget(box);
    }
    else if(aiInfo.canReachEnemyZone() && objectState.stalkBox(engine, *box))
    {
      creatureInfo.pathFinder.setRandomSearchTarget(box);
      creatureInfo.mood = Mood::Stalk;
    }
    break;
  }
  }

  if(creatureInfo.pathFinder.target_box == nullptr)
  {
    creatureInfo.pathFinder.setRandomSearchTarget(objectState.box);
  }
  creatureInfo.pathFinder.calculateTarget(engine, creatureInfo.target, objectState);
}

std::shared_ptr<CreatureInfo> create(const serialization::TypeId<std::shared_ptr<CreatureInfo>>&,
                                     const serialization::Serializer& ser)
{
  if(!ser.node.has_val())
    return nullptr;

  auto result = std::make_shared<CreatureInfo>(ser.engine, core::TypeId::create(ser["type"]));
  ser(S_NV("data", *result));
  return result;
}

void serialize(std::shared_ptr<CreatureInfo>& data, const serialization::Serializer& ser)
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

AiInfo::AiInfo(Engine& engine, objects::ObjectState& objectState)
{
  if(objectState.creatureInfo == nullptr)
    return;

  const auto zoneRef = loader::file::Box::getZoneRef(
    engine.roomsAreSwapped(), objectState.creatureInfo->pathFinder.fly, objectState.creatureInfo->pathFinder.step);

  objectState.box = objectState.getCurrentSector()->box;
  zone_number = objectState.box->*zoneRef;
  engine.getObjectManager().getLara().m_state.box = engine.getObjectManager().getLara().m_state.getCurrentSector()->box;
  enemy_zone = engine.getObjectManager().getLara().m_state.box->*zoneRef;
  enemy_unreachable = (!objectState.creatureInfo->pathFinder.canVisit(*engine.getObjectManager().getLara().m_state.box)
                       || (!objectState.creatureInfo->pathFinder.nodes[objectState.box].traversable
                           && objectState.creatureInfo->pathFinder.visited.count(objectState.box) != 0));

  auto objectInfo = pybind11::globals()["getObjectInfo"](objectState.type.get()).cast<script::ObjectInfo>();
  const core::Length pivotLength{objectInfo.pivot_length};
  const auto d = engine.getObjectManager().getLara().m_state.position.position
                 - (objectState.position.position + util::pitch(pivotLength, objectState.rotation.Y));
  const auto pivotAngle = core::angleFromAtan(d.X, d.Z);
  distance = util::square(d.X) + util::square(d.Z);
  angle = pivotAngle - objectState.rotation.Y;
  enemy_facing = pivotAngle - 180_deg - engine.getObjectManager().getLara().m_state.rotation.Y;
  ahead = angle > -90_deg && angle < 90_deg;
  if(ahead)
  {
    const auto laraY = engine.getObjectManager().getLara().m_state.position.position.Y;
    if(objectState.position.position.Y - core::QuarterSectorSize < laraY
       && objectState.position.position.Y + core::QuarterSectorSize > laraY)
    {
      bite = true;
    }
  }
}

CreatureInfo::CreatureInfo(const Engine& engine, const core::TypeId type)
    : type{type}
    , pathFinder{engine}
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

void CreatureInfo::serialize(const serialization::Serializer& ser)
{
  ser(S_NV("headRotation", head_rotation),
      S_NV("neckRotation", neck_rotation),
      S_NV("maximumTurn", maximum_turn),
      S_NV("flags", flags),
      S_NV("mood", mood),
      S_NV("pathFinder", pathFinder),
      S_NV("target", target));
}
} // namespace engine::ai
