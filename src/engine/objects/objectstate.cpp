#include "objectstate.h"

#include "engine/script/reflection.h"
#include "laraobject.h"
#include "serialization/animation_ptr.h"
#include "serialization/bitset.h"
#include "serialization/box_ptr.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"

#include <pybind11/pybind11.h>

namespace engine::objects
{
ObjectState::~ObjectState() = default;

bool ObjectState::isStalkBox(const World& world, const loader::file::Box& targetBox) const
{
  const auto laraPos = world.getObjectManager().getLara().m_state.position.position;

  const auto laraToBoxDistX = (targetBox.xmin + targetBox.xmax) / 2 - laraPos.X;
  const auto laraToBoxDistZ = (targetBox.zmin + targetBox.zmax) / 2 - laraPos.Z;

  if(abs(laraToBoxDistX) > 3 * core::SectorSize || abs(laraToBoxDistZ) > 3 * core::SectorSize)
  {
    return false;
  }

  const auto laraAxis = *axisFromAngle(world.getObjectManager().getLara().m_state.rotation.Y, 45_deg);
  const auto laraToBoxAxis = *axisFromAngle(angleFromAtan(laraToBoxDistX, laraToBoxDistZ), 45_deg);
  if(laraAxis == laraToBoxAxis)
  {
    return false;
  }

  const auto objectToLaraAxis
    = *axisFromAngle(angleFromAtan(laraPos.X - position.position.X, laraPos.Z - position.position.Z), 45_deg);
  if(laraAxis != objectToLaraAxis)
  {
    return true;
  }

  switch(laraAxis)
  {
  case core::Axis::PosZ: return laraToBoxAxis == core::Axis::NegZ;
  case core::Axis::PosX: return laraToBoxAxis == core::Axis::NegX;
  case core::Axis::NegZ: return laraToBoxAxis == core::Axis::PosZ;
  case core::Axis::NegX: return laraToBoxAxis == core::Axis::PosX;
  }

  BOOST_THROW_EXCEPTION(std::runtime_error("Unreachable code reached"));
}

bool ObjectState::isInsideZoneButNotInBox(const World& world,
                                          const int16_t zoneId,
                                          const loader::file::Box& targetBox) const
{
  Expects(creatureInfo != nullptr);

  const auto zoneRef = loader::file::Box::getZoneRef(
    world.roomsAreSwapped(), creatureInfo->pathFinder.fly, creatureInfo->pathFinder.step);

  if(zoneId != targetBox.*zoneRef)
  {
    return false;
  }

  if(!creatureInfo->pathFinder.canVisit(targetBox))
  {
    return false;
  }

  return !targetBox.contains(position.position.X, position.position.Z);
}

bool ObjectState::isEscapeBox(const World& world, const loader::file::Box& targetBox) const
{
  const auto laraPos = world.getObjectManager().getLara().m_state.position.position;

  const auto laraToBoxCtrX = (targetBox.xmin + targetBox.xmax) / 2 - laraPos.X;
  const auto laraToBoxCtrZ = (targetBox.zmin + targetBox.zmax) / 2 - laraPos.Z;
  if(abs(laraToBoxCtrX) < 5 * core::SectorSize && abs(laraToBoxCtrZ) < 5 * core::SectorSize)
    return false;

  const auto laraToObjX = position.position.X - laraPos.X;
  const auto laraToObjZ = position.position.Z - laraPos.Z;
  return ((laraToObjZ > 0_len) == (laraToBoxCtrZ > 0_len)) || ((laraToObjX > 0_len) == (laraToBoxCtrX > 0_len));
}

void ObjectState::initCreatureInfo(const World& world)
{
  if(creatureInfo != nullptr)
    return;

  creatureInfo = std::make_unique<ai::CreatureInfo>(world, type);
  collectZoneBoxes(world);
}

void ObjectState::collectZoneBoxes(const World& world)
{
  const auto zoneRef1
    = loader::file::Box::getZoneRef(false, creatureInfo->pathFinder.fly, creatureInfo->pathFinder.step);
  const auto zoneRef2
    = loader::file::Box::getZoneRef(true, creatureInfo->pathFinder.fly, creatureInfo->pathFinder.step);

  box = position.room->getInnerSectorByAbsolutePosition(position.position)->box;
  Ensures(box != nullptr);
  const auto zoneData1 = box->*zoneRef1;
  const auto zoneData2 = box->*zoneRef2;
  creatureInfo->pathFinder.boxes.clear();
  for(const auto& levelBox : world.getBoxes())
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

void ObjectState::loadObjectInfo()
{
  pybind11::object getObjectInfo = pybind11::globals()["getObjectInfo"];
  health = core::Health{getObjectInfo(type.get()).cast<script::ObjectInfo>().hit_points};
}

void ObjectState::serialize(const serialization::Serializer<World>& ser)
{
  ser(S_NV("position", position),
      S_NV("type", type),
      S_NV("rotation", rotation),
      S_NV("speed", speed),
      S_NV("fallspeed", fallspeed),
      S_NV("currentAnimState", current_anim_state),
      S_NV("goalAnimState", goal_anim_state),
      S_NV("requiredAnimState", required_anim_state),
      S_NV("health", health),
      S_NV("triggerState", triggerState),
      S_NV("timer", timer),
      S_NV("activationState", activationState),
      S_NV("floor", floor),
      S_NV("touchBits", touch_bits),
      S_NV("box", box),
      S_NV("shade", shade),
      S_NV("falling", falling),
      S_NV("isHit", is_hit),
      S_NV("collidable", collidable),
      S_NV("alreadyLookedAt", already_looked_at),
      S_NV("creatureInfo", creatureInfo));
}
} // namespace engine::objects
