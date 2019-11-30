#include "objectstate.h"

#include "engine/engine.h"
#include "engine/script/reflection.h"
#include "laraobject.h"
#include "serialization/animation_ptr.h"
#include "serialization/bitset.h"
#include "serialization/box_ptr.h"

namespace engine::objects
{
ObjectState::~ObjectState() = default;

bool ObjectState::stalkBox(const Engine& engine, const loader::file::Box& targetBox) const
{
  const auto laraToBoxDistX = (targetBox.xmin + targetBox.xmax) / 2 - engine.getLara().m_state.position.position.X;
  const auto laraToBoxDistZ = (targetBox.zmin + targetBox.zmax) / 2 - engine.getLara().m_state.position.position.Z;

  if(abs(laraToBoxDistX) > 3 * core::SectorSize || abs(laraToBoxDistZ) > 3 * core::SectorSize)
  {
    return false;
  }

  const auto laraAxisBack = *axisFromAngle(engine.getLara().m_state.rotation.Y + 180_deg, 45_deg);
  core::Axis laraToBoxAxis;
  if(laraToBoxDistZ > 0_len)
  {
    if(laraToBoxDistX > 0_len)
    {
      laraToBoxAxis = core::Axis::PosX;
    }
    else
    {
      laraToBoxAxis = core::Axis::NegZ;
    }
  }
  else if(laraToBoxDistX > 0_len)
  {
    // Z <= 0, X > 0
    laraToBoxAxis = core::Axis::NegX;
  }
  else
  {
    // Z <= 0, X <= 0
    laraToBoxAxis = core::Axis::PosZ;
  }

  if(laraAxisBack == laraToBoxAxis)
  {
    return false;
  }

  core::Axis objectToLaraAxis;
  if(position.position.Z <= engine.getLara().m_state.position.position.Z)
  {
    if(position.position.X <= engine.getLara().m_state.position.position.X)
    {
      objectToLaraAxis = core::Axis::PosZ;
    }
    else
    {
      objectToLaraAxis = core::Axis::NegX;
    }
  }
  else
  {
    if(position.position.X > engine.getLara().m_state.position.position.X)
    {
      objectToLaraAxis = core::Axis::PosX;
    }
    else
    {
      objectToLaraAxis = core::Axis::NegZ;
    }
  }

  if(laraAxisBack != objectToLaraAxis)
  {
    return true;
  }

  switch(laraAxisBack)
  {
  case core::Axis::PosZ: return laraToBoxAxis == core::Axis::NegZ;
  case core::Axis::PosX: return laraToBoxAxis == core::Axis::NegX;
  case core::Axis::NegZ: return laraToBoxAxis == core::Axis::PosZ;
  case core::Axis::NegX: return laraToBoxAxis == core::Axis::PosX;
  }

  BOOST_THROW_EXCEPTION(std::runtime_error("Unreachable code reached"));
}

bool ObjectState::isInsideZoneButNotInBox(const Engine& engine,
                                          const int16_t zoneId,
                                          const loader::file::Box& targetBox) const
{
  Expects(creatureInfo != nullptr);

  const auto zoneRef = loader::file::Box::getZoneRef(
    engine.roomsAreSwapped(), creatureInfo->pathFinder.fly, creatureInfo->pathFinder.step);

  if(zoneId != targetBox.*zoneRef)
  {
    return false;
  }

  if(!creatureInfo->pathFinder.canVisit(targetBox))
  {
    return false;
  }

  return position.position.Z <= targetBox.zmin || position.position.Z >= targetBox.zmax
         || position.position.X <= targetBox.xmin || position.position.X >= targetBox.xmax;
}

bool ObjectState::inSameQuadrantAsBoxRelativeToLara(const Engine& engine, const loader::file::Box& targetBox) const
{
  const auto laraToBoxX = (targetBox.xmin + targetBox.xmax) / 2 - engine.getLara().m_state.position.position.X;
  const auto laraToBoxZ = (targetBox.zmin + targetBox.zmax) / 2 - engine.getLara().m_state.position.position.Z;
  if(abs(laraToBoxX) < 5 * core::SectorSize && abs(laraToBoxZ) < 5 * core::SectorSize)
    return false;

  const auto laraToNpcX = position.position.X - engine.getLara().m_state.position.position.X;
  const auto laraToNpcZ = position.position.Z - engine.getLara().m_state.position.position.Z;
  return ((laraToNpcZ > 0_len) == (laraToBoxZ > 0_len)) || ((laraToNpcX > 0_len) == (laraToBoxX > 0_len));
}

void ObjectState::initCreatureInfo(const Engine& engine)
{
  if(creatureInfo != nullptr)
    return;

  creatureInfo = std::make_unique<ai::CreatureInfo>(engine, type);
  collectZoneBoxes(engine);
}

void ObjectState::collectZoneBoxes(const Engine& engine)
{
  const auto zoneRef1
    = loader::file::Box::getZoneRef(false, creatureInfo->pathFinder.fly, creatureInfo->pathFinder.step);
  const auto zoneRef2
    = loader::file::Box::getZoneRef(true, creatureInfo->pathFinder.fly, creatureInfo->pathFinder.step);

  box = position.room->getInnerSectorByAbsolutePosition(position.position)->box;
  const auto zoneData1 = box->*zoneRef1;
  const auto zoneData2 = box->*zoneRef2;
  creatureInfo->pathFinder.boxes.clear();
  for(const auto& levelBox : engine.getBoxes())
  {
    if(levelBox.*zoneRef1 == zoneData1 || levelBox.*zoneRef2 == zoneData2)
    {
      creatureInfo->pathFinder.boxes.emplace_back(&levelBox);
    }
  }
}

glm::vec3 ObjectState::getPosition() const
{
  return position.position.toRenderSystem();
}

void ObjectState::loadObjectInfo(const sol::state& engine)
{
  health = core::Health{engine["getObjectInfo"].call<script::ObjectInfo>(type.get()).hit_points};
}

void ObjectState::serialize(const serialization::Serializer& ser)
{
  ser(S_NVP(position),
      S_NVP(type),
      S_NVP(rotation),
      S_NVP(speed),
      S_NVP(fallspeed),
      S_NVP(current_anim_state),
      S_NVP(goal_anim_state),
      S_NVP(required_anim_state),
      S_NVP(anim),
      S_NVP(frame_number),
      S_NVP(health),
      S_NVP(triggerState),
      S_NVP(timer),
      S_NVP(activationState),
      S_NVP(floor),
      S_NVP(touch_bits),
      S_NVP(box),
      S_NVP(shade),
      S_NVP(falling),
      S_NVP(is_hit),
      S_NVP(collidable),
      S_NVP(already_looked_at),
      S_NVP(creatureInfo));
}
} // namespace engine::objects
