#pragma once

#include "core/angle.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/location.h"
#include "modelobject.h"
#include "objectstate.h"
#include "qs/qs.h"

#include <boost/assert.hpp>
#include <optional>

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
class TrapDoorUp final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(TrapDoorUp, true)

  void update() override;

  void patchFloor(const core::TRVec& pos, core::Length& y) override
  {
    if(m_state.current_anim_state != 1_as || !possiblyOnTrapdoor(pos) || pos.Y > m_state.location.position.Y)
      return;

    y = m_state.location.position.Y;
  }

  void patchCeiling(const core::TRVec& pos, core::Length& y) override
  {
    if(m_state.current_anim_state != 1_as || !possiblyOnTrapdoor(pos) || pos.Y <= m_state.location.position.Y)
      return;

    y = m_state.location.position.Y + core::QuarterSectorSize;
  }

private:
  bool possiblyOnTrapdoor(const core::TRVec& pos) const
  {
    const auto trapdoorSectorX = m_state.location.position.X / core::SectorSize;
    const auto trapdoorSectorZ = m_state.location.position.Z / core::SectorSize;
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
