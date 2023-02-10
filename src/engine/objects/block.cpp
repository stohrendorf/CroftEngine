#include "block.h"

#include "core/boundingbox.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/vec.h"
#include "engine/collisioninfo.h"
#include "engine/floordata/floordata.h"
#include "engine/heightinfo.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/presenter.h"
#include "engine/skeletalmodelnode.h"
#include "engine/soundeffects_tr1.h"
#include "engine/world/room.h"
#include "engine/world/sector.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "hid/inputstate.h"
#include "laraobject.h"
#include "loader/file/larastateid.h"
#include "modelobject.h"
#include "object.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/serialization.h"

#include <boost/throw_exception.hpp>
#include <gl/renderstate.h>
#include <memory>
#include <stdexcept>

namespace engine::objects
{
void Block::collide(CollisionInfo& /*collisionInfo*/)
{
  if(!getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action)
     || m_state.triggerState == TriggerState::Active || getWorld().getObjectManager().getLara().m_state.falling
     || getWorld().getObjectManager().getLara().m_state.location.position.Y != m_state.location.position.Y)
  {
    return;
  }

  static const InteractionLimits limits{core::BoundingBox{{-300_len, 0_len, -692_len}, {200_len, 0_len, -512_len}},
                                        {-10_deg, -30_deg, -10_deg},
                                        {+10_deg, +30_deg, +10_deg}};

  auto axis = axisFromAngle(getWorld().getObjectManager().getLara().m_state.rotation.Y);

  if(getWorld().getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::Stop)
  {
    if(getWorld().getPresenter().getInputHandler().getInputState().zMovement != hid::AxisMovement::Null
       || getWorld().getObjectManager().getLara().getHandStatus() != HandStatus::None)
    {
      return;
    }

    const core::Angle y = snapRotation(axis);
    m_state.rotation.Y = y;

    if(!limits.canInteract(m_state, getWorld().getObjectManager().getLara().m_state))
    {
      return;
    }

    getWorld().getObjectManager().getLara().m_state.rotation.Y = y;

    core::Length core::TRVec::*vp;
    core::Length d;
    switch(axis)
    {
    case core::Axis::PosZ:
      d = 1_sectors - core::DefaultCollisionRadius;
      vp = &core::TRVec::Z;
      break;
    case core::Axis::PosX:
      d = 1_sectors - core::DefaultCollisionRadius;
      vp = &core::TRVec::X;
      break;
    case core::Axis::NegZ:
      d = core::DefaultCollisionRadius;
      vp = &core::TRVec::Z;
      break;
    case core::Axis::NegX:
      d = core::DefaultCollisionRadius;
      vp = &core::TRVec::X;
      break;
    default:
      BOOST_THROW_EXCEPTION(std::domain_error("Invalid axis"));
    }

    getWorld().getObjectManager().getLara().m_state.location.position.*vp
      = snappedSector(getWorld().getObjectManager().getLara().m_state.location.position.*vp) + d;

    getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::PushableGrab);
    getWorld().getObjectManager().getLara().advanceFrame();
    if(getWorld().getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::PushableGrab)
    {
      getWorld().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
    }
    return;
  }

  if(getWorld().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::PushableGrab
     || getWorld().getObjectManager().getLara().getSkeleton()->getFrame() != 2091_frame
     || !limits.canInteract(m_state, getWorld().getObjectManager().getLara().m_state))
  {
    return;
  }

  if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Forward)
  {
    if(!canPushBlock(1_sectors, axis))
    {
      return;
    }

    m_state.goal_anim_state = 2_as;
    getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::PushablePush);
  }
  else if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Backward)
  {
    if(!canPullBlock(1_sectors, axis))
    {
      return;
    }

    m_state.goal_anim_state = 3_as;
    getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::PushablePull);
  }
  else
  {
    return;
  }

  // start moving the block, remove it from the floordata
  activate();
  world::patchHeightsForBlock(*this, 1_sectors);
  getSkeleton()->resetInterpolation();
  m_state.triggerState = TriggerState::Active;

  ModelObject::update();
  getWorld().getObjectManager().getLara().advanceFrame();
}

void Block::update()
{
  if(m_state.activationState.isOneshot())
  {
    world::patchHeightsForBlock(*this, 1_sectors);
    getSkeleton()->resetInterpolation();
    kill();
    return;
  }

  ModelObject::update();

  auto location = m_state.location;
  auto sector = location.updateRoom();
  const auto height = HeightInfo::fromFloor(sector, location.position, getWorld().getObjectManager().getObjects()).y;
  if(height > location.position.Y)
  {
    m_state.falling = true;
  }
  else if(m_state.falling)
  {
    location.position.Y = height;
    m_state.location.position = location.position;
    m_state.falling = false;
    m_state.triggerState = TriggerState::Deactivated;
    getWorld().dinoStompEffect(*this);
    playSoundEffect(TR1SoundEffect::TRexFootstep);
    applyTransform(); // needed for properly placing geometry on floor
  }

  setCurrentRoom(location.room);

  if(m_state.triggerState != TriggerState::Deactivated)
  {
    return;
  }

  m_state.triggerState = TriggerState::Inactive;
  deactivate();
  world::patchHeightsForBlock(*this, -1_sectors);
  getSkeleton()->resetInterpolation();
  location = m_state.location;
  sector = location.updateRoom();
  getWorld().handleCommandSequence(
    HeightInfo::fromFloor(sector, location.position, getWorld().getObjectManager().getObjects())
      .lastCommandSequenceOrDeath,
    true);
}

bool Block::isOnFloor(const core::Length& height) const
{
  const auto sector = m_state.location.moved({}).updateRoom();
  return sector->floorHeight == core::InvalidHeight || sector->floorHeight == m_state.location.position.Y - height;
}

bool Block::canPushBlock(const core::Length& height, const core::Axis axis) const
{
  if(!isOnFloor(height))
  {
    return false;
  }

  auto location = m_state.location;
  switch(axis)
  {
  case core::Axis::PosZ:
    location.position.Z += 1_sectors;
    break;
  case core::Axis::PosX:
    location.position.X += 1_sectors;
    break;
  case core::Axis::NegZ:
    location.position.Z -= 1_sectors;
    break;
  case core::Axis::NegX:
    location.position.X -= 1_sectors;
    break;
  default:
    break;
  }

  CollisionInfo tmp;
  tmp.facingAxis = axis;
  tmp.collisionRadius = 500_len;
  if(tmp.checkStaticMeshCollisions(location.position, 2 * tmp.collisionRadius, getWorld()))
  {
    return false;
  }

  const auto targetSector = location.updateRoom();
  if(targetSector->floorHeight != location.position.Y)
  {
    return false;
  }

  location.position.Y -= height;
  return location.position.Y >= location.updateRoom()->ceilingHeight;
}

bool Block::canPullBlock(const core::Length& height, const core::Axis axis) const
{
  if(!isOnFloor(height))
  {
    return false;
  }

  auto location = m_state.location;
  switch(axis)
  {
  case core::Axis::Deg0:
    location.position.Z -= 1_sectors;
    break;
  case core::Axis::Right90:
    location.position.X -= 1_sectors;
    break;
  case core::Axis::Deg180:
    location.position.Z += 1_sectors;
    break;
  case core::Axis::Left90:
    location.position.X += 1_sectors;
    break;
  default:
    break;
  }

  auto sector = location.updateRoom();

  CollisionInfo tmp;
  tmp.facingAxis = axis;
  tmp.collisionRadius = 500_len;
  if(tmp.checkStaticMeshCollisions(location.position, 2 * tmp.collisionRadius, getWorld()))
  {
    return false;
  }

  if(sector->floorHeight != location.position.Y)
  {
    return false;
  }

  auto topPos = location;
  topPos.position.Y -= height;
  const auto topSector = topPos.updateRoom();
  if(topPos.position.Y < topSector->ceilingHeight)
  {
    return false;
  }

  auto laraLocation = location;
  switch(axis)
  {
  case core::Axis::PosZ:
    laraLocation.position.Z -= 1_sectors;
    break;
  case core::Axis::PosX:
    laraLocation.position.X -= 1_sectors;
    break;
  case core::Axis::NegZ:
    laraLocation.position.Z += 1_sectors;
    break;
  case core::Axis::NegX:
    laraLocation.position.X += 1_sectors;
    break;
  default:
    break;
  }

  sector = laraLocation.updateRoom();
  if(sector->floorHeight != location.position.Y)
  {
    return false;
  }

  laraLocation.position.Y -= core::LaraWalkHeight;
  sector = laraLocation.updateRoom();
  if(laraLocation.position.Y < sector->ceilingHeight)
  {
    return false;
  }

  laraLocation = getWorld().getObjectManager().getLara().m_state.location;
  switch(axis)
  {
  case core::Axis::Deg0:
    laraLocation.position.Z -= 1_sectors;
    tmp.facingAxis = core::Axis::Deg180;
    break;
  case core::Axis::Right90:
    laraLocation.position.X -= 1_sectors;
    tmp.facingAxis = core::Axis::Left90;
    break;
  case core::Axis::Deg180:
    laraLocation.position.Z += 1_sectors;
    tmp.facingAxis = core::Axis::Deg0;
    break;
  case core::Axis::Left90:
    laraLocation.position.X += 1_sectors;
    tmp.facingAxis = core::Axis::Right90;
    break;
  default:
    break;
  }
  tmp.collisionRadius = core::DefaultCollisionRadius;

  return !tmp.checkStaticMeshCollisions(laraLocation.position, core::LaraWalkHeight, getWorld());
}

void Block::serialize(const serialization::Serializer<world::World>& ser) const
{
  ModelObject::serialize(ser);
}

void Block::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ModelObject::deserialize(ser);
  getSkeleton()->getRenderState().setScissorTest(false);
}

Block::Block(const std::string& name,
             const gsl::not_null<world::World*>& world,
             const gsl::not_null<const world::Room*>& room,
             const loader::file::Item& item,
             const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel, false}
{
  if(m_state.triggerState != TriggerState::Invisible)
  {
    world::patchHeightsForBlock(*this, -1_sectors);
    getSkeleton()->resetInterpolation();
  }
  getSkeleton()->getRenderState().setScissorTest(false);
}
} // namespace engine::objects
