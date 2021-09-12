#include "gorilla.h"

#include "engine/particle.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/world.h"
#include "serialization/serialization.h"

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
    if(enemyLocation.enemyAhead)
      headRot = enemyLocation.angleToEnemy;
    updateMood(*this, enemyLocation, false);

    turn = rotateTowardsTarget(getCreatureInfo()->maxTurnSpeed);
    if(m_state.is_hit || enemyLocation.enemyDistance < util::square(2 * core::SectorSize))
    {
      m_wantAttack = true;
    }
    switch(m_state.current_anim_state.get())
    {
    case 1:
      // standing
      if(m_turnedRight)
      {
        m_state.rotation.Y -= 90_deg;
        m_turnedRight = false;
      }
      else if(m_turnedLeft)
      {
        m_state.rotation.Y += 90_deg;
        m_turnedLeft = false;
      }
      if(m_state.required_anim_state != 0_as)
      {
        goal(m_state.required_anim_state);
      }
      else if(enemyLocation.canAttackForward && enemyLocation.enemyDistance < util::square(430_len))
      {
        goal(4_as);
      }
      else if(m_wantAttack || !enemyLocation.canReachEnemyZone() || !enemyLocation.enemyAhead)
      {
        goal(3_as);
      }
      else
      {
        const auto r = util::rand15(1024);
        if(r < 160)
        {
          goal(10_as);
        }
        else if(r < 320)
        {
          goal(6_as);
        }
        else if(r < 480)
        {
          goal(7_as);
        }
        else if(r < 752)
        {
          goal(8_as);
          getCreatureInfo()->maxTurnSpeed = 0_deg / 1_frame;
        }
        else
        {
          goal(9_as);
          getCreatureInfo()->maxTurnSpeed = 0_deg / 1_frame;
        }
      }
      break;
    case 3:
      // running
      getCreatureInfo()->maxTurnSpeed = 5_deg / 1_frame;
      if(!m_wantAttack && !m_turnedRight && !m_turnedLeft && enemyLocation.angleToEnemy > -45_deg
         && enemyLocation.angleToEnemy < 45_deg)
      {
        goal(1_as);
      }
      else if(enemyLocation.enemyAhead && touched(0xff00))
      {
        goal(1_as, 4_as);
      }
      else if(!isEscaping())
      {
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
      // turn left
      if(!m_turnedLeft)
      {
        m_state.rotation.Y -= 90_deg;
        m_turnedLeft = true;
      }
      goal(1_as);
      break;
    case 9:
      // turn right
      if(!m_turnedRight)
      {
        m_state.rotation.Y += 90_deg;
        m_turnedRight = true;
      }
      goal(1_as);
      break;
    default: break;
    }
  }
  else if(m_state.current_anim_state != 5_as)
  {
    getSkeleton()->setAnim(&getWorld().findAnimatedModelForType(TR1ItemId::Gorilla)->animations[7 + util::rand15(2)]);
    m_state.current_anim_state = 5_as;
  }
  rotateCreatureHead(headRot);
  if(m_state.current_anim_state == 11_as)
  {
    // climbing
    getSkeleton()->patchBone(14, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
    animateCreature(turn, 0_deg);
  }
  else
  {
    if(m_turnedRight)
    {
      m_state.rotation.Y -= 90_deg;
      m_turnedRight = false;
    }
    else if(m_turnedLeft)
    {
      m_state.rotation.Y += 90_deg;
      m_turnedLeft = false;
    }
    const auto old = m_state.location.position;
    getSkeleton()->patchBone(14, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
    animateCreature(turn, 0_deg);
    if(old.Y - 384_len < m_state.location.position.Y)
      return;

    const auto xSectorOld = old.X / core::SectorSize;
    const auto zSectorOld = old.Z / core::SectorSize;
    const auto xSectorNew = m_state.location.position.X / core::SectorSize;
    const auto zSectorNew = m_state.location.position.Z / core::SectorSize;
    if(zSectorOld == zSectorNew)
    {
      if(xSectorOld == xSectorNew)
      {
        return;
      }
      if(xSectorOld >= xSectorNew)
      {
        m_state.rotation.Y = -90_deg;
        m_state.location.position.X = xSectorOld * core::SectorSize + 75_len;
      }
      else
      {
        m_state.rotation.Y = 90_deg;
        m_state.location.position.X = xSectorNew * core::SectorSize - 75_len;
      }
    }
    else if(xSectorOld == xSectorNew)
    {
      if(zSectorOld >= zSectorNew)
      {
        m_state.rotation.Y = -180_deg;
        m_state.location.position.Z = zSectorOld * core::SectorSize + 75_len;
      }
      else
      {
        m_state.rotation.Y = 0_deg;
        m_state.location.position.Z = zSectorNew * core::SectorSize - 75_len;
      }
    }

    m_state.location.position.Y = old.Y;
    getSkeleton()->setAnim(&getWorld().findAnimatedModelForType(TR1ItemId::Gorilla)->animations[19]);
    m_state.current_anim_state = 11_as;
  }
}

void Gorilla::serialize(const serialization::Serializer<world::World>& ser)
{
  AIAgent::serialize(ser);
  ser(S_NV("wantAttack", m_wantAttack), S_NV("turnedRight", m_turnedRight), S_NV("turnedLeft", m_turnedLeft));
}
} // namespace engine::objects
