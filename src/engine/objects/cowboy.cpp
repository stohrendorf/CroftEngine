#include "cowboy.h"

#include "engine/particle.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"

namespace engine::objects
{
void Cowboy::update()
{
  activateAi();

  core::Angle tiltRot = 0_deg;
  core::Angle creatureTurn = 0_deg;
  core::Angle headRot = 0_deg;
  if(alive())
  {
    ai::EnemyLocation enemyLocation{*this};
    if(enemyLocation.enemyAhead)
    {
      headRot = enemyLocation.angleToEnemy;
    }

    updateMood(*this, enemyLocation, false);

    creatureTurn = rotateTowardsTarget(getCreatureInfo()->maxTurnSpeed);
    switch(m_state.current_anim_state.get())
    {
    case 1:
      if(m_state.required_anim_state != 0_as)
      {
        goal(m_state.required_anim_state);
      }
      else if(canShootAtLara(enemyLocation))
      {
        goal(4_as);
      }
      else if(getCreatureInfo()->mood != ai::Mood::Bored)
      {
        goal(3_as);
      }
      else
      {
        goal(2_as);
      }
      break;
    case 2:
      getCreatureInfo()->maxTurnSpeed = 3_deg / 1_frame;
      if(getCreatureInfo()->mood == ai::Mood::Escape || !enemyLocation.enemyAhead)
        goal(1_as, 3_as);
      else if(canShootAtLara(enemyLocation))
        goal(1_as, 4_as);
      else if(enemyLocation.enemyDistance > util::square(3072_len))
        goal(1_as, 3_as);
      break;
    case 3:
      getCreatureInfo()->maxTurnSpeed = 6_deg / 1_frame;
      tiltRot = creatureTurn / 2;
      if(getCreatureInfo()->mood != ai::Mood::Escape || enemyLocation.enemyAhead)
      {
        if(canShootAtLara(enemyLocation))
          goal(1_as, 4_as);
        else if(enemyLocation.enemyAhead && enemyLocation.enemyDistance < util::square(3072_len))
          goal(1_as, 2_as);
      }
      break;
    case 4:
      m_aimTime = 0_frame;
      if(m_state.required_anim_state != 0_as || !canShootAtLara(enemyLocation))
      {
        goal(1_as);
      }
      else
      {
        goal(6_as);
      }
      break;
    case 6:
      if(m_aimTime == 0_frame)
      {
        if(tryShootAtLara(*this, enemyLocation.enemyDistance, {1_len, 200_len, 41_len}, 5, headRot))
        {
          hitLara(70_hp);
        }
      }
      else if(m_aimTime == 6_frame)
      {
        if(canShootAtLara(enemyLocation))
        {
          if(tryShootAtLara(*this, enemyLocation.enemyDistance, {-2_len, 200_len, 40_len}, 8, headRot))
          {
            hitLara(70_hp);
          }
        }
        else
        {
          auto p = emitParticle({-2_len, 200_len, 40_len}, 8, &createMuzzleFlash);
          p->angle.Y += headRot;
        }
      }
      m_aimTime += 1_frame;
      if(getCreatureInfo()->mood == ai::Mood::Escape)
      {
        require(3_as);
      }
      break;
    default: break;
    }
  }
  else if(m_state.current_anim_state != 5_as) // injured/dying
  {
    getSkeleton()->setAnim(&getWorld().findAnimatedModelForType(TR1ItemId::Cowboy)->animations[7]);
    m_state.current_anim_state = 5_as;
    getWorld().createPickup(TR1ItemId::MagnumsSprite, m_state.location.room, m_state.location.position);
  }
  rotateCreatureTilt(tiltRot);
  rotateCreatureHead(headRot);
  animateCreature(creatureTurn, 0_deg);
  if(getCreatureInfo() != nullptr)
    getSkeleton()->patchBone(0, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
}

void Cowboy::serialize(const serialization::Serializer<world::World>& ser)
{
  AIAgent::serialize(ser);
  ser(S_NV("aimTime", m_aimTime));
}
} // namespace engine::objects
