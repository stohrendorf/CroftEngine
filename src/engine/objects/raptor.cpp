#include "raptor.h"

#include "core/angle.h"
#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/ai/ai.h"
#include "engine/items_tr1.h"
#include "engine/particle.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/animation.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "util/helpers.h"

#include <memory>

namespace engine::objects
{
void Raptor::update()
{
  activateAi();

  core::Angle roll = 0_deg;
  core::Angle turn = 0_deg;
  core::Angle animHead = 0_deg;
  if(alive())
  {
    const ai::EnemyLocation enemyLocation{*this};
    if(enemyLocation.laraInView)
    {
      animHead = enemyLocation.visualAngleToLara;
    }
    updateMood(*this, enemyLocation, true);
    turn = rotateTowardsTarget(getCreatureInfo()->maxTurnSpeed);
    switch(m_state.current_anim_state.get())
    {
    case 1:
      if(m_state.required_anim_state != 0_as)
        goal(m_state.required_anim_state);
      else if(touched(0xff7c00UL))
        goal(8_as);
      else if(enemyLocation.canAttackLara && enemyLocation.distance < util::square(680_len))
        goal(8_as);
      else if(enemyLocation.canAttackLara && enemyLocation.distance < util::square(1536_len))
        goal(4_as);
      else if(!isBored())
        goal(3_as);
      else
        goal(2_as);
      break;
    case 2:
      getCreatureInfo()->maxTurnSpeed = 1_deg / 1_frame; // cppcheck-suppress duplicateExpression
      if(!isBored())
        goal(1_as);
      else if(enemyLocation.laraInView && util::rand15() < 256)
        goal(1_as, 6_as);
      break;
    case 3:
      getCreatureInfo()->maxTurnSpeed = 4_deg / 1_frame;
      roll = turn;
      if(touched(0xff7c00UL))
      {
        goal(1_as);
      }
      else if(enemyLocation.canAttackLara && enemyLocation.distance < util::square(1536_len))
      {
        if(m_state.goal_anim_state == 3_as)
        {
          if(util::rand15() >= 8192)
            goal(7_as);
          else
            goal(1_as);
        }
      }
      else if(enemyLocation.laraInView && !isEscaping() && util::rand15() < 256)
        goal(1_as, 6_as);
      else if(isBored())
        goal(1_as);
      break;
    case 4:
      roll = turn;
      if(m_state.required_anim_state == 0_as)
      {
        if(enemyLocation.laraInView)
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
      roll = turn;
      if(m_state.required_anim_state == 0_as && enemyLocation.laraInView)
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
      roll = turn;
      if(m_state.required_anim_state == 0_as && touched(0xff7c00UL))
      {
        emitParticle(core::TRVec{0_len, 66_len, 318_len}, 22, &createBloodSplat);
        hitLara(100_hp);
        require(1_as);
      }
      break;
    default:
      break;
    }
  }
  else if(m_state.current_anim_state != 5_as)
  {
    getSkeleton()->setAnim(gsl::not_null{
      &getWorld().getWorldGeometry().findAnimatedModelForType(TR1ItemId::Raptor)->animations[9 + util::rand15(2)]});
    m_state.current_anim_state = 5_as;
  }

  rotateCreatureTilt(roll);
  rotateCreatureHead(animHead);
  getSkeleton()->patchBone(20, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
  animateCreature(turn, roll);
}
} // namespace engine::objects
