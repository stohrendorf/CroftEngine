#include "trapdoordown.h"

#include "core/angle.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/vec.h"
#include "engine/location.h"
#include "engine/skeletalmodelnode.h"
#include "modelobject.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/serialization.h" // IWYU pragma: keep

#include <boost/assert.hpp>
#include <gl/renderstate.h>
#include <memory>
#include <optional>

namespace engine::objects
{
TrapDoorDown::TrapDoorDown(const std::string& name,
                           const gsl::not_null<world::World*>& world,
                           const gsl::not_null<const world::Room*>& room,
                           const loader::file::Item& item,
                           const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel, false}
{
  getSkeleton()->getRenderState().setScissorTest(false);
  getSkeleton()->getRenderState().setPolygonOffsetFill(true);
  getSkeleton()->getRenderState().setPolygonOffset(-1, -1);
}

void TrapDoorDown::serialize(const serialization::Serializer<world::World>& ser)
{
  ModelObject::serialize(ser);
  if(ser.loading)
  {
    getSkeleton()->getRenderState().setScissorTest(false);
    getSkeleton()->getRenderState().setPolygonOffsetFill(true);
    getSkeleton()->getRenderState().setPolygonOffset(-1, -1);
  }
}

void TrapDoorDown::update()
{
  if(m_state.updateActivationTimeout())
  {
    if(m_state.current_anim_state == 0_as)
      m_state.goal_anim_state = 1_as;
  }
  else
  {
    if(m_state.current_anim_state == 1_as)
      m_state.goal_anim_state = 0_as;
  }

  ModelObject::update();
}

void TrapDoorDown::patchFloor(const core::TRVec& pos, core::Length& y)
{
  if(m_state.current_anim_state != 0_as || !possiblyOnTrapdoor(pos) || pos.Y > m_state.location.position.Y
     || y <= m_state.location.position.Y)
    return;

  y = m_state.location.position.Y;
}

void TrapDoorDown::patchCeiling(const core::TRVec& pos, core::Length& y)
{
  if(m_state.current_anim_state != 0_as || !possiblyOnTrapdoor(pos) || pos.Y <= m_state.location.position.Y
     || y > m_state.location.position.Y)
    return;

  y = m_state.location.position.Y + core::QuarterSectorSize;
}

bool TrapDoorDown::possiblyOnTrapdoor(const core::TRVec& pos) const
{
  const auto trapdoorSectorX = sectorOf(m_state.location.position.X);
  const auto trapdoorSectorZ = sectorOf(m_state.location.position.Z);
  const auto posSectorX = sectorOf(pos.X);
  const auto posSectorZ = sectorOf(pos.Z);
  auto trapdoorAxis = axisFromAngle(m_state.rotation.Y, 1_au);
  BOOST_ASSERT(trapdoorAxis.has_value());

  if(*trapdoorAxis == core::Axis::PosZ && trapdoorSectorX == posSectorX
     && (trapdoorSectorZ + 1 == posSectorZ || trapdoorSectorZ == posSectorZ))
    return true;
  if(*trapdoorAxis == core::Axis::NegZ && trapdoorSectorX == posSectorX
     && (trapdoorSectorZ - 1 == posSectorZ || trapdoorSectorZ == posSectorZ))
    return true;
  if(*trapdoorAxis == core::Axis::PosX && trapdoorSectorZ == posSectorZ
     && (trapdoorSectorX + 1 == posSectorX || trapdoorSectorX == posSectorX))
    return true;
  if(*trapdoorAxis == core::Axis::NegX && trapdoorSectorZ == posSectorZ
     && (trapdoorSectorX - 1 == posSectorX || trapdoorSectorX == posSectorX))
    return true;

  return false;
}
} // namespace engine::objects
