#include "lion.h"

#include "core/angle.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/ai/ai.h"
#include "engine/items_tr1.h"
#include "engine/particle.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "util/helpers.h"

#include <gsl/gsl-lite.hpp>
#include <memory>

namespace engine::objects
{
void Lion::update()
{
  activateAi();

  core::Angle roll = 0_deg;
  core::Angle turn = 0_deg;
  core::Angle headRot = 0_deg;

  if(alive())
  {
    const ai::EnemyLocation enemyLocation{*this};
    if(enemyLocation.laraInView)
    {
      headRot = enemyLocation.visualAngleToLara;
    }
    updateMood(*this, enemyLocation, true);
    turn = rotateTowardsTarget(getCreatureInfo()->maxTurnSpeed);
    switch(m_state.current_anim_state.get())
    {
    case 1:
      if(m_state.required_anim_state != 0_as)
      {
        goal(m_state.required_anim_state);
      }
      else if(!isBored())
      {
        if(enemyLocation.laraInView && touched(0x380066UL))
          goal(7_as);
        else if(enemyLocation.laraInView && enemyLocation.distance < util::square(1_sectors))
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
      getCreatureInfo()->maxTurnSpeed = 2_deg / 1_frame;
      if(!isBored())
        goal(1_as);
      else if(util::rand15() < 128)
        goal(1_as, 6_as);
      break;
    case 3:
      roll = turn;
      getCreatureInfo()->maxTurnSpeed = 5_deg / 1_frame;
      if(isBored())
        goal(1_as); // NOLINT(bugprone-branch-clone)
      else if(enemyLocation.laraInView && enemyLocation.distance < util::square(1_sectors))
        goal(1_as);
      else if(enemyLocation.laraInView && touched(0x380066UL))
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
        getSkeleton()->setAnim(gsl::not_null{&getWorld()
                                                .getWorldGeometry()
                                                .findAnimatedModelForType(TR1ItemId::Panther)
                                                ->animations[4 + util::rand15(2)]});
      }
      else if(m_state.type == TR1ItemId::LionMale)
      {
        getSkeleton()->setAnim(gsl::not_null{&getWorld()
                                                .getWorldGeometry()
                                                .findAnimatedModelForType(TR1ItemId::LionMale)
                                                ->animations[7 + util::rand15(2)]});
      }
      else
      {
        getSkeleton()->setAnim(gsl::not_null{&getWorld()
                                                .getWorldGeometry()
                                                .findAnimatedModelForType(TR1ItemId::LionFemale)
                                                ->animations[7 + util::rand15(2)]});
      }
      m_state.current_anim_state = 5_as;
    }
  }

  rotateCreatureTilt(roll);
  rotateCreatureHead(headRot);
  getSkeleton()->patchBone(20, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
  animateCreature(turn, roll);
}
} // namespace engine::objects
