#pragma once

#include "modelobject.h"

namespace engine::objects
{
class BridgeFlat final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(BridgeFlat, false)

  void patchFloor(const core::TRVec& pos, core::Length& y) override
  {
    if(!core::isSameSector(pos, m_state.location.position))
      return;

    if(pos.Y > m_state.location.position.Y)
      return;
    
    y = m_state.location.position.Y;
  }

  void patchCeiling(const core::TRVec& pos, core::Length& y) override
  {
    if(!core::isSameSector(pos, m_state.location.position))
      return;

    if(pos.Y <= m_state.location.position.Y)
      return;

    y = m_state.location.position.Y + core::QuarterSectorSize;
  }
};
} // namespace engine::objects
