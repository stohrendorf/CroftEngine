#include "wolf.h"

#include "aiagent.h"
#include "core/angle.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/ai/ai.h"
#include "engine/particle.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/animation.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "util/helpers.h"

#include <boost/assert.hpp>
#include <memory>
#include <utility>

namespace engine::objects
{
void Wolf::update()
{
  activateAi();

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
    const ai::EnemyLocation enemyLocation{*this};

    if(enemyLocation.enemyAhead)
    {
      pitch = enemyLocation.angleToEnemy;
    }

    updateMood(*this, enemyLocation, false);
    rotationToMoveTarget = rotateTowardsTarget(getCreatureInfo()->maxTurnSpeed);
    switch(m_state.current_anim_state.get())
    {
    case LyingDown.get():
      pitch = 0_deg;
      if(isEscaping() || enemyLocation.canReachEnemyZone())
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
      getCreatureInfo()->maxTurnSpeed = 2_deg / 1_frame;
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
        goal(Jumping); // NOLINT(bugprone-branch-clone)
      else if(enemyLocation.enemyDistance < util::square(345_len) && enemyLocation.canAttackForward)
        goal(Biting);
      else if(isStalking())
        goal(Stalking);
      else if(!isBored())
        goal(Jumping);
      else
        goal(Walking);
      break;
    case Stalking.get():
      getCreatureInfo()->maxTurnSpeed = 2_deg / 1_frame;
      if(isEscaping())
      { // NOLINT(bugprone-branch-clone)
        goal(Jumping);
      }
      else if(enemyLocation.enemyDistance < util::square(345_len) && enemyLocation.canAttackForward)
      {
        goal(Biting);
      }
      else if(enemyLocation.enemyDistance <= util::square(3 * core::SectorSize))
      {
        if(isAttacking())
        {
          if(!enemyLocation.enemyAhead || enemyLocation.enemyDistance > util::square(1.5f * core::SectorSize)
             || abs(enemyLocation.enemyAngleToSelf) < 90_deg)
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
      getCreatureInfo()->maxTurnSpeed = 5_deg / 1_frame;
      roll = rotationToMoveTarget;
      if(enemyLocation.enemyAhead && enemyLocation.enemyDistance < util::square(1.5f * core::SectorSize))
      {
        if(enemyLocation.enemyDistance <= util::square(1.5f * core::SectorSize) / 2
           || abs(enemyLocation.enemyAngleToSelf) <= 90_deg)
        {
          goal(JumpAttack, 0_as);
        }
        else
        {
          goal(PrepareToStrike, Stalking);
        }
      }
      else if(isStalking() || enemyLocation.enemyDistance >= util::square(3 * core::SectorSize))
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
      if(m_state.required_anim_state == 0_as && touched(0x774fUL) && enemyLocation.enemyAhead)
      {
        emitParticle(core::TRVec{0_len, -14_len, 174_len}, 6, &createBloodSplat);
        hitLara(100_hp);
        require(PrepareToStrike);
      }
      break;
    default:
      break;
    }
  }
  else if(m_state.current_anim_state != Dying)
  {
    const auto r = util::rand15(3);
    getSkeleton()->setAnimation(m_state.current_anim_state,
                                gsl::not_null{&getWorld().findAnimatedModelForType(m_state.type)->animations[20 + r]},
                                0_frame);
    BOOST_ASSERT(m_state.current_anim_state == Dying);
  }
  rotateCreatureTilt(roll);
  rotateCreatureHead(pitch);
  getSkeleton()->patchBone(3, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
  animateCreature(rotationToMoveTarget, roll);
}

Wolf::Wolf(const std::string& name,
           const gsl::not_null<world::World*>& world,
           const gsl::not_null<const world::Room*>& room,
           const loader::file::Item& item,
           const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : AIAgent{name, world, room, item, animatedModel}
{
  getSkeleton()->setAnim(gsl::not_null{getSkeleton()->getAnim()}, 96_frame);
  getSkeleton()->updatePose();
}
} // namespace engine::objects
