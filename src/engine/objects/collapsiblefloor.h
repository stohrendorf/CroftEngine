#pragma once

#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/location.h"
#include "modelobject.h"
#include "objectstate.h"
#include "qs/qs.h"

namespace engine::world
{
class World;
struct Room;
struct SkeletalModelType;
} // namespace engine::world

namespace loader::file
{
struct Item;
}

namespace engine::objects
{
class CollapsibleFloor final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(CollapsibleFloor, true)

  void update() override;

  void patchFloor(const core::TRVec& pos, core::Length& y) override
  {
    if(pos.Y > m_state.location.position.Y - 512_len)
      return;

    if(m_state.current_anim_state != 0_as && m_state.current_anim_state != 1_as)
      return;

    y = m_state.location.position.Y - 512_len;
  }

  void patchCeiling(const core::TRVec& pos, core::Length& y) override
  {
    if(pos.Y <= m_state.location.position.Y - 512_len)
      return;

    if(m_state.current_anim_state != 0_as && m_state.current_anim_state != 1_as)
      return;

    y = m_state.location.position.Y - 256_len;
  }
};
} // namespace engine::objects
