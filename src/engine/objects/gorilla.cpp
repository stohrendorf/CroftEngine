
#include "gorilla.h"

#include "aiagent.h"
#include "core/angle.h"
#include "core/genericvec.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/ai/ai.h"
#include "engine/items_tr1.h"
#include "engine/location.h"
#include "engine/particle.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/animation.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/serialization.h"
#include "util/helpers.h"

#include <boost/assert.hpp>
#include <exception>
#include <memory>

namespace engine::objects
{
void Gorilla::update()
{
  activateAi();

  core::Angle headRot = 0_deg;
  core::Angle turn = 0_deg;

  if(alive())
  {
    const ai::EnemyLocation enemyLocation{*this};
    if(enemyLocation.laraInView)
      headRot = enemyLocation.visualAngleToLara;
    updateMood(*this, enemyLocation, false);

    turn = rotateTowardsTarget(getCreatureInfo()->maxTurnSpeed);
    if(m_state.is_hit || enemyLocation.distance < util::square(2_sectors))
    {
      m_wantAttack = true;
    }
    switch(m_state.current_anim_state.get())
    {
    case 1:
      // standing
      if(m_crabWalkRight)
      {
        m_state.rotation.Y -= 90_deg;
        m_crabWalkRight = false;
      }
      else if(m_crabWalkLeft)
      {
        m_state.rotation.Y += 90_deg;
        m_crabWalkLeft = false;
      }
      if(m_state.required_anim_state != 0_as)
      {
        goal(m_state.required_anim_state);
      }
      else if(enemyLocation.canAttackLara && enemyLocation.distance < util::square(430_len))
      {
        // attack
        goal(4_as);
      }
      else if(m_wantAttack || !enemyLocation.canReachLara() || !enemyLocation.laraInView)
      {
        // run on legs and arms
        goal(3_as);
      }
      else
      {
        const auto r = util::rand15(1024);
        if(r < 160)
        {
          // jump and wave arms
          goal(10_as);
        }
        else if(r < 320)
        {
          // thump chest
          goal(6_as);
        }
        else if(r < 480)
        {
          // wave arms
          goal(7_as);
        }
        else if(r < 752)
        {
          // crab walk right
          goal(8_as);
          getCreatureInfo()->maxTurnSpeed = 0_deg / 1_frame;
        }
        else
        {
          // crab walk left
          goal(9_as);
          getCreatureInfo()->maxTurnSpeed = 0_deg / 1_frame;
        }
      }
      break;
    case 3:
      // running
      getCreatureInfo()->maxTurnSpeed = 5_deg / 1_frame;
      if(!m_wantAttack && !m_crabWalkRight && !m_crabWalkLeft && abs(enemyLocation.visualAngleToLara) < 45_deg)
      {
        goal(1_as);
      }
      else if(enemyLocation.laraInView && touched(0xff00))
      {
        goal(1_as, 4_as);
      }
      else if(!isEscaping())
      {
        // random "rage" animation
        const auto r = util::rand15();
        if(r < 160)
          goal(1_as, 10_as);
        else if(r < 320)
          goal(1_as, 6_as);
        else if(r < 480)
          goal(1_as, 7_as);
      }
      break;
    case 4:
      // attacking
      if(m_state.required_anim_state == 0_as && touched(0xff00))
      {
        emitParticle({0_len, -19_len, 75_len}, 15, &createBloodSplat);
        hitLara(200_hp);
        require(1_as);
      }
      break;
    case 8:
      BOOST_ASSERT(!m_crabWalkRight);
      if(!m_crabWalkLeft)
      {
        m_state.rotation.Y -= 90_deg;
        m_crabWalkLeft = true;
      }
      goal(1_as);
      break;
    case 9:
      BOOST_ASSERT(!m_crabWalkLeft);
      if(!m_crabWalkRight)
      {
        m_state.rotation.Y += 90_deg;
        m_crabWalkRight = true;
      }
      goal(1_as);
      break;
    default:
      break;
    }
  }
  else if(m_state.current_anim_state != 5_as)
  {
    // die
    getSkeleton()->setAnim(
      gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::Gorilla)->animations[7 + util::rand15(2)]});
    m_state.current_anim_state = 5_as;
  }
  rotateCreatureHead(headRot);
  if(m_state.current_anim_state == 11_as)
  {
    // climbing
    getSkeleton()->patchBone(14, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
    animateCreature(turn, 0_deg);
    return;
  }

  if(m_crabWalkRight)
  {
    m_state.rotation.Y -= 90_deg;
    m_crabWalkRight = false;
  }
  else if(m_crabWalkLeft)
  {
    m_state.rotation.Y += 90_deg;
    m_crabWalkLeft = false;
  }
  const auto old = m_state.location.position;
  getSkeleton()->patchBone(14, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
  animateCreature(turn, 0_deg);
  if(old.Y - 384_len < m_state.location.position.Y)
    return;

  // rotate for climbing
  const auto xSectorOld = sectorOf(old.X);
  const auto zSectorOld = sectorOf(old.Z);
  const auto xSectorNew = sectorOf(m_state.location.position.X);
  const auto zSectorNew = sectorOf(m_state.location.position.Z);
  if(zSectorOld == zSectorNew)
  {
    if(xSectorOld == xSectorNew)
    {
      return;
    }
    if(xSectorOld >= xSectorNew)
    {
      m_state.rotation.Y = -90_deg;
      m_state.location.position.X = xSectorOld * 1_sectors + 75_len;
    }
    else
    {
      m_state.rotation.Y = 90_deg;
      m_state.location.position.X = xSectorNew * 1_sectors - 75_len;
    }
  }
  else if(xSectorOld == xSectorNew)
  {
    if(zSectorOld >= zSectorNew)
    {
      m_state.rotation.Y = -180_deg;
      m_state.location.position.Z = zSectorOld * 1_sectors + 75_len;
    }
    else
    {
      m_state.rotation.Y = 0_deg;
      m_state.location.position.Z = zSectorNew * 1_sectors - 75_len;
    }
  }

  m_state.location.position.Y = old.Y;
  getSkeleton()->setAnim(gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::Gorilla)->animations[19]});
  m_state.current_anim_state = 11_as;
}

void Gorilla::serialize(const serialization::Serializer<world::World>& ser) const
{
  AIAgent::serialize(ser);
  ser(S_NV("wantAttack", m_wantAttack), S_NV("turnedRight", m_crabWalkRight), S_NV("turnedLeft", m_crabWalkLeft));
}

void Gorilla::deserialize(const serialization::Deserializer<world::World>& ser)
{
  AIAgent::deserialize(ser);
  ser(S_NV("wantAttack", m_wantAttack), S_NV("turnedRight", m_crabWalkRight), S_NV("turnedLeft", m_crabWalkLeft));
}
} // namespace engine::objects
