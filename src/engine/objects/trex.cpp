#include "trex.h"

#include "aiagent.h"
#include "core/angle.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/ai/ai.h"
#include "engine/cameracontroller.h"
#include "engine/items_tr1.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/player.h"
#include "engine/skeletalmodelnode.h"
#include "engine/weapontype.h"
#include "engine/world/animation.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "loader/file/larastateid.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/serialization.h"
#include "util/helpers.h"

#include <exception>
#include <gl/renderstate.h>
#include <memory>

namespace engine::objects
{
void TRex::update()
{
  static constexpr auto Think = 1_as;
  static constexpr auto Attack = 2_as;
  static constexpr auto RunningAttack = 3_as;
  static constexpr auto BiteToDeath = 7_as;

  activateAi();

  core::Angle rotationToMoveTarget;

  core::Angle creatureHead = 0_deg;
  if(alive())
  {
    const ai::EnemyLocation enemyLocation{*this};
    if(enemyLocation.enemyAhead)
    {
      creatureHead = enemyLocation.angleToEnemy;
    }
    updateMood(*this, enemyLocation, true);

    rotationToMoveTarget = rotateTowardsTarget(getCreatureInfo()->maxTurnSpeed);

    auto& lara = getWorld().getObjectManager().getLara();
    if(touched())
    {
      if(m_state.current_anim_state == RunningAttack)
        lara.m_state.health -= 10_hp;
      else
        lara.m_state.health -= 1_hp;
    }

    m_wantAttack = !isEscaping() && !enemyLocation.enemyAhead && abs(enemyLocation.enemyAngleToSelf) < 90_deg;
    if(!m_wantAttack && enemyLocation.enemyDistance > util::square(1500_len)
       && enemyLocation.enemyDistance < util::square(4 * core::SectorSize) && enemyLocation.canAttackForward)
    {
      m_wantAttack = true;
    }

    switch(m_state.current_anim_state.get())
    {
    case Think.get():
      if(m_state.required_anim_state != 0_as)
        goal(m_state.required_anim_state);
      else if(enemyLocation.enemyDistance < util::square(1500_len) && enemyLocation.canAttackForward)
        goal(BiteToDeath);
      else if(isBored() || m_wantAttack)
        goal(Attack);
      else
        goal(RunningAttack);
      break;
    case Attack.get():
      getCreatureInfo()->maxTurnSpeed = 2_deg / 1_frame;
      if(!isBored() || !m_wantAttack)
        goal(Think);
      else if(enemyLocation.enemyAhead && util::rand15() < 512)
        goal(Think, 6_as);
      break;
    case RunningAttack.get():
      getCreatureInfo()->maxTurnSpeed = 4_deg / 1_frame;
      if(enemyLocation.enemyDistance < util::square(5 * core::SectorSize) && enemyLocation.canAttackForward)
        goal(Think); // NOLINT(bugprone-branch-clone)
      else if(m_wantAttack)
        goal(Think);
      else if(!isEscaping() && enemyLocation.enemyAhead && util::rand15() < 512)
        goal(Think, 6_as);
      else if(isBored())
        goal(Think);
      break;
    case BiteToDeath.get():
      if(touched(0x3000UL))
      {
        goal(8_as);

        lara.m_state.falling = false;

        lara.setCurrentRoom(m_state.location.room);
        lara.m_state.location.position = m_state.location.position;
        lara.m_state.rotation.X = 0_deg;
        lara.m_state.rotation.Y = m_state.rotation.Y;
        lara.m_state.rotation.Z = 0_deg;
        lara.getSkeleton()->setAnim(
          gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations[1]});
        lara.setCurrentAnimState(loader::file::LaraStateId::BoulderDeath);
        lara.setGoalAnimState(loader::file::LaraStateId::BoulderDeath);
        lara.setHandStatus(HandStatus::Grabbing);
        getWorld().getPlayer().selectedWeaponType = WeaponType::None;
        getWorld().getCameraController().setModifier(CameraModifier::FollowCenter);
        getWorld().getCameraController().setRotationAroundLara(-25_deg, 170_deg);
        getWorld().useAlternativeLaraAppearance(true);
        lara.setAir(-1_frame);
        lara.m_state.health = core::DeadHealth;
        lara.m_state.is_hit = true;
      }
      require(Attack);
      break;
    default: break;
    }
  }
  else
  {
    if(m_state.current_anim_state == Think)
    {
      goal(5_as);
    }
    else
    {
      goal(1_as);
    }
  }

  rotateCreatureHead(creatureHead);
  getCreatureInfo()->neckRotation = getCreatureInfo()->headRotation;
  getSkeleton()->patchBone(11, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
  getSkeleton()->patchBone(12, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
  animateCreature(rotationToMoveTarget, 0_deg);
  m_state.collidable = true;
}

void TRex::serialize(const serialization::Serializer<world::World>& ser)
{
  AIAgent::serialize(ser);
  ser(S_NV("wantAttack", m_wantAttack));
  if(ser.loading)
    getSkeleton()->getRenderState().setScissorTest(false);
}

TRex::TRex(const std::string& name,
           const gsl::not_null<world::World*>& world,
           const gsl::not_null<const world::Room*>& room,
           const loader::file::Item& item,
           const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : AIAgent{name, world, room, item, animatedModel}
{
  getSkeleton()->getRenderState().setScissorTest(false);
}
} // namespace engine::objects
