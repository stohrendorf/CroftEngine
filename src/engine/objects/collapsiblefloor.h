#pragma once

#include "modelobject.h"

namespace engine::objects
{
class CollapsibleFloor final : public ModelObject
{
public:
  CollapsibleFloor(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  CollapsibleFloor(const gsl::not_null<world::World*>& world,
                   const gsl::not_null<const loader::file::Room*>& room,
                   const loader::file::Item& item,
                   const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
  }

  void update() override;

  void patchFloor(const core::TRVec& pos, core::Length& y) override
  {
    if(pos.Y > m_state.position.position.Y - 512_len)
      return;

    if(m_state.current_anim_state != 0_as && m_state.current_anim_state != 1_as)
      return;

    y = m_state.position.position.Y - 512_len;
  }

  void patchCeiling(const core::TRVec& pos, core::Length& y) override
  {
    if(pos.Y <= m_state.position.position.Y - 512_len)
      return;

    if(m_state.current_anim_state != 0_as && m_state.current_anim_state != 1_as)
      return;

    y = m_state.position.position.Y - 256_len;
  }
};
} // namespace engine::objects
