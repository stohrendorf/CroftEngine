#include "tallblock.h"

#include "core/genericvec.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/vec.h"
#include "engine/location.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/room.h"
#include "modelobject.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/serialization.h" // IWYU pragma: keep

#include <gl/renderstate.h>
#include <gsl-lite/gsl-lite.hpp>
#include <memory>
#include <string>

// IWYU pragma: no_forward_declare serialization::Serializer

namespace engine::objects
{
void TallBlock::updateLogic()
{
  if(m_state.updateActivationTimeout())
  {
    if(m_state.current_anim_state == 0_as)
    {
      world::patchHeightsForBlock(*this, 2_sectors);
      getSkeleton()->resetSmoothing();
      m_state.goal_anim_state = 1_as;
    }
  }
  else
  {
    if(m_state.current_anim_state == 1_as)
    {
      world::patchHeightsForBlock(*this, 2_sectors);
      getSkeleton()->resetSmoothing();
      m_state.goal_anim_state = 0_as;
    }
  }

  advanceFrame();
  m_state.location.updateRoom();
  setCurrentRoom(m_state.location.room);

  if(m_state.triggerState != TriggerState::Deactivated)
  {
    return;
  }

  m_state.triggerState = TriggerState::Active;
  world::patchHeightsForBlock(*this, -2_sectors);
  getSkeleton()->resetSmoothing();
  auto pos = m_state.location.position;
  pos.X = snappedSector(pos.X) + 1_sectors / 2;
  pos.Z = snappedSector(pos.Z) + 1_sectors / 2;
  m_state.location.position = pos;
}

void TallBlock::serialize(const serialization::Serializer<world::World>& ser) const
{
  world::patchHeightsForBlock(*this, 2_sectors);
  ModelObject::serialize(ser);
  world::patchHeightsForBlock(*this, -2_sectors);
}

void TallBlock::deserialize(const serialization::Deserializer<world::World>& ser)
{
  world::patchHeightsForBlock(*this, 2_sectors);
  ModelObject::deserialize(ser);
  world::patchHeightsForBlock(*this, -2_sectors);
  getSkeleton()->resetSmoothing();
  getSkeleton()->getRenderState().setScissorTest(false);
}

TallBlock::TallBlock(const std::string& name,
                     const gsl_lite::not_null<world::World*>& world,
                     const gsl_lite::not_null<const world::Room*>& room,
                     const loader::file::Item& item,
                     const gsl_lite::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel, false}
{
  world::patchHeightsForBlock(*this, -2_sectors);
  getSkeleton()->getRenderState().setScissorTest(false);
  getSkeleton()->resetSmoothing();
}
} // namespace engine::objects