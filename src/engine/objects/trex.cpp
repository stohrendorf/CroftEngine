#include "trex.h"

#include "engine/cameracontroller.h"
#include "engine/player.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "serialization/serialization.h"

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
    const ai::AiInfo aiInfo{getWorld(), m_state};
    if(aiInfo.ahead)
    {
      creatureHead = aiInfo.angle;
    }
    updateMood(getWorld(), m_state, aiInfo, true);

    rotationToMoveTarget = rotateTowardsTarget(m_state.creatureInfo->maximum_turn);
    if(touched())
    {
      if(m_state.current_anim_state == RunningAttack)
        getWorld().getObjectManager().getLara().m_state.health -= 10_hp;
      else
        getWorld().getObjectManager().getLara().m_state.health -= 1_hp;
    }

    m_wantAttack = !isEscaping() && !aiInfo.ahead && abs(aiInfo.enemy_facing) < 90_deg;
    if(!m_wantAttack && aiInfo.distance > util::square(1500_len) && aiInfo.distance < util::square(4 * core::SectorSize)
       && aiInfo.bite)
    {
      m_wantAttack = true;
    }

    switch(m_state.current_anim_state.get())
    {
    case Think.get():
      if(m_state.required_anim_state != 0_as)
        goal(m_state.required_anim_state);
      else if(aiInfo.distance < util::square(1500_len) && aiInfo.bite)
        goal(BiteToDeath);
      else if(isBored() || m_wantAttack)
        goal(Attack);
      else
        goal(RunningAttack);
      break;
    case Attack.get():
      m_state.creatureInfo->maximum_turn = 2_deg;
      if(!isBored() || !m_wantAttack)
        goal(Think);
      else if(aiInfo.ahead && util::rand15() < 512)
        goal(Think, 6_as);
      break;
    case RunningAttack.get():
      m_state.creatureInfo->maximum_turn = 4_deg;
      if(aiInfo.distance < util::square(5 * core::SectorSize) && aiInfo.bite)
        goal(Think); // NOLINT(bugprone-branch-clone)
      else if(m_wantAttack)
        goal(Think);
      else if(!isEscaping() && aiInfo.ahead && util::rand15() < 512)
        goal(Think, 6_as);
      else if(isBored())
        goal(Think);
      break;
    case BiteToDeath.get():
      if(touched(0x3000UL))
      {
        goal(8_as);

        hitLara(1_hp);
        getWorld().getObjectManager().getLara().m_state.falling = false;

        getWorld().getObjectManager().getLara().setCurrentRoom(m_state.position.room);
        getWorld().getObjectManager().getLara().m_state.position.position = m_state.position.position;
        getWorld().getObjectManager().getLara().m_state.rotation.X = 0_deg;
        getWorld().getObjectManager().getLara().m_state.rotation.Y = m_state.rotation.Y;
        getWorld().getObjectManager().getLara().m_state.rotation.Z = 0_deg;
        getWorld().getObjectManager().getLara().getSkeleton()->setAnim(
          &getWorld().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations[1]);
        getWorld().getObjectManager().getLara().setCurrentAnimState(loader::file::LaraStateId::BoulderDeath);
        getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::BoulderDeath);
        getWorld().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
        getWorld().getPlayer().gunType = WeaponId::None;
        getWorld().getCameraController().setModifier(CameraModifier::FollowCenter);
        getWorld().getCameraController().setRotationAroundLara(-25_deg, 170_deg);
        getWorld().getObjectManager().getLara().setAir(-1_frame);
        getWorld().useAlternativeLaraAppearance(true);
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
  m_state.creatureInfo->neck_rotation = m_state.creatureInfo->head_rotation;
  getSkeleton()->patchBone(11, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix());
  getSkeleton()->patchBone(12, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix());
  animateCreature(rotationToMoveTarget, 0_deg);
  m_state.collidable = true;
}

void TRex::serialize(const serialization::Serializer<world::World>& ser)
{
  AIAgent::serialize(ser);
  ser(S_NV("wantAttack", m_wantAttack));
}
} // namespace engine::objects
