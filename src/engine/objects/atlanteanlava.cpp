#include "atlanteanlava.h"

#include "core/angle.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/cameracontroller.h"
#include "engine/heightinfo.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/world/room.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "objectstate.h"
#include "qs/quantity.h"
#include "render/scene/node.h"

#include <bitset>
#include <gsl/gsl-lite.hpp>

namespace engine::objects
{
void AtlanteanLava::update()
{
  m_state.location.updateRoom();
  setParent(gsl::not_null{getNode()}, m_state.location.room->node);
  if(m_state.triggerState != TriggerState::Deactivated)
  {
    auto location = m_state.location;
    switch(core::axisFromAngle(m_state.rotation.Y))
    {
    case core::Axis::Deg0:
      location.position.Z += 2048_len;
      m_state.location.position.Z += 25_len;
      break;
    case core::Axis::Right90:
      location.position.X += 2048_len;
      m_state.location.position.X += 25_len;
      break;
    case core::Axis::Deg180:
      location.position.Z -= 2048_len;
      m_state.location.position.Z -= 25_len;
      break;
    case core::Axis::Left90:
      location.position.X -= 2048_len;
      m_state.location.position.X -= 25_len;
      break;
    }

    applyTransform();

    const auto sector = location.updateRoom();
    if(HeightInfo::fromFloor(sector, location.position, getWorld().getObjectManager().getObjects()).y
       != m_state.location.position.Y)
    {
      m_state.triggerState = TriggerState::Deactivated;
    }
  }

  if(m_state.touch_bits.any())
  {
    getWorld().getObjectManager().getLara().burnIfAlive();

    auto& cameraController = getWorld().getCameraController();
    cameraController.setLookAtObject(gsl::not_null{getWorld().getObjectManager().find(this)});
    cameraController.setMode(CameraMode::FixedPosition);
    cameraController.setModifier(CameraModifier::Chase);
    cameraController.setDistance(3_sectors);
    cameraController.setRotationAroundLaraY(-180_deg);
  }
}

void AtlanteanLava::collide(CollisionInfo& collisionInfo)
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
} // namespace engine::objects
