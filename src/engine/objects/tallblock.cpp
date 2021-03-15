#include "tallblock.h"

namespace engine::objects
{
void TallBlock::update()
{
  if(m_state.updateActivationTimeout())
  {
    if(m_state.current_anim_state == 0_as)
    {
      world::Room::patchHeightsForBlock(*this, 2 * core::SectorSize);
      m_state.goal_anim_state = 1_as;
    }
  }
  else
  {
    if(m_state.current_anim_state == 1_as)
    {
      world::Room::patchHeightsForBlock(*this, 2 * core::SectorSize);
      m_state.goal_anim_state = 0_as;
    }
  }

  ModelObject::update();
  auto room = m_state.position.room;
  findRealFloorSector(m_state.position.position, &room);
  setCurrentRoom(room);

  if(m_state.triggerState != TriggerState::Deactivated)
  {
    return;
  }

  m_state.triggerState = TriggerState::Active;
  world::Room::patchHeightsForBlock(*this, -2 * core::SectorSize);
  auto pos = m_state.position.position;
  pos.X = (pos.X / core::SectorSize) * core::SectorSize + core::SectorSize / 2;
  pos.Z = (pos.Z / core::SectorSize) * core::SectorSize + core::SectorSize / 2;
  m_state.position.position = pos;
}

void TallBlock::serialize(const serialization::Serializer<world::World>& ser)
{
  world::Room::patchHeightsForBlock(*this, 2 * core::SectorSize);
  ModelObject::serialize(ser);
  world::Room::patchHeightsForBlock(*this, -2 * core::SectorSize);
}
} // namespace engine::objects
