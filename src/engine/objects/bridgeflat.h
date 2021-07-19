#pragma once

#include "modelobject.h"

namespace engine::objects
{
class BridgeFlat final : public ModelObject
{
public:
  BridgeFlat(const gsl::not_null<world::World*>& world, const RoomBoundPosition& location)
      : ModelObject{world, location}
  {
  }

  BridgeFlat(const gsl::not_null<world::World*>& world,
             const gsl::not_null<const world::Room*>& room,
             const loader::file::Item& item,
             const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, false, animatedModel}
  {
  }

  void patchFloor(const core::TRVec& pos, core::Length& y) override
  {
    if(pos.Y <= m_state.location.position.Y)
      y = m_state.location.position.Y;
  }

  void patchCeiling(const core::TRVec& pos, core::Length& y) override
  {
    if(pos.Y <= m_state.location.position.Y)
      return;

    y = m_state.location.position.Y + core::QuarterSectorSize;
  }
};
} // namespace engine::objects
