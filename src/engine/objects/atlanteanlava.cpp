#include "atlanteanlava.h"

#include "engine/particle.h"
#include "engine/world/world.h"
#include "laraobject.h"

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

    getWorld().getCameraController().setLookAtObject(getWorld().getObjectManager().find(this));
    getWorld().getCameraController().setMode(CameraMode::FixedPosition);
    getWorld().getCameraController().setModifier(CameraModifier::Chase);
    getWorld().getCameraController().setDistance(3 * core::SectorSize);
    getWorld().getCameraController().setRotationAroundLaraY(-180_deg);
  }
}
} // namespace engine::objects
