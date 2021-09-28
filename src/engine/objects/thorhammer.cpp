#include "thorhammer.h"

#include "core/angle.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/collisioninfo.h"
#include "engine/heightinfo.h"
#include "engine/items_tr1.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/animation.h"
#include "engine/world/room.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "loader/file/animationid.h"
#include "loader/file/item.h"
#include "loader/file/larastateid.h"
#include "modelobject.h"
#include "object.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/objectreference.h"
#include "serialization/serialization.h"
#include "util/helpers.h"

#include <exception>
#include <gl/renderstate.h>
#include <iterator>
#include <map>

namespace engine::objects
{
namespace
{
constexpr auto Idle = 0_as;
constexpr auto Raising = 1_as;
constexpr auto Falling = 2_as;
constexpr auto Settle = 3_as;
} // namespace

ThorHammerHandle::ThorHammerHandle(const std::string& name,
                                   const gsl::not_null<world::World*>& world,
                                   const gsl::not_null<const world::Room*>& room,
                                   loader::file::Item item,
                                   const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel}
{
  item.type = TR1ItemId::ThorHammerBlock;
  m_block = std::make_shared<ThorHammerBlock>(
    makeObjectName(item.type.get_as<TR1ItemId>(), 999999),
    world,
    room,
    item,
    gsl::not_null{world->findAnimatedModelForType(TR1ItemId::ThorHammerBlock).get()});
  getWorld().getObjectManager().registerObject(gsl::not_null{m_block});
  m_block->activate();
  m_block->m_state.triggerState = TriggerState::Active;

  getSkeleton()->getRenderState().setScissorTest(false);
}

ThorHammerHandle::ThorHammerHandle(const gsl::not_null<world::World*>& world, const Location& location)
    : ModelObject{world, location}
{
}

void ThorHammerHandle::update()
{
  switch(m_state.current_anim_state.get())
  {
  case Idle.get():
    if(m_state.updateActivationTimeout())
    {
      m_state.goal_anim_state = Raising;
    }
    else
    {
      deactivate();
      m_state.triggerState = TriggerState::Inactive;
    }
    break;
  case Raising.get():
    if(m_state.updateActivationTimeout())
    {
      m_state.goal_anim_state = Falling;
    }
    else
    {
      m_state.goal_anim_state = Idle;
    }
    break;
  case Falling.get():
    if(getSkeleton()->getLocalFrame() > 30_frame)
    {
      const auto pos = m_state.location.position + util::pitch(3 * core::SectorSize, m_state.rotation.Y);

      if(auto& lara = getWorld().getObjectManager().getLara(); !lara.isDead())
      {
        core::TRVec& laraPos = lara.m_state.location.position;
        if(pos.X - 520_len < laraPos.X && pos.X + 520_len > laraPos.X && pos.Z - 520_len < laraPos.Z
           && pos.Z + 520_len > laraPos.Z)
        {
          lara.m_state.health = core::DeadHealth;
          lara.setAnimation(loader::file::AnimationId::SQUASH_BOULDER);
          lara.setCurrentAnimState(loader::file::LaraStateId::BoulderDeath);
          lara.setGoalAnimState(loader::file::LaraStateId::BoulderDeath);
          laraPos.Y = m_state.location.position.Y;
          lara.m_state.falling = false;
        }
      }
    }
    break;
  case Settle.get():
  {
    const auto sector = m_state.location.moved({}).updateRoom();
    const auto hi
      = HeightInfo::fromFloor(sector, m_state.location.position, getWorld().getObjectManager().getObjects());
    getWorld().handleCommandSequence(hi.lastCommandSequenceOrDeath, true);

    const auto oldPosX = m_state.location.position.X;
    const auto oldPosZ = m_state.location.position.Z;
    if(m_state.rotation.Y == 0_deg)
      m_state.location.position.Z += 3 * core::SectorSize;
    else if(m_state.rotation.Y == 90_deg)
      m_state.location.position.X += 3 * core::SectorSize;
    else if(m_state.rotation.Y == 180_deg)
      m_state.location.position.Z -= 3 * core::SectorSize;
    else if(m_state.rotation.Y == -90_deg)
      m_state.location.position.X -= 3 * core::SectorSize;
    if(!getWorld().getObjectManager().getLara().isDead())
    {
      world::patchHeightsForBlock(*this, -2 * core::SectorSize);
    }
    m_state.location.position.X = oldPosX;
    m_state.location.position.Z = oldPosZ;
    deactivate();
    m_state.triggerState = TriggerState::Deactivated;
    break;
  }
  default: break;
  }
  ModelObject::update();

  // sync anim
  const auto animIdx = std::distance(&getWorld().findAnimatedModelForType(TR1ItemId::ThorHammerHandle)->animations[0],
                                     getSkeleton()->getAnim());
  m_block->getSkeleton()->replaceAnim(
    gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::ThorHammerBlock)->animations[animIdx]},
    getSkeleton()->getLocalFrame());
  m_block->m_state.current_anim_state = m_state.current_anim_state;
}

void ThorHammerHandle::collide(CollisionInfo& info)
{
  if(!info.policies.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  if(!isNear(getWorld().getObjectManager().getLara(), info.collisionRadius))
    return;

  enemyPush(info, false, true);
}

void ThorHammerHandle::serialize(const serialization::Serializer<world::World>& ser)
{
  ModelObject::serialize(ser);
  ser(S_NV("block", serialization::ObjectReference{m_block}));
  if(ser.loading)
    getSkeleton()->getRenderState().setScissorTest(false);
}

void ThorHammerBlock::collide(CollisionInfo& info)
{
  if(m_state.current_anim_state == Falling)
    return;

  if(!info.policies.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  if(!isNear(getWorld().getObjectManager().getLara(), info.collisionRadius))
    return;

  enemyPush(info, false, true);
}

void ThorHammerBlock::serialize(const serialization::Serializer<world::World>& ser)
{
  ModelObject::serialize(ser);
  if(ser.loading)
    getSkeleton()->getRenderState().setScissorTest(false);
}

ThorHammerBlock::ThorHammerBlock(const std::string& name,
                                 const gsl::not_null<world::World*>& world,
                                 const gsl::not_null<const world::Room*>& room,
                                 const loader::file::Item& item,
                                 const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel}
{
  getSkeleton()->getRenderState().setScissorTest(false);
}
} // namespace engine::objects
