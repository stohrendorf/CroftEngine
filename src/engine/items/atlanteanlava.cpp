#include "atlanteanlava.h"

#include "engine/laranode.h"
#include "engine/particle.h"

namespace engine
{
namespace items
{
void AtlanteanLava::update()
{
  loader::file::findRealFloorSector(m_state.position);
  setParent(getNode(), m_state.position.room->node);
  if(m_state.triggerState != TriggerState::Deactivated)
  {
    auto pos = m_state.position.position;
    if(m_state.rotation.Y == 0_deg)
    {
      pos.Z += 2048_len;
      m_state.position.position.Z += 25_len;
    }
    else if(m_state.rotation.Y == -180_deg)
    {
      pos.Z -= 2048_len;
      m_state.position.position.Z -= 25_len;
    }
    else if(m_state.rotation.Y == 90_deg)
    {
      pos.X += 2048_len;
      m_state.position.position.X += 25_len;
    }
    else
    {
      pos.X -= 2048_len;
      m_state.position.position.X -= 25_len;
    }

    const auto sector = loader::file::findRealFloorSector(pos, m_state.position.room);
    if(HeightInfo::fromFloor(sector, pos, getEngine().getItemNodes()).y != m_state.position.position.Y)
    {
      m_state.triggerState = TriggerState::Deactivated;
    }
  }

  if(m_state.touch_bits.any())
  {
    getEngine().getLara().burnIfAlive();

    getEngine().getCameraController().setLookAtItem(getEngine().find(this));
    getEngine().getCameraController().setMode(CameraMode::Fixed);
    getEngine().getCameraController().setModifier(CameraModifier::Chase);
    getEngine().getCameraController().setEyeCenterDistance(3 * core::SectorSize);
    getEngine().getCameraController().setRotationAroundCenterY(-180_deg);
  }
}
} // namespace items
} // namespace engine
