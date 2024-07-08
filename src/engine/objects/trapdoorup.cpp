#include "trapdoorup.h"

#include "core/angle.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/location.h"
#include "engine/skeletalmodelnode.h"
#include "modelobject.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/serialization.h" // IWYU pragma: keep

#include <boost/assert.hpp>
#include <gl/renderstate.h>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <optional>
#include <string>

namespace engine::objects
{
void TrapDoorUp::update()
{
  if(m_state.updateActivationTimeout())
  {
    m_state.goal_anim_state = 1_as;
  }
  else
  {
    m_state.goal_anim_state = 0_as;
  }

  ModelObject::update();
  m_state.location.updateRoom();
  setCurrentRoom(m_state.location.room);
}

TrapDoorUp::TrapDoorUp(const std::string& name,
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

void TrapDoorUp::serialize(const serialization::Serializer<world::World>& ser) const
{
  ModelObject::serialize(ser);
}

void TrapDoorUp::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ModelObject::deserialize(ser);
  getSkeleton()->getRenderState().setScissorTest(false);
  getSkeleton()->getRenderState().setPolygonOffsetFill(true);
  getSkeleton()->getRenderState().setPolygonOffset(-1, -1);
}

void TrapDoorUp::patchFloor(const core::TRVec& pos, core::Length& y)
{
  if(m_state.current_anim_state != 1_as || !possiblyOnTrapdoor(pos) || pos.Y > m_state.location.position.Y)
    return;

  y = m_state.location.position.Y;
}

void TrapDoorUp::patchCeiling(const core::TRVec& pos, core::Length& y)
{
  if(m_state.current_anim_state != 1_as || !possiblyOnTrapdoor(pos) || pos.Y <= m_state.location.position.Y)
    return;

  y = m_state.location.position.Y + core::QuarterSectorSize;
}

bool TrapDoorUp::possiblyOnTrapdoor(const core::TRVec& pos) const
{
  const auto trapdoorSectorX = sectorOf(m_state.location.position.X);
  const auto trapdoorSectorZ = sectorOf(m_state.location.position.Z);
  const auto posSectorX = sectorOf(pos.X);
  const auto posSectorZ = sectorOf(pos.Z);
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
} // namespace engine::objects
