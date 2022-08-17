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
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <string>

// IWYU pragma: no_forward_declare serialization::Serializer

namespace engine::objects
{
void TallBlock::update()
{
  if(m_state.updateActivationTimeout())
  {
    if(m_state.current_anim_state == 0_as)
    {
      world::patchHeightsForBlock(*this, 2_sectors);
      getSkeleton()->resetInterpolation();
      m_state.goal_anim_state = 1_as;
    }
  }
  else
  {
    if(m_state.current_anim_state == 1_as)
    {
      world::patchHeightsForBlock(*this, 2_sectors);
      getSkeleton()->resetInterpolation();
      m_state.goal_anim_state = 0_as;
    }
  }

  ModelObject::update();
  m_state.location.updateRoom();
  setCurrentRoom(m_state.location.room);

  if(m_state.triggerState != TriggerState::Deactivated)
  {
    return;
  }

  m_state.triggerState = TriggerState::Active;
  world::patchHeightsForBlock(*this, -2_sectors);
  getSkeleton()->resetInterpolation();
  auto pos = m_state.location.position;
  pos.X = sectorOf(pos.X) * 1_sectors + 1_sectors / 2;
  pos.Z = sectorOf(pos.Z) * 1_sectors + 1_sectors / 2;
  m_state.location.position = pos;
}

void TallBlock::serialize(const serialization::Serializer<world::World>& ser)
{
  world::patchHeightsForBlock(*this, 2_sectors);
  ModelObject::serialize(ser);
  world::patchHeightsForBlock(*this, -2_sectors);
  if(ser.loading)
  {
    getSkeleton()->resetInterpolation();
    getSkeleton()->getRenderState().setScissorTest(false);
  }
}

TallBlock::TallBlock(const std::string& name,
                     const gsl::not_null<world::World*>& world,
                     const gsl::not_null<const world::Room*>& room,
                     const loader::file::Item& item,
                     const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel, false}
{
  world::patchHeightsForBlock(*this, -2_sectors);
  getSkeleton()->getRenderState().setScissorTest(false);
  getSkeleton()->resetInterpolation();
}
} // namespace engine::objects
