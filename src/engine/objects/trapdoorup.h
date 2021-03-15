#pragma once

#include "modelobject.h"

namespace engine::objects
{
class TrapDoorUp final : public ModelObject
{
public:
  TrapDoorUp(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  TrapDoorUp(const gsl::not_null<world::World*>& world,
             const gsl::not_null<const world::Room*>& room,
             const loader::file::Item& item,
             const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
  }

  void update() override;

  void patchFloor(const core::TRVec& pos, core::Length& y) override
  {
    if(m_state.current_anim_state != 1_as || !possiblyOnTrapdoor(pos) || pos.Y > m_state.position.position.Y)
      return;

    y = m_state.position.position.Y;
  }

  void patchCeiling(const core::TRVec& pos, core::Length& y) override
  {
    if(m_state.current_anim_state != 1_as || !possiblyOnTrapdoor(pos) || pos.Y <= m_state.position.position.Y)
      return;

    y = m_state.position.position.Y + core::QuarterSectorSize;
  }

private:
  bool possiblyOnTrapdoor(const core::TRVec& pos) const
  {
    const auto trapdoorSectorX = m_state.position.position.X / core::SectorSize;
    const auto trapdoorSectorZ = m_state.position.position.Z / core::SectorSize;
    const auto posSectorX = pos.X / core::SectorSize;
    const auto posSectorZ = pos.Z / core::SectorSize;
    auto trapdoorAxis = axisFromAngle(m_state.rotation.Y, 1_au);
    BOOST_ASSERT(trapdoorAxis.has_value());

    if(*trapdoorAxis == core::Axis::PosZ && trapdoorSectorX == posSectorX
       && (trapdoorSectorZ - 1 == posSectorZ || trapdoorSectorZ - 2 == posSectorZ))
      return true;
    if(*trapdoorAxis == core::Axis::NegZ && trapdoorSectorX == posSectorX
       && (trapdoorSectorZ + 1 == posSectorZ || trapdoorSectorZ + 2 == posSectorZ))
      return true;
    if(*trapdoorAxis == core::Axis::PosX && trapdoorSectorZ == posSectorZ
       && (trapdoorSectorX - 1 == posSectorX || trapdoorSectorX - 2 == posSectorX))
      return true;
    if(*trapdoorAxis == core::Axis::NegX && trapdoorSectorZ == posSectorZ
       && (trapdoorSectorX + 1 == posSectorX || trapdoorSectorX + 2 == posSectorX))
      return true;

    return false;
  }
};
} // namespace engine::objects
