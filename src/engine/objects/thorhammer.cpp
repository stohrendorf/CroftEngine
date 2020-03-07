#include "thorhammer.h"

#include "laraobject.h"

namespace engine::objects
{
ThorHammerHandle::ThorHammerHandle(const gsl::not_null<Engine*>& engine,
                                   const gsl::not_null<const loader::file::Room*>& room,
                                   const loader::file::Item& item,
                                   const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
    : ModelObject{engine, room, item, true, animatedModel}
    , m_block{engine->createObject<ThorHammerBlock>(TR1ItemId::ThorHammerBlock, room, item.rotation, item.position, 0)}
{
  m_block->activate();
  m_block->m_state.triggerState = TriggerState::Active;
}

void ThorHammerHandle::update()
{
  switch(m_state.current_anim_state.get())
  {
  case 0:
    if(m_state.updateActivationTimeout())
    {
      m_state.goal_anim_state = 1_as;
    }
    else
    {
      deactivate();
      m_state.triggerState = TriggerState::Inactive;
    }
    break;
  case 1:
    if(m_state.updateActivationTimeout())
    {
      m_state.goal_anim_state = 2_as;
    }
    else
    {
      m_state.goal_anim_state = 0_as;
    }
    break;
  case 2:
    if(getSkeleton()->frame_number > getSkeleton()->anim->firstFrame + 30_frame)
    {
      auto posX = m_state.position.position.X;
      auto posZ = m_state.position.position.Z;
      if(m_state.rotation.Y == 0_deg)
      {
        posZ += 3 * core::SectorSize;
      }
      else if(m_state.rotation.Y == 90_deg)
      {
        posX += 3 * core::SectorSize;
      }
      else if(m_state.rotation.Y == 180_deg)
      {
        posZ -= 3 * core::SectorSize;
      }
      else if(m_state.rotation.Y == -90_deg)
      {
        posX -= 3 * core::SectorSize;
      }
      if(getEngine().getLara().m_state.health >= 0_hp)
      {
        if(posX - 520_len < getEngine().getLara().m_state.position.position.X
           && posX + 520_len > getEngine().getLara().m_state.position.position.X
           && posZ - 520_len < getEngine().getLara().m_state.position.position.Z
           && posZ + 520_len > getEngine().getLara().m_state.position.position.Z)
        {
          getEngine().getLara().m_state.health = -1_hp;
          getEngine().getLara().getSkeleton()->anim
            = &getEngine().findAnimatedModelForType(TR1ItemId::Lara)->animations[139];
          getEngine().getLara().getSkeleton()->frame_number = 3561_frame;
          getEngine().getLara().setCurrentAnimState(loader::file::LaraStateId::BoulderDeath);
          getEngine().getLara().setGoalAnimState(loader::file::LaraStateId::BoulderDeath);
          getEngine().getLara().m_state.position.position.Y = m_state.position.position.Y;
          getEngine().getLara().m_state.falling = false;
        }
      }
    }
    break;
  case 3:
  {
    const auto sector = findRealFloorSector(m_state.position.position, m_state.position.room);
    const auto hi = HeightInfo::fromFloor(sector, m_state.position.position, getEngine().getObjects());
    getEngine().handleCommandSequence(hi.lastCommandSequenceOrDeath, true);

    const auto oldPosX = m_state.position.position.X;
    const auto oldPosZ = m_state.position.position.Z;
    if(m_state.rotation.Y == 0_deg)
    {
      m_state.position.position.Z += 3 * core::SectorSize;
    }
    else if(m_state.rotation.Y == 90_deg)
    {
      m_state.position.position.X += 3 * core::SectorSize;
    }
    else if(m_state.rotation.Y == 180_deg)
    {
      m_state.position.position.Z -= 3 * core::SectorSize;
    }
    else if(m_state.rotation.Y == -90_deg)
    {
      m_state.position.position.X -= 3 * core::SectorSize;
    }
    if(getEngine().getLara().m_state.health >= 0_hp)
    {
      m_state.position.room->patchHeightsForBlock(*this, -2 * core::SectorSize);
    }
    m_state.position.position.X = oldPosX;
    m_state.position.position.Z = oldPosZ;
    deactivate();
    m_state.triggerState = TriggerState::Deactivated;
    break;
  }
  default: break;
  }
  ModelObject::update();

  // sync anim
  const auto animIdx = std::distance(&getEngine().findAnimatedModelForType(TR1ItemId::ThorHammerHandle)->animations[0],
                                     getSkeleton()->anim);
  m_block->getSkeleton()->anim = &getEngine().findAnimatedModelForType(TR1ItemId::ThorHammerBlock)->animations[animIdx];
  m_block->getSkeleton()->frame_number
    = getSkeleton()->frame_number - getSkeleton()->anim->firstFrame + m_block->getSkeleton()->anim->firstFrame;
  m_block->m_state.current_anim_state = m_state.current_anim_state;
}

void ThorHammerHandle::collide(CollisionInfo& info)
{
  if(!info.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  if(!isNear(getEngine().getLara(), info.collisionRadius))
    return;

  enemyPush(info, false, true);
}

void ThorHammerBlock::collide(CollisionInfo& info)
{
  if(m_state.current_anim_state == 2_as)
    return;

  if(!info.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  if(!isNear(getEngine().getLara(), info.collisionRadius))
    return;

  enemyPush(info, false, true);
}
} // namespace engine::objects
