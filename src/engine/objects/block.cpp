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
     || getWorld().getObjectManager().getLara().m_state.position.position.Y != m_state.position.position.Y)
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

    getWorld().getObjectManager().getLara().m_state.position.position.*vp
      = (getWorld().getObjectManager().getLara().m_state.position.position.*vp / core::SectorSize) * core::SectorSize
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

  auto pos = m_state.position;
  auto sector = world::findRealFloorSector(pos);
  const auto height = HeightInfo::fromFloor(sector, pos.position, getWorld().getObjectManager().getObjects()).y;
  if(height > pos.position.Y)
  {
    m_state.falling = true;
  }
  else if(m_state.falling)
  {
    pos.position.Y = height;
    m_state.position.position = pos.position;
    m_state.falling = false;
    m_state.triggerState = TriggerState::Deactivated;
    getWorld().dinoStompEffect(*this);
    playSoundEffect(TR1SoundEffect::TRexFootstep);
    applyTransform(); // needed for properly placing geometry on floor
  }

  setCurrentRoom(pos.room);

  if(m_state.triggerState != TriggerState::Deactivated)
  {
    return;
  }

  m_state.triggerState = TriggerState::Inactive;
  deactivate();
  world::patchHeightsForBlock(*this, -core::SectorSize);
  pos = m_state.position;
  sector = world::findRealFloorSector(pos);
  getWorld().handleCommandSequence(
    HeightInfo::fromFloor(sector, pos.position, getWorld().getObjectManager().getObjects()).lastCommandSequenceOrDeath,
    true);
}

bool Block::isOnFloor(const core::Length& height) const
{
  const auto sector = findRealFloorSector(m_state.position.position, m_state.position.room);
  return sector->floorHeight == -core::HeightLimit || sector->floorHeight == m_state.position.position.Y - height;
}

bool Block::canPushBlock(const core::Length& height, const core::Axis axis) const
{
  if(!isOnFloor(height))
  {
    return false;
  }

  auto pos = m_state.position.position;
  switch(axis)
  {
  case core::Axis::PosZ: pos.Z += core::SectorSize; break;
  case core::Axis::PosX: pos.X += core::SectorSize; break;
  case core::Axis::NegZ: pos.Z -= core::SectorSize; break;
  case core::Axis::NegX: pos.X -= core::SectorSize; break;
  default: break;
  }

  CollisionInfo tmp;
  tmp.facingAxis = axis;
  tmp.collisionRadius = 500_len;
  if(tmp.checkStaticMeshCollisions(pos, 2 * tmp.collisionRadius, getWorld()))
  {
    return false;
  }

  const auto targetSector = findRealFloorSector(pos, m_state.position.room);
  if(targetSector->floorHeight != pos.Y)
  {
    return false;
  }

  pos.Y -= height;
  return pos.Y >= findRealFloorSector(pos, m_state.position.room)->ceilingHeight;
}

bool Block::canPullBlock(const core::Length& height, const core::Axis axis) const
{
  if(!isOnFloor(height))
  {
    return false;
  }

  auto pos = m_state.position.position;
  switch(axis)
  {
  case core::Axis::Deg0: pos.Z -= core::SectorSize; break;
  case core::Axis::Right90: pos.X -= core::SectorSize; break;
  case core::Axis::Deg180: pos.Z += core::SectorSize; break;
  case core::Axis::Left90: pos.X += core::SectorSize; break;
  default: break;
  }

  auto room = m_state.position.room;
  auto sector = findRealFloorSector(pos, &room);

  CollisionInfo tmp;
  tmp.facingAxis = axis;
  tmp.collisionRadius = 500_len;
  if(tmp.checkStaticMeshCollisions(pos, 2 * tmp.collisionRadius, getWorld()))
  {
    return false;
  }

  if(sector->floorHeight != pos.Y)
  {
    return false;
  }

  auto topPos = pos;
  topPos.Y -= height;
  const auto topSector = findRealFloorSector(topPos, m_state.position.room);
  if(topPos.Y < topSector->ceilingHeight)
  {
    return false;
  }

  auto laraPos = pos;
  switch(axis)
  {
  case core::Axis::PosZ: laraPos.Z -= core::SectorSize; break;
  case core::Axis::PosX: laraPos.X -= core::SectorSize; break;
  case core::Axis::NegZ: laraPos.Z += core::SectorSize; break;
  case core::Axis::NegX: laraPos.X += core::SectorSize; break;
  default: break;
  }

  sector = findRealFloorSector(laraPos, &room);
  if(sector->floorHeight != pos.Y)
  {
    return false;
  }

  laraPos.Y -= core::LaraWalkHeight;
  sector = findRealFloorSector(laraPos, &room);
  if(laraPos.Y < sector->ceilingHeight)
  {
    return false;
  }

  laraPos = getWorld().getObjectManager().getLara().m_state.position.position;
  switch(axis)
  {
  case core::Axis::Deg0:
    laraPos.Z -= core::SectorSize;
    tmp.facingAxis = core::Axis::Deg180;
    break;
  case core::Axis::Right90:
    laraPos.X -= core::SectorSize;
    tmp.facingAxis = core::Axis::Left90;
    break;
  case core::Axis::Deg180:
    laraPos.Z += core::SectorSize;
    tmp.facingAxis = core::Axis::Deg0;
    break;
  case core::Axis::Left90:
    laraPos.X += core::SectorSize;
    tmp.facingAxis = core::Axis::Right90;
    break;
  default: break;
  }
  tmp.collisionRadius = core::DefaultCollisionRadius;

  return !tmp.checkStaticMeshCollisions(laraPos, core::LaraWalkHeight, getWorld());
}
} // namespace engine::objects
