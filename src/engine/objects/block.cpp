#include "block.h"

#include "core/boundingbox.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/inputhandler.h"
#include "laraobject.h"
#include "serialization/quantity.h"

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
      d = core::SectorSize - core::DefaultCollisionRadius;
      vp = &core::TRVec::Z;
      break;
    case core::Axis::PosX:
      d = core::SectorSize - core::DefaultCollisionRadius;
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
    default: BOOST_THROW_EXCEPTION(std::domain_error("Invalid axis"));
    }

    getWorld().getObjectManager().getLara().m_state.location.position.*vp
      = (getWorld().getObjectManager().getLara().m_state.location.position.*vp / core::SectorSize) * core::SectorSize
        + d;

    getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::PushableGrab);
    getWorld().getObjectManager().getLara().updateImpl();
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
    if(!canPushBlock(core::SectorSize, axis))
    {
      return;
    }

    m_state.goal_anim_state = 2_as;
    getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::PushablePush);
  }
  else if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Backward)
  {
    if(!canPullBlock(core::SectorSize, axis))
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
  world::patchHeightsForBlock(*this, core::SectorSize);
  m_state.triggerState = TriggerState::Active;

  ModelObject::update();
  getWorld().getObjectManager().getLara().updateImpl();
}

void Block::update()
{
  if(m_state.activationState.isOneshot())
  {
    world::patchHeightsForBlock(*this, core::SectorSize);
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
  world::patchHeightsForBlock(*this, -core::SectorSize);
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
  return sector->floorHeight == -core::HeightLimit || sector->floorHeight == m_state.location.position.Y - height;
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
  case core::Axis::PosZ: location.position.Z += core::SectorSize; break;
  case core::Axis::PosX: location.position.X += core::SectorSize; break;
  case core::Axis::NegZ: location.position.Z -= core::SectorSize; break;
  case core::Axis::NegX: location.position.X -= core::SectorSize; break;
  default: break;
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
  case core::Axis::Deg0: location.position.Z -= core::SectorSize; break;
  case core::Axis::Right90: location.position.X -= core::SectorSize; break;
  case core::Axis::Deg180: location.position.Z += core::SectorSize; break;
  case core::Axis::Left90: location.position.X += core::SectorSize; break;
  default: break;
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
  case core::Axis::PosZ: laraLocation.position.Z -= core::SectorSize; break;
  case core::Axis::PosX: laraLocation.position.X -= core::SectorSize; break;
  case core::Axis::NegZ: laraLocation.position.Z += core::SectorSize; break;
  case core::Axis::NegX: laraLocation.position.X += core::SectorSize; break;
  default: break;
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
    laraLocation.position.Z -= core::SectorSize;
    tmp.facingAxis = core::Axis::Deg180;
    break;
  case core::Axis::Right90:
    laraLocation.position.X -= core::SectorSize;
    tmp.facingAxis = core::Axis::Left90;
    break;
  case core::Axis::Deg180:
    laraLocation.position.Z += core::SectorSize;
    tmp.facingAxis = core::Axis::Deg0;
    break;
  case core::Axis::Left90:
    laraLocation.position.X += core::SectorSize;
    tmp.facingAxis = core::Axis::Right90;
    break;
  default: break;
  }
  tmp.collisionRadius = core::DefaultCollisionRadius;

  return !tmp.checkStaticMeshCollisions(laraLocation.position, core::LaraWalkHeight, getWorld());
}
} // namespace engine::objects
