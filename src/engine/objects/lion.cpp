#include "lion.h"

#include "engine/particle.h"
#include "laraobject.h"

namespace engine::objects
{
void Lion::update()
{
  activate();

  core::Angle tiltRot = 0_deg;
  core::Angle angle = 0_deg;
  core::Angle headRot = 0_deg;

  if(alive())
  {
    const ai::AiInfo aiInfo{getEngine(), m_state};
    if(aiInfo.ahead)
    {
      headRot = aiInfo.angle;
    }
    updateMood(getEngine(), m_state, aiInfo, true);
    angle = rotateTowardsTarget(m_state.creatureInfo->maximum_turn);
    switch(m_state.current_anim_state.get())
    {
    case 1:
      if(m_state.required_anim_state != 0_as)
      {
        goal(m_state.required_anim_state);
      }
      else if(!isBored())
      {
        if(aiInfo.ahead && touched(0x380066UL))
          goal(7_as);
        else if(aiInfo.ahead && aiInfo.distance < util::square(core::SectorSize))
          goal(4_as);
        else
          goal(3_as);
      }
      else
      {
        goal(2_as);
      }
      break;
    case 2:
      m_state.creatureInfo->maximum_turn = 2_deg;
      if(!isBored())
        goal(1_as);
      else if(util::rand15() < 128)
        goal(1_as, 6_as);
      break;
    case 3:
      tiltRot = angle;
      m_state.creatureInfo->maximum_turn = 5_deg;
      if(isBored())
        goal(1_as);
      else if(aiInfo.ahead && aiInfo.distance < util::square(core::SectorSize))
        goal(1_as);
      else if(aiInfo.ahead && touched(0x380066UL))
        goal(1_as);
      else if(!isEscaping() && util::rand15() < 128)
        goal(1_as, 6_as);
      break;
    case 4:
      if(m_state.required_anim_state == 0_as && touched(0x380066UL))
      {
        hitLara(150_hp);
        require(1_as);
      }
      break;
    case 7:
      if(m_state.required_anim_state == 0_as && touched(0x380066UL))
      {
        emitParticle({-2_len, -10_len, 132_len}, 21, &createBloodSplat);
        hitLara(250_hp);
        require(1_as);
      }
      break;
    default:
      // silence compiler
      break;
    }
  }
  else
  {
    if(m_state.current_anim_state != 5_as)
    {
      if(m_state.type == TR1ItemId::Panther)
      {
        getSkeleton()->anim
          = &getEngine().findAnimatedModelForType(TR1ItemId::Panther)->animations[4 + util::rand15(2)];
      }
      else if(m_state.type == TR1ItemId::LionMale)
      {
        getSkeleton()->anim
          = &getEngine().findAnimatedModelForType(TR1ItemId::LionMale)->animations[7 + util::rand15(2)];
        getSkeleton()->frame_number = getSkeleton()->anim->firstFrame;
        m_state.current_anim_state = 5_as;
      }
      else
      {
        getSkeleton()->anim
          = &getEngine().findAnimatedModelForType(TR1ItemId::LionFemale)->animations[7 + util::rand15(2)];
        getSkeleton()->frame_number = getSkeleton()->anim->firstFrame;
        m_state.current_anim_state = 5_as;
      }
    }
  }

  rotateCreatureTilt(tiltRot);
  rotateCreatureHead(headRot);
  getSkeleton()->patchBone(20, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix());
  animateCreature(angle, tiltRot);
}
} // namespace engine::objects
