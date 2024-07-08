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
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "loader/file/larastateid.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/serialization.h"
#include "util/helpers.h"

#include <gsl/gsl-lite.hpp>
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

  core::Angle turn;

  core::Angle creatureHead = 0_deg;
  if(alive())
  {
    const ai::EnemyLocation enemyLocation{*this};
    if(enemyLocation.laraInView)
    {
      creatureHead = enemyLocation.visualAngleToLara;
    }
    updateMood(*this, enemyLocation, true);

    turn = rotateTowardsTarget(getCreatureInfo()->maxTurnSpeed);

    if(touched())
    {
      if(m_state.current_anim_state == RunningAttack)
        getWorld().hitLara(10_hp);
      else
        getWorld().hitLara(1_hp);
    }

    m_wantAttack = !isEscaping() && !enemyLocation.laraInView && abs(enemyLocation.visualLaraAngleToSelf) < 90_deg;
    if(!m_wantAttack && enemyLocation.distance > util::square(1500_len)
       && enemyLocation.distance < util::square(4_sectors) && enemyLocation.canAttackLara)
    {
      m_wantAttack = true;
    }

    switch(m_state.current_anim_state.get())
    {
    case Think.get():
      if(m_state.required_anim_state != 0_as)
        goal(m_state.required_anim_state);
      else if(enemyLocation.distance < util::square(1500_len) && enemyLocation.canAttackLara)
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
      else if(enemyLocation.laraInView && util::rand15() < 512)
        goal(Think, 6_as);
      break;
    case RunningAttack.get():
      getCreatureInfo()->maxTurnSpeed = 4_deg / 1_frame;
      if(enemyLocation.distance < util::square(5_sectors) && enemyLocation.canAttackLara)
        goal(Think); // NOLINT(bugprone-branch-clone)
      else if(m_wantAttack)
        goal(Think);
      else if(!isEscaping() && enemyLocation.laraInView && util::rand15() < 512)
        goal(Think, 6_as);
      else if(isBored())
        goal(Think);
      break;
    case BiteToDeath.get():
      if(touched(0x3000UL))
      {
        goal(8_as);

        auto& lara = getWorld().getObjectManager().getLara();
        lara.m_state.falling = false;

        lara.setCurrentRoom(m_state.location.room);
        lara.m_state.location.position = m_state.location.position;
        lara.m_state.rotation.X = 0_deg;
        lara.m_state.rotation.Y = m_state.rotation.Y;
        lara.m_state.rotation.Z = 0_deg;
        lara.getSkeleton()->setAnim(gsl::not_null{
          &getWorld().getWorldGeometry().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations[1]});
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
    default:
      break;
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

  rotateCreatureHead(creatureHead / 2);
  getCreatureInfo()->neckRotation = getCreatureInfo()->headRotation;
  getSkeleton()->patchBone(11, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
  getSkeleton()->patchBone(12, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
  animateCreature(turn, 0_deg);
  m_state.collidable = true;
}

void TRex::serialize(const serialization::Serializer<world::World>& ser) const
{
  AIAgent::serialize(ser);
  ser(S_NV("wantAttack", m_wantAttack));
}

void TRex::deserialize(const serialization::Deserializer<world::World>& ser)
{
  AIAgent::deserialize(ser);
  ser(S_NV("wantAttack", m_wantAttack));
}
} // namespace engine::objects
