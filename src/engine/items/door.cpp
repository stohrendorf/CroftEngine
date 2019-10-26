#include "door.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
// #define NO_DOOR_BLOCK

Door::Door(const gsl::not_null<engine::Engine*>& engine,
           const gsl::not_null<const loader::file::Room*>& room,
           const loader::file::Item& item,
           const loader::file::SkeletalModelType& animatedModel)
    : ModelItemNode{engine, room, item, true, animatedModel}
{
#ifndef NO_DOOR_BLOCK
  core::Length dx = 0_len, dz = 0_len;
  const auto axis = core::axisFromAngle(m_state.rotation.Y, 45_deg);
  Expects(axis.is_initialized());
  switch(*axis)
  {
  case core::Axis::PosZ: dz = -core::SectorSize; break;
  case core::Axis::PosX: dx = -core::SectorSize; break;
  case core::Axis::NegZ: dz = core::SectorSize; break;
  case core::Axis::NegX: dx = core::SectorSize; break;
  }

  const auto wingsPosition = m_state.position.position + core::TRVec{dx, 0_len, dz};

  if(const auto portalTarget = m_info.init(*m_state.position.room, wingsPosition))
  {
    m_target.init(*portalTarget, m_state.position.position);
  }

  if(m_state.position.room->alternateRoom.get() >= 0)
  {
    if(const auto alternatePortalTarget
       = m_alternateInfo.init(getEngine().getRooms().at(m_state.position.room->alternateRoom.get()), wingsPosition))
    {
      m_alternateTarget.init(*alternatePortalTarget, m_state.position.position);
    }
  }
#endif
}

void Door::update()
{
  if(m_state.updateActivationTimeout())
  {
    if(m_state.current_anim_state == 0_as)
    {
      m_state.goal_anim_state = 1_as;
    }
    else
    {
#ifndef NO_DOOR_BLOCK
      m_info.open();
      m_target.open();
      m_alternateInfo.open();
      m_alternateTarget.open();
#endif
    }
  }
  else
  {
    if(m_state.current_anim_state == 1_as)
    {
      m_state.goal_anim_state = 0_as;
    }
    else
    {
#ifndef NO_DOOR_BLOCK
      m_info.close();
      m_target.close();
      m_alternateInfo.close();
      m_alternateTarget.close();
#endif
    }
  }

  ModelItemNode::update();
}

void Door::collide(LaraNode& lara, CollisionInfo& collisionInfo)
{
#ifndef NO_DOOR_BLOCK
  if(!isNear(lara, collisionInfo.collisionRadius))
    return;

  if(!testBoneCollision(lara))
    return;

  if(!collisionInfo.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  if(m_state.current_anim_state == m_state.goal_anim_state)
  {
    enemyPush(lara, collisionInfo, false, true);
  }
  else
  {
    const auto enableSpaz = collisionInfo.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableSpaz);
    enemyPush(lara, collisionInfo, enableSpaz, true);
  }
#endif
}
} // namespace items
}