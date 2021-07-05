#include "objectstate.h"

#include "engine/script/reflection.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "serialization/bitset.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "serialization/vector_element.h"

#include <pybind11/pybind11.h>

namespace engine::objects
{
ObjectState::~ObjectState() = default;

bool ObjectState::isStalkBox(const world::World& world, const world::Box& targetBox) const
{
  const auto laraPos = world.getObjectManager().getLara().m_state.position.position;

  const auto laraToBoxDistX = (targetBox.xmin + targetBox.xmax) / 2 - laraPos.X;
  const auto laraToBoxDistZ = (targetBox.zmin + targetBox.zmax) / 2 - laraPos.Z;

  if(abs(laraToBoxDistX) > 3 * core::SectorSize || abs(laraToBoxDistZ) > 3 * core::SectorSize)
  {
    return false;
  }

  const auto laraAxis = axisFromAngle(world.getObjectManager().getLara().m_state.rotation.Y);
  const auto laraToBoxAxis = axisFromAngle(angleFromAtan(laraToBoxDistX, laraToBoxDistZ));
  if(laraAxis == laraToBoxAxis)
  {
    return false;
  }

  const auto objectToLaraAxis
    = axisFromAngle(angleFromAtan(laraPos.X - position.position.X, laraPos.Z - position.position.Z));
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

bool ObjectState::isInsideZoneButNotInBox(const world::World& world,
                                          const world::ZoneId zoneId,
                                          const world::Box& targetBox) const
{
  Expects(creatureInfo != nullptr);

  const auto zoneRef = world::Box::getZoneRef(
    world.roomsAreSwapped(), creatureInfo->pathFinder.isFlying(), creatureInfo->pathFinder.step);

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

bool ObjectState::isEscapeBox(const world::World& world, const world::Box& targetBox) const
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

void ObjectState::initCreatureInfo(const world::World& world)
{
  if(creatureInfo != nullptr)
    return;

  box = position.room->getInnerSectorByAbsolutePosition(position.position)->box;
  Ensures(box != nullptr);
  creatureInfo = std::make_unique<ai::CreatureInfo>(world, type, box);
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

void ObjectState::serialize(const serialization::Serializer<world::World>& ser)
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
      S_NV_VECTOR_ELEMENT("box", ser.context.getBoxes(), box),
      S_NV("falling", falling),
      S_NV("isHit", is_hit),
      S_NV("collidable", collidable),
      S_NV("alreadyLookedAt", already_looked_at),
      S_NV("creatureInfo", creatureInfo));
}

const world::Sector* ObjectState::getCurrentSector() const
{
  return position.room->getSectorByAbsolutePosition(position.position);
}
} // namespace engine::objects
