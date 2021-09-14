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
#include <memory>

namespace engine::objects
{
void AtlanteanLava::update()
{
  m_state.location.updateRoom();
  setParent(getNode(), m_state.location.room->node);
  if(m_state.triggerState != TriggerState::Deactivated)
  {
    auto location = m_state.location;
    if(m_state.rotation.Y == 0_deg)
    {
      location.position.Z += 2048_len;
      m_state.location.position.Z += 25_len;
    }
    else if(m_state.rotation.Y == -180_deg)
    {
      location.position.Z -= 2048_len;
      m_state.location.position.Z -= 25_len;
    }
    else if(m_state.rotation.Y == 90_deg)
    {
      location.position.X += 2048_len;
      m_state.location.position.X += 25_len;
    }
    else
    {
      location.position.X -= 2048_len;
      m_state.location.position.X -= 25_len;
    }

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
    cameraController.setLookAtObject(getWorld().getObjectManager().find(this));
    cameraController.setMode(CameraMode::FixedPosition);
    cameraController.setModifier(CameraModifier::Chase);
    cameraController.setDistance(3 * core::SectorSize);
    cameraController.setRotationAroundLaraY(-180_deg);
  }
}
} // namespace engine::objects
