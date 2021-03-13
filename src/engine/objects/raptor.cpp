#include "raptor.h"

#include "engine/particle.h"
#include "engine/world/animation.h"
#include "engine/world/world.h"
#include "laraobject.h"

namespace engine::objects
{
void Raptor::update()
{
  activateAi();

  core::Angle animTilt = 0_deg;
  core::Angle animAngle = 0_deg;
  core::Angle animHead = 0_deg;
  if(alive())
  {
    const ai::AiInfo aiInfo{getWorld(), m_state};
    if(aiInfo.ahead)
    {
      animHead = aiInfo.angle;
    }
    updateMood(getWorld(), m_state, aiInfo, true);
    animAngle = rotateTowardsTarget(m_state.creatureInfo->maximum_turn);
    switch(m_state.current_anim_state.get())
    {
    case 1:
      if(m_state.required_anim_state != 0_as)
        goal(m_state.required_anim_state);
      else if(touched(0xff7c00UL))
        goal(8_as);
      else if(aiInfo.bite && aiInfo.distance < util::square(680_len))
        goal(8_as);
      else if(aiInfo.bite && aiInfo.distance < util::square(1536_len))
        goal(4_as);
      else if(!isBored())
        goal(3_as);
      else
        goal(2_as);
      break;
    case 2:
      m_state.creatureInfo->maximum_turn = 1_deg;
      if(!isBored())
        goal(1_as);
      else if(aiInfo.ahead && util::rand15() < 256)
        goal(1_as, 6_as);
      break;
    case 3:
      m_state.creatureInfo->maximum_turn = 4_deg;
      animTilt = animAngle;
      if(touched(0xff7c00UL))
      {
        goal(1_as);
      }
      else if(aiInfo.bite && aiInfo.distance < util::square(1536_len))
      {
        if(m_state.goal_anim_state == 3_as)
        {
          if(util::rand15() >= 8192)
            goal(7_as);
          else
            goal(1_as);
        }
      }
      else if(aiInfo.ahead && !isEscaping() && util::rand15() < 256)
        goal(1_as, 6_as);
      else if(isBored())
        goal(1_as);
      break;
    case 4:
      animTilt = animAngle;
      if(m_state.required_anim_state == 0_as)
      {
        if(aiInfo.ahead)
        {
          if(touched(0xff7c00UL))
          {
            emitParticle(core::TRVec{0_len, 66_len, 318_len}, 22, &createBloodSplat);
            hitLara(100_hp);
            require(1_as);
          }
        }
      }
      break;
    case 7:
      animTilt = animAngle;
      if(m_state.required_anim_state == 0_as && aiInfo.ahead)
      {
        if(touched(0xff7c00UL))
        {
          emitParticle(core::TRVec{0_len, 66_len, 318_len}, 22, &createBloodSplat);
          hitLara(100_hp);
          require(3_as);
        }
      }
      break;
    case 8:
      animTilt = animAngle;
      if(m_state.required_anim_state == 0_as && touched(0xff7c00UL))
      {
        emitParticle(core::TRVec{0_len, 66_len, 318_len}, 22, &createBloodSplat);
        hitLara(100_hp);
        require(1_as);
      }
      break;
    default: break;
    }
  }
  else if(m_state.current_anim_state != 5_as)
  {
    getSkeleton()->setAnim(getWorld().findAnimatedModelForType(TR1ItemId::Raptor)->animations + 9 + util::rand15(3));
    m_state.current_anim_state = 5_as;
  }

  rotateCreatureTilt(animTilt);
  rotateCreatureHead(animHead);
  getSkeleton()->patchBone(20, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix());
  animateCreature(animAngle, animTilt);
}
} // namespace engine::objects
