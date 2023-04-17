#include "crocodile.h"

#include "core/angle.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/ai/ai.h"
#include "engine/ai/pathfinder.h"
#include "engine/heightinfo.h"
#include "engine/items_tr1.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/particle.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/animation.h"
#include "engine/world/room.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "modelobject.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "util/helpers.h"

#include <boost/assert.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <optional>

namespace engine::objects
{
void Crocodile::updateInWaterAlive()
{
  core::Angle headRot = 0_deg;
  const ai::EnemyLocation enemyLocation{*this};
  if(enemyLocation.laraInView)
  {
    headRot = enemyLocation.visualAngleToLara;
  }
  updateMood(*this, enemyLocation, true);
  rotateTowardsTarget(3_deg / 1_frame);
  if(m_state.current_anim_state == 1_as)
  {
    if(enemyLocation.canAttackLara && touched())
      goal(2_as);
  }
  else if(m_state.current_anim_state == 2_as)
  {
    if(getSkeleton()->getLocalFrame() == 0_frame)
    {
      require(0_as);
    }
    if(enemyLocation.canAttackLara && touched())
    {
      if(m_state.required_anim_state == 0_as)
      {
        emitParticle({5_len, -21_len, 467_len}, 9, &createBloodSplat);
        hitLara(100_hp);
        require(1_as);
      }
    }
    else
    {
      goal(1_as);
    }
  }
  rotateCreatureHead(headRot);
  if(auto waterSurfaceHeight = getWaterSurfaceHeight())
  {
    *waterSurfaceHeight += core::QuarterSectorSize;
    if(*waterSurfaceHeight > m_state.location.position.Y)
    {
      m_state.location.position.Y = *waterSurfaceHeight;
    }
  }
  else
  {
    m_state.type = TR1ItemId::CrocodileOnLand;
    getSkeleton()->setAnim(
      gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::CrocodileOnLand)->animations[0]});
    goal(getSkeleton()->getAnim()->state_id);
    m_state.current_anim_state = getSkeleton()->getAnim()->state_id;
    m_state.rotation.X = 0_deg;
    m_state.location.position.Y = m_state.floor;
    getCreatureInfo()->pathFinder.setLimits(getWorld(), 256_len, -256_len, 0_len);

    loadObjectInfo(true);
  }
  getSkeleton()->patchBone(8, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
  animateCreature(0_deg, 0_deg);
}

void Crocodile::updateInWaterDead()
{
  if(m_state.current_anim_state != 3_as)
  {
    getSkeleton()->setAnim(
      gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::CrocodileInWater)->animations[4]});
    m_state.current_anim_state = 3_as;
    m_state.health = core::DeadHealth;
  }
  if(const auto waterSurfaceHeight = getWaterSurfaceHeight())
  {
    if(*waterSurfaceHeight + 32_len < m_state.location.position.Y)
    {
      m_state.location.position.Y = m_state.location.position.Y - 32_len;
    }
    else if(*waterSurfaceHeight > m_state.location.position.Y)
    {
      m_state.location.position.Y = *waterSurfaceHeight;
      freeCreatureInfo();
    }
  }
  else
  {
    getSkeleton()->setAnim(
      gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::CrocodileOnLand)->animations[11]});
    m_state.type = TR1ItemId::CrocodileOnLand;
    goal(7_as);
    m_state.current_anim_state = m_state.goal_anim_state;
    const auto sector = m_state.location.moved({}).updateRoom();
    m_state.location.position.Y
      = HeightInfo::fromFloor(sector, m_state.location.position, getWorld().getObjectManager().getObjects()).y;
    m_state.rotation.X = 0_deg;

    loadObjectInfo(true);
  }
  ModelObject::update();
  const auto sector = m_state.location.updateRoom();
  m_state.floor
    = HeightInfo::fromFloor(sector, m_state.location.position, getWorld().getObjectManager().getObjects()).y;
  setCurrentRoom(m_state.location.room);
}

void Crocodile::updateInWater()
{
  if(alive())
  {
    updateInWaterAlive();
  }
  else
  {
    updateInWaterDead();
  }
}

std::tuple<core::Angle, core::Angle> Crocodile::updateOnLandAlive()
{
  core::Angle turnRot = 0_deg;
  core::Angle headRot = 0_deg;
  const ai::EnemyLocation enemyLocation{*this};
  if(enemyLocation.laraInView)
  {
    headRot = enemyLocation.visualAngleToLara;
  }
  updateMood(*this, enemyLocation, true);
  if(m_state.current_anim_state == 4_as)
  {
    m_state.rotation.Y += 6_deg;
  }
  else
  {
    turnRot = rotateTowardsTarget(3_deg / 1_frame);
  }
  switch(m_state.current_anim_state.get())
  {
  case 1:
    if(enemyLocation.canAttackLara && enemyLocation.distance < util::square(435_len))
    {
      goal(5_as);
      break;
    }
    switch(getCreatureInfo()->mood)
    {
    case ai::Mood::Escape:
      goal(2_as);
      break;
    case ai::Mood::Attack:
      if(abs(enemyLocation.visualAngleToLara) <= 90_deg || enemyLocation.distance <= util::square(3_sectors))
        goal(2_as);
      else
        goal(4_as);
      break;
    case ai::Mood::Stalk:
      goal(3_as);
      break;
    default:
      // silence compiler
      break;
    }
    break;
  case 2:
    if(enemyLocation.laraInView && touched(0x3fcUL))
      goal(1_as);
    else if(isStalking())
      goal(3_as);
    else if(isBored())
      goal(1_as);
    else if(isAttacking() && enemyLocation.distance > util::square(3_sectors)
            && abs(enemyLocation.visualAngleToLara) > 90_deg)
      goal(1_as);
    break;
  case 3:
    if(enemyLocation.laraInView && touched(0x03fcUL))
      goal(1_as);
    else if(isAttacking() || isEscaping())
      goal(2_as);
    else if(isBored())
      goal(1_as);
    break;
  case 4:
    if(abs(enemyLocation.visualAngleToLara) < 90_deg)
      goal(3_as);
    break;
  case 5:
    if(m_state.required_anim_state == 0_as)
    {
      emitParticle({5_len, -21_len, 467_len}, 9, &createBloodSplat);
      hitLara(100_hp);
      require(1_as);
    }
    break;
  default:
    break;
  }

  return {headRot, turnRot};
}

void Crocodile::updateOnLandDead()
{
  if(m_state.current_anim_state == 7_as)
    return;

  getSkeleton()->setAnim(
    gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::CrocodileOnLand)->animations[11]});
  m_state.current_anim_state = 7_as;
}

void Crocodile::updateOnLand()
{
  BOOST_ASSERT(m_state.type == TR1ItemId::CrocodileOnLand);
  core::Angle turn = 0_deg;
  core::Angle headRot = 0_deg;
  if(alive())
  {
    std::tie(headRot, turn) = updateOnLandAlive();
  }
  else
  {
    updateOnLandDead();
  }
  if(getCreatureInfo() != nullptr)
  {
    rotateCreatureHead(headRot);
  }
  if(m_state.location.room->isWaterRoom)
  {
    getSkeleton()->setAnim(
      gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::CrocodileInWater)->animations[0]});
    goal(getSkeleton()->getAnim()->state_id);
    m_state.current_anim_state = getSkeleton()->getAnim()->state_id;
    m_state.type = TR1ItemId::CrocodileInWater;
    if(getCreatureInfo() != nullptr)
    {
      getCreatureInfo()->pathFinder.setLimits(getWorld(), 20_sectors, -20_sectors, 16_len);
    }

    loadObjectInfo(true);
  }
  if(getCreatureInfo() != nullptr)
  {
    getSkeleton()->patchBone(8, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
    animateCreature(turn, 0_deg);
  }
  else
  {
    ModelObject::update();
  }
}

void Crocodile::update()
{
  activateAi();

  if(m_state.type == TR1ItemId::CrocodileInWater)
  {
    updateInWater();
  }
  else
  {
    updateOnLand();
  }
}
} // namespace engine::objects
