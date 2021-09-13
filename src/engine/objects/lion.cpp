#include "lion.h"

#include "engine/particle.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"

namespace engine::objects
{
void Lion::update()
{
  activateAi();

  core::Angle tiltRot = 0_deg;
  core::Angle angle = 0_deg;
  core::Angle headRot = 0_deg;

  if(alive())
  {
    const ai::EnemyLocation enemyLocation{*this};
    if(enemyLocation.enemyAhead)
    {
      headRot = enemyLocation.angleToEnemy;
    }
    updateMood(*this, enemyLocation, true);
    angle = rotateTowardsTarget(getCreatureInfo()->maxTurnSpeed);
    switch(m_state.current_anim_state.get())
    {
    case 1:
      if(m_state.required_anim_state != 0_as)
      {
        goal(m_state.required_anim_state);
      }
      else if(!isBored())
      {
        if(enemyLocation.enemyAhead && touched(0x380066UL))
          goal(7_as);
        else if(enemyLocation.enemyAhead && enemyLocation.enemyDistance < util::square(core::SectorSize))
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
      tiltRot = angle;
      getCreatureInfo()->maxTurnSpeed = 5_deg / 1_frame;
      if(isBored())
        goal(1_as); // NOLINT(bugprone-branch-clone)
      else if(enemyLocation.enemyAhead && enemyLocation.enemyDistance < util::square(core::SectorSize))
        goal(1_as);
      else if(enemyLocation.enemyAhead && touched(0x380066UL))
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
        getSkeleton()->setAnim(
          &getWorld().findAnimatedModelForType(TR1ItemId::Panther)->animations[4 + util::rand15(2)]);
      }
      else if(m_state.type == TR1ItemId::LionMale)
      {
        getSkeleton()->setAnim(
          &getWorld().findAnimatedModelForType(TR1ItemId::LionMale)->animations[7 + util::rand15(2)]);
      }
      else
      {
        getSkeleton()->setAnim(
          &getWorld().findAnimatedModelForType(TR1ItemId::LionFemale)->animations[7 + util::rand15(2)]);
      }
      m_state.current_anim_state = 5_as;
    }
  }

  rotateCreatureTilt(tiltRot);
  rotateCreatureHead(headRot);
  getSkeleton()->patchBone(20, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
  animateCreature(angle, tiltRot);
}
} // namespace engine::objects
