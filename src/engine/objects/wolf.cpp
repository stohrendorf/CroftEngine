#include "wolf.h"

#include "engine/particle.h"
#include "laraobject.h"

namespace engine::objects
{
void Wolf::update()
{
  activate();

  static constexpr auto Walking = 1_as;
  static constexpr auto Running = 2_as;
  static constexpr auto Jumping = 3_as;
  static constexpr auto Stalking = 5_as;
  static constexpr auto JumpAttack = 6_as;
  static constexpr auto Attacking = 7_as;
  static constexpr auto LyingDown = 8_as;
  static constexpr auto PrepareToStrike = 9_as;
  // static constexpr const uint16_t RunningJump = 10;
  static constexpr auto Dying = 11_as;
  static constexpr auto Biting = 12_as;

  core::Angle pitch = 0_deg;
  core::Angle roll = 0_deg;
  core::Angle rotationToMoveTarget = 0_deg;
  if(alive())
  {
    const ai::AiInfo aiInfo{getEngine(), m_state};

    if(aiInfo.ahead)
    {
      pitch = aiInfo.angle;
    }

    updateMood(getEngine(), m_state, aiInfo, false);
    rotationToMoveTarget = rotateTowardsTarget(m_state.creatureInfo->maximum_turn);
    switch(m_state.current_anim_state.get())
    {
    case LyingDown.get():
      pitch = 0_deg;
      if(isEscaping() || aiInfo.canReachEnemyZone())
        goal(Walking, PrepareToStrike);
      else if(util::rand15() < 32)
        goal(Walking, Running);
      break;
    case Walking.get():
      if(m_state.required_anim_state != 0_as)
        goal(std::exchange(m_state.required_anim_state, 0_as));
      else
        goal(Running);
      break;
    case Running.get():
      m_state.creatureInfo->maximum_turn = 2_deg;
      if(!isBored())
        goal(Stalking, 0_as);
      else if(util::rand15() < 32)
        goal(Walking, LyingDown);
      break;
    case PrepareToStrike.get():
      if(m_state.required_anim_state != 0_as)
      {
        goal(std::exchange(m_state.required_anim_state, 0_as));
        break;
      }
      if(isEscaping())
        goal(Jumping);
      else if(aiInfo.distance < util::square(345_len) && aiInfo.bite)
        goal(Biting);
      else if(isStalking())
        goal(Stalking);
      else if(!isBored())
        goal(Jumping);
      else
        goal(Walking);
      break;
    case Stalking.get():
      m_state.creatureInfo->maximum_turn = 2_deg;
      if(isEscaping())
      {
        goal(Jumping);
      }
      else if(aiInfo.distance < util::square(345_len) && aiInfo.bite)
      {
        goal(Biting);
      }
      else if(aiInfo.distance <= util::square(3 * core::SectorSize))
      {
        if(isAttacking())
        {
          if(!aiInfo.ahead || aiInfo.distance > util::square(3 * core::SectorSize / 2)
             || (aiInfo.enemy_facing < 90_deg && aiInfo.enemy_facing > -90_deg))
          {
            goal(Jumping);
          }
        }
        else if(util::rand15() >= 384)
        {
          if(isBored())
            goal(PrepareToStrike);
        }
        else
        {
          goal(PrepareToStrike, Attacking);
        }
      }
      else
      {
        goal(Jumping);
      }
      break;
    case Jumping.get():
      m_state.creatureInfo->maximum_turn = 5_deg;
      roll = rotationToMoveTarget;
      if(aiInfo.ahead && aiInfo.distance < util::square(3 * core::SectorSize / 2))
      {
        if(aiInfo.distance <= util::square(3 * core::SectorSize / 2) / 2
           || (aiInfo.enemy_facing <= 90_deg && aiInfo.enemy_facing >= -90_deg))
        {
          goal(JumpAttack, 0_as);
        }
        else
        {
          goal(PrepareToStrike, Stalking);
        }
      }
      else if(isStalking() || aiInfo.distance >= util::square(3 * core::SectorSize))
      {
        if(isBored())
          goal(PrepareToStrike);
      }
      else
      {
        goal(PrepareToStrike, Stalking);
      }
      break;
    case JumpAttack.get():
      roll = rotationToMoveTarget;
      if(m_state.required_anim_state == 0_as && touched(0x774fUL))
      {
        emitParticle(core::TRVec{0_len, -14_len, 174_len}, 6, &createBloodSplat);
        hitLara(50_hp);
        require(Jumping);
      }
      goal(Jumping);
      break;
    case Biting.get():
      if(m_state.required_anim_state == 0_as && touched(0x774fUL) && aiInfo.ahead)
      {
        emitParticle(core::TRVec{0_len, -14_len, 174_len}, 6, &createBloodSplat);
        hitLara(100_hp);
        require(PrepareToStrike);
      }
      break;
    default: break;
    }
  }
  else if(m_state.current_anim_state != Dying)
  {
    const auto r = util::rand15(3);
    getSkeleton()->setAnimation(
      m_state.current_anim_state, &getEngine().findAnimatedModelForType(m_state.type)->animations[20 + r], 0_frame);
    BOOST_ASSERT(m_state.current_anim_state == Dying);
  }
  rotateCreatureTilt(roll);
  rotateCreatureHead(pitch);
  getSkeleton()->patchBone(3, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix());
  animateCreature(rotationToMoveTarget, roll);
}
} // namespace engine::objects
