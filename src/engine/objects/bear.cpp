#include "bear.h"

#include "engine/particle.h"
#include "engine/world.h"
#include "laraobject.h"

namespace engine::objects
{
void Bear::update()
{
  activateAi();

  core::Angle rotationToMoveTarget;

  static constexpr auto Walking = 0_as;
  static constexpr auto GettingDown = 1_as;
  static constexpr auto WalkingTall = 2_as;
  static constexpr auto Running = 3_as;
  static constexpr auto RoaringStanding = 4_as;
  static constexpr auto Growling = 5_as;
  static constexpr auto RunningAttack = 6_as;
  static constexpr auto Standing = 7_as;
  static constexpr auto Biting = 8_as;
  static constexpr auto Dying = 9_as;

  if(alive())
  {
    const ai::AiInfo aiInfo{getWorld(), m_state};
    updateMood(getWorld(), m_state, aiInfo, true);

    rotationToMoveTarget = rotateTowardsTarget(m_state.creatureInfo->maximum_turn);
    if(m_state.is_hit)
      m_state.creatureInfo->flags = 1;

    switch(m_state.current_anim_state.get())
    {
    case Walking.get():
      m_state.creatureInfo->maximum_turn = 2_deg;
      if(getWorld().getObjectManager().getLara().m_state.health <= 0_hp && touched(0x2406cUL) && aiInfo.ahead)
      {
        goal(GettingDown);
      }
      else if(!isBored())
      {
        goal(GettingDown);
        if(isEscaping())
          require(0_as);
      }
      else if(util::rand15() < 80)
      {
        goal(GettingDown, Growling);
      }
      break;
    case GettingDown.get():
      if(getWorld().getObjectManager().getLara().m_state.health <= 0_hp)
      {
        if(aiInfo.bite && aiInfo.distance < util::square(768_len))
          goal(Biting);
        else
          goal(Walking);
      }
      else
      {
        if(m_state.required_anim_state != 0_as)
          goal(m_state.required_anim_state);
        else if(!isBored())
          goal(Running);
        else
          goal(Walking);
      }
      break;
    case WalkingTall.get():
      if(m_state.creatureInfo->flags != 0)
        goal(RoaringStanding, 0_as); // NOLINT(bugprone-branch-clone)
      else if(aiInfo.ahead && touched(0x2406cUL))
        goal(RoaringStanding);
      else if(isEscaping())
        goal(RoaringStanding, 0_as);
      else if(isBored() || util::rand15() < 80)
        goal(RoaringStanding, Growling);
      else if(aiInfo.distance > util::square(2 * core::SectorSize) || util::rand15() < 1536)
        goal(RoaringStanding, GettingDown);
      break;
    case Running.get():
      m_state.creatureInfo->maximum_turn = 5_deg;
      if(touched(0x2406cUL))
      {
        hitLara(3_hp);
      }
      if(isBored() || getWorld().getObjectManager().getLara().m_state.health <= 0_hp)
      {
        goal(GettingDown);
      }
      else if(aiInfo.ahead && m_state.required_anim_state == 0_as)
      {
        if(m_state.creatureInfo->flags == 0 && aiInfo.distance < util::square(2048_len) && util::rand15() < 768)
          goal(GettingDown, RoaringStanding);
        else if(aiInfo.distance < util::square(core::SectorSize))
          goal(RunningAttack);
      }
      break;
    case RoaringStanding.get():
      if(m_state.creatureInfo->flags != 0)
        goal(GettingDown, 0_as);
      else if(m_state.required_anim_state != 0_as)
        goal(m_state.required_anim_state);
      else if(isBored() || isEscaping())
        goal(GettingDown);
      else if(aiInfo.bite && aiInfo.distance < util::square(600_len))
        goal(Standing);
      else
        goal(WalkingTall);
      break;
    case RunningAttack.get():
      if(m_state.required_anim_state == 0_as && touched(0x2406cUL))
      {
        emitParticle(core::TRVec{0_len, 96_len, 335_len}, 14, &createBloodSplat);
        hitLara(200_hp);
        require(GettingDown);
      }
      break;
    case Standing.get():
      if(m_state.required_anim_state == 0_as && touched(0x2406cUL))
      {
        hitLara(400_hp);
        require(RoaringStanding);
      }
      break;
    default: break;
    }
    rotateCreatureHead(aiInfo.angle);
  }
  else
  {
    rotationToMoveTarget = rotateTowardsTarget(1_deg);
    switch(m_state.current_anim_state.get())
    {
    case Walking.get():
    case Running.get(): goal(GettingDown); break;
    case GettingDown.get():
      m_state.creatureInfo->flags = 0;
      goal(Dying);
      break;
    case WalkingTall.get(): goal(RoaringStanding); break;
    case RoaringStanding.get():
      m_state.creatureInfo->flags = 1;
      goal(Dying);
      break;
    case Dying.get():
      if(m_state.creatureInfo->flags != 0 && touched(0x2406cUL))
      {
        hitLara(200_hp);
        m_state.creatureInfo->flags = 0;
      }
      break;
    default: break;
    }
    rotateCreatureHead(0_deg);
  }
  getSkeleton()->patchBone(14, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix());
  animateCreature(rotationToMoveTarget, 0_deg);
}
} // namespace engine::objects
