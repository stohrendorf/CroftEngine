#include "block.h"

#include "core/boundingbox.h"
#include "hid/inputhandler.h"
#include "laraobject.h"

namespace engine::objects
{
void Block::collide(CollisionInfo& /*collisionInfo*/)
{
  if(!getEngine().getInputHandler().getInputState().action || m_state.triggerState == TriggerState::Active
     || getEngine().getLara().m_state.falling
     || getEngine().getLara().m_state.position.position.Y != m_state.position.position.Y)
  {
    return;
  }

  static const InteractionLimits limits{core::BoundingBox{{-300_len, 0_len, -692_len}, {200_len, 0_len, -512_len}},
                                        {-10_deg, -30_deg, -10_deg},
                                        {+10_deg, +30_deg, +10_deg}};

  auto axis = axisFromAngle(getEngine().getLara().m_state.rotation.Y, 45_deg);
  Expects(axis.has_value());

  if(getEngine().getLara().getCurrentAnimState() == loader::file::LaraStateId::Stop)
  {
    if(getEngine().getInputHandler().getInputState().zMovement != hid::AxisMovement::Null
       || getEngine().getLara().getHandStatus() != HandStatus::None)
    {
      return;
    }

    const core::Angle y = alignRotation(*axis);
    m_state.rotation.Y = y;

    if(!limits.canInteract(m_state, getEngine().getLara().m_state))
    {
      return;
    }

    getEngine().getLara().m_state.rotation.Y = y;

    core::Length core::TRVec::*vp;
    core::Length d = 0_len;
    switch(*axis)
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

    getEngine().getLara().m_state.position.position.*vp
      = (getEngine().getLara().m_state.position.position.*vp / core::SectorSize) * core::SectorSize + d;

    getEngine().getLara().setGoalAnimState(loader::file::LaraStateId::PushableGrab);
    getEngine().getLara().updateImpl();
    if(getEngine().getLara().getCurrentAnimState() == loader::file::LaraStateId::PushableGrab)
    {
      getEngine().getLara().setHandStatus(HandStatus::Grabbing);
    }
    return;
  }

  if(getEngine().getLara().getCurrentAnimState() != loader::file::LaraStateId::PushableGrab
     || getEngine().getLara().getSkeleton()->frame_number != 2091_frame
     || !limits.canInteract(m_state, getEngine().getLara().m_state))
  {
    return;
  }

  if(getEngine().getInputHandler().getInputState().zMovement == hid::AxisMovement::Forward)
  {
    if(!canPushBlock(core::SectorSize, *axis))
    {
      return;
    }

    m_state.goal_anim_state = 2_as;
    getEngine().getLara().setGoalAnimState(loader::file::LaraStateId::PushablePush);
  }
  else if(getEngine().getInputHandler().getInputState().zMovement == hid::AxisMovement::Backward)
  {
    if(!canPullBlock(core::SectorSize, *axis))
    {
      return;
    }

    m_state.goal_anim_state = 3_as;
    getEngine().getLara().setGoalAnimState(loader::file::LaraStateId::PushablePull);
  }
  else
  {
    return;
  }

  // start moving the block, remove it from the floordata
  activate();
  loader::file::Room::patchHeightsForBlock(*this, core::SectorSize);
  m_patched = false;
  m_state.triggerState = TriggerState::Active;

  ModelObject::update();
  getEngine().getLara().updateImpl();
}

void Block::update()
{
  if(m_state.activationState.isOneshot())
  {
    loader::file::Room::patchHeightsForBlock(*this, core::SectorSize);
    m_patched = false;
    kill();
    return;
  }

  ModelObject::update();

  auto pos = m_state.position;
  auto sector = loader::file::findRealFloorSector(pos);
  const auto height = HeightInfo::fromFloor(sector, pos.position, getEngine().getObjectManager().getObjects()).y;
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
    getEngine().dinoStompEffect(*this);
    playSoundEffect(TR1SoundId::TRexFootstep);
    applyTransform(); // needed for properly placing geometry on floor
  }

  setCurrentRoom(pos.room);

  if(m_state.triggerState != TriggerState::Deactivated)
  {
    return;
  }

  m_state.triggerState = TriggerState::Inactive;
  deactivate();
  loader::file::Room::patchHeightsForBlock(*this, -core::SectorSize);
  m_patched = true;
  pos = m_state.position;
  sector = loader::file::findRealFloorSector(pos);
  getEngine().handleCommandSequence(
    HeightInfo::fromFloor(sector, pos.position, getEngine().getObjectManager().getObjects()).lastCommandSequenceOrDeath,
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
  if(tmp.checkStaticMeshCollisions(pos, 2 * tmp.collisionRadius, getEngine()))
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
  case core::Axis::PosZ: pos.Z -= core::SectorSize; break;
  case core::Axis::PosX: pos.X -= core::SectorSize; break;
  case core::Axis::NegZ: pos.Z += core::SectorSize; break;
  case core::Axis::NegX: pos.X += core::SectorSize; break;
  default: break;
  }

  auto room = m_state.position.room;
  auto sector = findRealFloorSector(pos, &room);

  CollisionInfo tmp;
  tmp.facingAxis = axis;
  tmp.collisionRadius = 500_len;
  if(tmp.checkStaticMeshCollisions(pos, 2 * tmp.collisionRadius, getEngine()))
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

  laraPos = getEngine().getLara().m_state.position.position;
  switch(axis)
  {
  case core::Axis::PosZ:
    laraPos.Z -= core::SectorSize;
    tmp.facingAxis = core::Axis::NegZ;
    break;
  case core::Axis::PosX:
    laraPos.X -= core::SectorSize;
    tmp.facingAxis = core::Axis::NegX;
    break;
  case core::Axis::NegZ:
    laraPos.Z += core::SectorSize;
    tmp.facingAxis = core::Axis::PosZ;
    break;
  case core::Axis::NegX:
    laraPos.X += core::SectorSize;
    tmp.facingAxis = core::Axis::PosX;
    break;
  default: break;
  }
  tmp.collisionRadius = core::DefaultCollisionRadius;

  return !tmp.checkStaticMeshCollisions(laraPos, core::LaraWalkHeight, getEngine());
}

void Block::serialize(const serialization::Serializer& ser)
{
  if(m_patched)
    loader::file::Room::patchHeightsForBlock(*this, core::SectorSize);

  ModelObject::serialize(ser);
  ser(S_NV("patched", m_patched));

  if(m_patched)
    loader::file::Room::patchHeightsForBlock(*this, -core::SectorSize);
}
} // namespace engine::objects
