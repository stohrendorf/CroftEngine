#pragma once

#include "modelobject.h"

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
