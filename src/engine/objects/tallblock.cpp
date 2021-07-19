#include "tallblock.h"

namespace engine::objects
{
void TallBlock::update()
{
  if(m_state.updateActivationTimeout())
  {
    if(m_state.current_anim_state == 0_as)
    {
      world::patchHeightsForBlock(*this, 2 * core::SectorSize);
      m_state.goal_anim_state = 1_as;
    }
  }
  else
  {
    if(m_state.current_anim_state == 1_as)
    {
      world::patchHeightsForBlock(*this, 2 * core::SectorSize);
      m_state.goal_anim_state = 0_as;
    }
  }

  ModelObject::update();
  m_state.location.updateRoom();
  setCurrentRoom(m_state.location.room);

  if(m_state.triggerState != TriggerState::Deactivated)
  {
    return;
  }

  m_state.triggerState = TriggerState::Active;
  world::patchHeightsForBlock(*this, -2 * core::SectorSize);
  auto pos = m_state.location.position;
  pos.X = (pos.X / core::SectorSize) * core::SectorSize + core::SectorSize / 2;
  pos.Z = (pos.Z / core::SectorSize) * core::SectorSize + core::SectorSize / 2;
  m_state.location.position = pos;
}

void TallBlock::serialize(const serialization::Serializer<world::World>& ser)
{
  world::patchHeightsForBlock(*this, 2 * core::SectorSize);
  ModelObject::serialize(ser);
  world::patchHeightsForBlock(*this, -2 * core::SectorSize);
}
} // namespace engine::objects
