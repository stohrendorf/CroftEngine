#include "objectstate.h"

#include "core/interval.h"
#include "core/magic.h"
#include "core/vec.h"
#include "engine/objectmanager.h"
#include "engine/script/reflection.h"
#include "engine/script/scriptengine.h"
#include "engine/world/box.h"
#include "engine/world/room.h"
#include "engine/world/sector.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "qs/qs.h"
#include "serialization/bitset.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"

#include <boost/throw_exception.hpp>
#include <exception>
#include <stdexcept>

namespace engine::objects
{
ObjectState::~ObjectState() = default;

bool ObjectState::isStalkBox(const world::World& world, const world::Box& targetBox) const
{
  const auto laraPos = world.getObjectManager().getLara().m_state.location.position;

  const auto laraToBoxDistX = targetBox.xInterval.mid() - laraPos.X;
  const auto laraToBoxDistZ = targetBox.zInterval.mid() - laraPos.Z;

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
    = axisFromAngle(angleFromAtan(laraPos.X - location.position.X, laraPos.Z - location.position.Z));
  if(laraAxis != objectToLaraAxis)
  {
    return true;
  }

  switch(laraAxis)
  {
  case core::Axis::PosZ:
    return laraToBoxAxis == core::Axis::NegZ;
  case core::Axis::PosX:
    return laraToBoxAxis == core::Axis::NegX;
  case core::Axis::NegZ:
    return laraToBoxAxis == core::Axis::PosZ;
  case core::Axis::NegX:
    return laraToBoxAxis == core::Axis::PosX;
  }

  BOOST_THROW_EXCEPTION(std::runtime_error("Unreachable code reached"));
}

bool ObjectState::isEscapeBox(const world::World& world, const world::Box& targetBox) const
{
  const auto laraPos = world.getObjectManager().getLara().m_state.location.position;

  const auto laraToBoxCtrX = targetBox.xInterval.mid() - laraPos.X;
  const auto laraToBoxCtrZ = targetBox.zInterval.mid() - laraPos.Z;
  if(abs(laraToBoxCtrX) < 5 * core::SectorSize && abs(laraToBoxCtrZ) < 5 * core::SectorSize)
    return false;

  const auto laraToObjX = location.position.X - laraPos.X;
  const auto laraToObjZ = location.position.Z - laraPos.Z;
  return ((laraToObjZ > 0_len) == (laraToBoxCtrZ > 0_len)) || ((laraToObjX > 0_len) == (laraToBoxCtrX > 0_len));
}

glm::vec3 ObjectState::getPosition() const
{
  return location.position.toRenderSystem();
}

void ObjectState::loadObjectInfo(const script::ScriptEngine& scriptEngine)
{
  health = core::Health{scriptEngine.getObjectInfo(type).hit_points};
}

void ObjectState::serialize(const serialization::Serializer<world::World>& ser)
{
  ser(S_NV("location", location),
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
      S_NV("falling", falling),
      S_NV("isHit", is_hit),
      S_NV("collidable", collidable),
      S_NV("alreadyLookedAt", already_looked_at));
}

const world::Sector* ObjectState::getCurrentSector() const
{
  return location.room->getSectorByAbsolutePosition(location.position);
}

gsl::not_null<const world::Box*> ObjectState::getCurrentBox() const
{
  gsl::not_null sector{getCurrentSector()};
  return gsl::not_null{sector->box};
}
} // namespace engine::objects
