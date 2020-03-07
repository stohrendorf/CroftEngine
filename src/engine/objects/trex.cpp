#include "trex.h"

#include "engine/cameracontroller.h"
#include "laraobject.h"

namespace engine::objects
{
void TRex::update()
{
  activate();

  core::Angle rotationToMoveTarget;

  core::Angle creatureHead = 0_deg;
  if(alive())
  {
    const ai::AiInfo aiInfo{getEngine(), m_state};
    if(aiInfo.ahead)
    {
      creatureHead = aiInfo.angle;
    }
    updateMood(getEngine(), m_state, aiInfo, true);

    rotationToMoveTarget = rotateTowardsTarget(m_state.creatureInfo->maximum_turn);
    if(touched())
    {
      if(m_state.current_anim_state == 3_as)
        getEngine().getLara().m_state.health -= 10_hp;
      else
        getEngine().getLara().m_state.health -= 1_hp;
    }

    m_state.creatureInfo->flags
      = !isEscaping() && !aiInfo.ahead && aiInfo.enemy_facing > -90_deg && aiInfo.enemy_facing < 90_deg;
    if(m_state.creatureInfo->flags == 0 && aiInfo.distance > util::square(1500_len)
       && aiInfo.distance < util::square(4 * core::SectorSize) && aiInfo.bite)
    {
      m_state.creatureInfo->flags = 1;
    }

    switch(m_state.current_anim_state.get())
    {
    case 1:
      if(m_state.required_anim_state != 0_as)
        goal(m_state.required_anim_state);
      else if(aiInfo.distance < util::square(1500_len) && aiInfo.bite)
        goal(7_as);
      else if(!isBored() && m_state.creatureInfo->flags == 0)
        goal(3_as);
      else
        goal(2_as);
      break;
    case 2:
      m_state.creatureInfo->maximum_turn = 2_deg;
      if(!isBored() || m_state.creatureInfo->flags == 0)
        goal(1_as);
      else if(aiInfo.ahead && util::rand15() < 512)
        goal(1_as, 6_as);
      break;
    case 3:
      m_state.creatureInfo->maximum_turn = 4_deg;
      if(aiInfo.distance < util::square(5 * core::SectorSize) && aiInfo.bite)
        goal(1_as);
      else if(m_state.creatureInfo->flags)
        goal(1_as);
      else if(!isEscaping() && aiInfo.ahead && util::rand15() < 512)
        goal(1_as, 6_as);
      else if(isBored())
        goal(1_as);
      break;
    case 7:
      if(touched(0x3000UL))
      {
        goal(8_as);

        hitLara(1_hp);
        getEngine().getLara().m_state.falling = false;

        getEngine().getLara().setCurrentRoom(m_state.position.room);
        getEngine().getLara().m_state.position.position = m_state.position.position;
        getEngine().getLara().m_state.rotation.X = 0_deg;
        getEngine().getLara().m_state.rotation.Y = m_state.rotation.Y;
        getEngine().getLara().m_state.rotation.Z = 0_deg;
        getEngine().getLara().getSkeleton()->anim
          = &getEngine().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations[1];
        getEngine().getLara().getSkeleton()->frame_number = getEngine().getLara().getSkeleton()->anim->firstFrame;
        getEngine().getLara().setCurrentAnimState(loader::file::LaraStateId::BoulderDeath);
        getEngine().getLara().setGoalAnimState(loader::file::LaraStateId::BoulderDeath);
        getEngine().getLara().setHandStatus(HandStatus::Grabbing);
        getEngine().getLara().gunType = LaraObject::WeaponId::None;
        getEngine().getCameraController().setModifier(CameraModifier::FollowCenter);
        getEngine().getCameraController().setRotationAroundCenter(-25_deg, 170_deg);
        getEngine().getLara().setAir(-1_frame);
        getEngine().useAlternativeLaraAppearance(true);
      }
      require(2_as);
      break;
    default: break;
    }
  }
  else if(m_state.current_anim_state == 1_as)
  {
    goal(5_as);
  }
  else
  {
    goal(1_as);
  }

  rotateCreatureHead(creatureHead);
  m_state.creatureInfo->neck_rotation = m_state.creatureInfo->head_rotation;
  getSkeleton()->patchBone(11, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix());
  getSkeleton()->patchBone(12, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix());
  animateCreature(rotationToMoveTarget, 0_deg);
  m_state.collidable = true;
}
} // namespace engine::objects
