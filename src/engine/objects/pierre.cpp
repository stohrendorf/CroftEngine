#include "pierre.h"

#include "aiagent.h"
#include "core/angle.h"
#include "core/genericvec.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/ai/ai.h"
#include "engine/cameracontroller.h"
#include "engine/floordata/floordata.h"
#include "engine/items_tr1.h"
#include "engine/location.h"
#include "engine/raycast.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/animation.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "object.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "util/helpers.h"

#include <cstdint>
#include <exception>
#include <map>
#include <memory>
#include <optional>

namespace engine::objects
{
void Pierre::update()
{
  if(getWorld().getPierre() == nullptr)
  {
    getWorld().setPierre(this);
  }
  else if(this != getWorld().getPierre().get())
  {
    if(m_state.activationState.isOneshot())
    {
      if(getWorld().getPierre() != nullptr)
        getWorld().getPierre()->kill();

      getWorld().setPierre(nullptr);
    }
    else
    {
      kill();
    }
  }

  activateAi();

  core::Angle tiltRot = 0_deg;
  core::Angle turn = 0_deg;
  core::Angle headRot = 0_deg;
  if(m_state.health <= 40_hp && !m_state.activationState.isOneshot())
  {
    m_state.health = 40_hp;
    m_fleeTime += 1_frame;
  }
  if(alive())
  {
    ai::EnemyLocation enemyLocation{*this};
    if(enemyLocation.laraInView)
    {
      headRot = enemyLocation.visualAngleToLara;
    }
    if(m_fleeTime != 0_frame)
    {
      enemyLocation.laraZoneId = static_cast<uint32_t>(-1);
      enemyLocation.laraUnreachable = true;
      m_state.is_hit = true;
    }

    updateMood(*this, enemyLocation, false);

    turn = rotateTowardsTarget(getCreatureInfo()->maxTurnSpeed);
    switch(m_state.current_anim_state.get())
    {
    case 1:
      if(m_state.required_anim_state != 0_as)
      {
        goal(m_state.required_anim_state);
      }
      else if(isBored())
      {
        if(util::rand15() >= 96)
          goal(2_as);
        else
          goal(6_as);
      }
      else if(isEscaping())
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
      if(isBored() && util::rand15() < 96)
        goal(1_as, 6_as);
      else if(isEscaping())
        goal(1_as, 3_as);
      else if(canShootAtLara(enemyLocation))
        goal(1_as, 4_as);
      else if(!enemyLocation.laraInView || enemyLocation.distance > util::square(3_sectors))
        goal(1_as, 3_as);
      break;
    case 3:
      getCreatureInfo()->maxTurnSpeed = 6_deg / 1_frame;
      tiltRot = turn / 2;
      if(isBored() && util::rand15() < 96)
        goal(1_as, 6_as);
      else if(canShootAtLara(enemyLocation))
        goal(1_as, 4_as);
      else if(enemyLocation.laraInView && enemyLocation.distance < util::square(3_sectors))
        goal(1_as, 2_as);
      break;
    case 4:
      if(m_state.required_anim_state != 0_as)
        goal(m_state.required_anim_state);
      else if(canShootAtLara(enemyLocation))
        goal(7_as);
      else
        goal(1_as);
      break;
    case 6:
      if(!isBored())
        goal(1_as);
      else if(util::rand15() < 96)
        goal(1_as, 2_as);
      break;
    case 7:
      if(m_state.required_anim_state == 0_as)
      {
        if(tryShootAtLara(*this, enemyLocation.distance, {60_len, 200_len, 0_len}, 11, headRot))
          hitLara(25_hp);
        if(tryShootAtLara(*this, enemyLocation.distance, {-57_len, 200_len, 0_len}, 14, headRot))
          hitLara(25_hp);
        require(4_as);
      }
      if(isEscaping() && util::rand15() > 8192)
        require(1_as);
      break;
    default:
      break;
    }
  }
  else if(m_state.current_anim_state != 5_as) // injured/dying
  {
    getSkeleton()->setAnim(gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::Pierre)->animations[12]});
    m_state.current_anim_state = 5_as;
    getWorld().createPickup(TR1ItemId::MagnumsSprite, m_state.location.room, m_state.location.position);
    getWorld().createPickup(TR1ItemId::ScionPiece2, m_state.location.room, m_state.location.position);
    getWorld().createPickup(TR1ItemId::Key1Sprite, m_state.location.room, m_state.location.position);
  }
  rotateCreatureTilt(tiltRot);
  rotateCreatureHead(headRot);
  getSkeleton()->patchBone(7, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
  animateCreature(turn, 0_deg);
  if(m_fleeTime != 0_frame)
  {
    if(raycastLineOfSight(getWorld().getCameraController().getTRLocation(),
                          m_state.location.position - core::TRVec{0_len, 1_sectors, 0_len},
                          getWorld().getObjectManager())
         .first)
    {
      m_fleeTime = 1_frame;
    }
    else if(m_fleeTime > 10_frame)
    {
      m_state.health = core::DeadHealth;
      freeCreatureInfo();
      kill();
      getWorld().setPierre(nullptr);
    }
  }
  if(getWaterSurfaceHeight().has_value())
  {
    m_state.health = core::DeadHealth;
    freeCreatureInfo();
    kill();
    getWorld().setPierre(nullptr);
  }
}

void Pierre::serialize(const serialization::Serializer<world::World>& ser) const
{
  AIAgent::serialize(ser);
  ser(S_NV("fleeTime", m_fleeTime));
}

void Pierre::deserialize(const serialization::Deserializer<world::World>& ser)
{
  AIAgent::deserialize(ser);
  ser(S_NV("fleeTime", m_fleeTime));
}
} // namespace engine::objects
