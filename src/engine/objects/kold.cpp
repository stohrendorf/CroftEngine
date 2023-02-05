#include "kold.h"

#include "aiagent.h"
#include "core/angle.h"
#include "core/units.h"
#include "engine/ai/ai.h"
#include "engine/items_tr1.h"
#include "engine/location.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/animation.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/serialization.h"
#include "util/helpers.h"

#include <exception>
#include <memory>

namespace engine::objects
{
void Kold::update()
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

    updateMood(*this, enemyLocation, true);

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
      else if(enemyLocation.enemyDistance > util::square(4096_len))
        goal(1_as, 3_as);
      break;
    case 3:
      getCreatureInfo()->maxTurnSpeed = 6_deg / 1_frame;
      tiltRot = creatureTurn / 2;
      if(getCreatureInfo()->mood != ai::Mood::Escape || enemyLocation.enemyAhead)
      {
        if(canShootAtLara(enemyLocation))
          goal(1_as, 4_as);
        else if(enemyLocation.enemyAhead && enemyLocation.enemyDistance < util::square(4096_len))
          goal(1_as, 2_as);
      }
      break;
    case 4:
      m_shotAtLara = false;
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
      if(!m_shotAtLara)
      {
        if(tryShootAtLara(*this, enemyLocation.enemyDistance / 2, {-20_len, 440_len, 20_len}, 9, headRot))
        {
          hitLara(150_hp);
        }
        m_shotAtLara = true;
      }
      if(getCreatureInfo()->mood == ai::Mood::Escape)
      {
        require(3_as);
      }
      break;
    default:
      break;
    }
  }
  else if(m_state.current_anim_state != 5_as) // injured/dying
  {
    getSkeleton()->setAnim(gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::Kold)->animations[14]});
    m_state.current_anim_state = 5_as;
    getWorld().createPickup(TR1ItemId::ShotgunSprite, m_state.location.room, m_state.location.position);
  }
  rotateCreatureTilt(tiltRot);
  rotateCreatureHead(headRot);
  animateCreature(creatureTurn, 0_deg);
  if(getCreatureInfo() != nullptr)
    getSkeleton()->patchBone(0, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
}

void Kold::serialize(const serialization::Serializer<world::World>& ser) const
{
  AIAgent::serialize(ser);
  ser(S_NV("shotAtLara", m_shotAtLara));
}

void Kold::deserialize(const serialization::Deserializer<world::World>& ser)
{
  AIAgent::deserialize(ser);
  ser(S_NV("shotAtLara", m_shotAtLara));
}
} // namespace engine::objects
