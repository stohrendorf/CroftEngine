#include "larson.h"

#include "core/angle.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/ai/ai.h"
#include "engine/items_tr1.h"
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
void Larson::update()
{
  activateAi();

  core::Angle tiltRot = 0_deg;
  core::Angle turn = 0_deg;
  core::Angle headRot = 0_deg;
  if(alive())
  {
    const ai::EnemyLocation enemyLocation{*this};
    if(enemyLocation.laraInView)
    {
      headRot = enemyLocation.visualAngleToLara;
    }

    updateMood(*this, enemyLocation, false);

    turn = rotateTowardsTarget(getCreatureInfo()->maxTurnSpeed);
    switch(m_state.current_anim_state.get())
    {
    case 1: // standing holding weapon
      if(m_state.required_anim_state != 0_as)
      {
        goal(m_state.required_anim_state);
      }
      else if(isBored())
      {
        if(util::rand15() >= 96)
          goal(2_as); // walking
        else
          goal(6_as); // standing
      }
      else if(isEscaping())
      {
        goal(3_as); // running
      }
      else
      {
        goal(2_as); // walking
      }
      break;
    case 2: // walking
      getCreatureInfo()->maxTurnSpeed = 3_deg / 1_frame;
      if(isBored() && util::rand15() < 96)
        goal(1_as, 6_as);
      else if(isEscaping())
        goal(1_as, 3_as);
      else if(canShootAtLara(enemyLocation))
        goal(1_as, 4_as);
      else if(!enemyLocation.laraInView || enemyLocation.distance > util::square(3_sectors))
        goal(1_as, 3_as);
      break;
    case 3: // running
      getCreatureInfo()->maxTurnSpeed = 6_deg / 1_frame;
      tiltRot = turn / 2;
      if(isBored() && util::rand15() < 96)
        goal(1_as, 6_as);
      else if(canShootAtLara(enemyLocation))
        goal(1_as, 4_as);
      else if(enemyLocation.laraInView && enemyLocation.distance < util::square(3_sectors))
        goal(1_as, 2_as);
      break;
    case 4: // aiming
      if(m_state.required_anim_state != 0_as)
        goal(m_state.required_anim_state);
      else if(canShootAtLara(enemyLocation))
        goal(7_as);
      else
        goal(1_as);
      break;
    case 6:         // standing
      if(!isBored())
        goal(1_as); // standing/holding weapon
      else if(util::rand15() < 96)
        goal(1_as, 2_as);
      break;
    case 7: // firing
      if(m_state.required_anim_state == 0_as)
      {
        if(tryShootAtLara(*this, enemyLocation.distance, core::TRVec{-60_len, 170_len, 0_len}, 14, headRot))
          hitLara(50_hp);
        require(4_as);
      }
      if(isEscaping())
        require(1_as);
      break;
    default:
      break;
    }
  }
  else if(m_state.current_anim_state != 5_as) // injured/dying
  {
    getSkeleton()->setAnim(
      gsl::not_null{&getWorld().getWorldGeometry().findAnimatedModelForType(TR1ItemId::Larson)->animations[15]});
    m_state.current_anim_state = 5_as;
  }
  rotateCreatureTilt(tiltRot);
  rotateCreatureHead(headRot);
  getSkeleton()->patchBone(7, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
  animateCreature(turn, 0_deg);
}
} // namespace engine::objects
