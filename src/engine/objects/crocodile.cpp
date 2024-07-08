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
#include <tuple>

namespace engine::objects
{
namespace
{
constexpr auto InWaterPrepareAttack = 1_as;
constexpr auto InWaterBite = 2_as;
constexpr auto InWaterDead = 3_as;

constexpr auto OnLandThink = 1_as;
constexpr auto OnLandPrepareAttack = 2_as;
constexpr auto OnLandRunForward = 3_as;
constexpr auto OnLandTurnRight = 4_as;
constexpr auto OnLandBite = 5_as;
constexpr auto OnLandDead = 7_as;
} // namespace

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
  if(m_state.current_anim_state == InWaterPrepareAttack)
  {
    if(enemyLocation.canAttackLara && touched())
      goal(InWaterBite);
  }
  else if(m_state.current_anim_state == InWaterBite)
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
        require(InWaterPrepareAttack);
      }
    }
    else
    {
      goal(InWaterPrepareAttack);
    }
  }
  rotateCreatureHead(headRot);
  if(auto waterSurfaceHeight = getWaterSurfaceHeight(); waterSurfaceHeight.has_value())
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
    getSkeleton()->setAnim(gsl::not_null{
      &getWorld().getWorldGeometry().findAnimatedModelForType(TR1ItemId::CrocodileOnLand)->animations[0]});
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
  if(m_state.current_anim_state != InWaterDead)
  {
    getSkeleton()->setAnim(gsl::not_null{
      &getWorld().getWorldGeometry().findAnimatedModelForType(TR1ItemId::CrocodileInWater)->animations[4]});
    m_state.current_anim_state = InWaterDead;
    m_state.health = core::DeadHealth;
  }
  if(const auto waterSurfaceHeight = getWaterSurfaceHeight(); waterSurfaceHeight.has_value())
  {
    if(*waterSurfaceHeight + 32_len < m_state.location.position.Y)
    {
      m_state.location.position.Y -= 32_len;
    }
    else if(*waterSurfaceHeight > m_state.location.position.Y)
    {
      m_state.location.position.Y = *waterSurfaceHeight;
      freeCreatureInfo();
    }
  }
  else
  {
    getSkeleton()->setAnim(gsl::not_null{
      &getWorld().getWorldGeometry().findAnimatedModelForType(TR1ItemId::CrocodileOnLand)->animations[11]});
    m_state.type = TR1ItemId::CrocodileOnLand;
    goal(OnLandDead);
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
  if(m_state.current_anim_state == OnLandTurnRight)
  {
    m_state.rotation.Y += 6_deg;
  }
  else
  {
    turnRot = rotateTowardsTarget(3_deg / 1_frame);
  }
  switch(m_state.current_anim_state.get())
  {
  case OnLandThink.get():
    if(enemyLocation.canAttackLara && enemyLocation.distance < util::square(435_len))
    {
      goal(OnLandBite);
      break;
    }
    switch(getCreatureInfo()->mood)
    {
    case ai::Mood::Escape:
      goal(OnLandPrepareAttack);
      break;
    case ai::Mood::Attack:
      if(abs(enemyLocation.visualAngleToLara) <= 90_deg || enemyLocation.distance <= util::square(3_sectors))
        goal(OnLandPrepareAttack);
      else
        goal(OnLandTurnRight);
      break;
    case ai::Mood::Stalk:
      goal(OnLandRunForward);
      break;
    default:
      // silence compiler
      break;
    }
    break;
  case OnLandPrepareAttack.get():
    if(enemyLocation.laraInView && touched(0x3fcUL))
      goal(OnLandThink);
    else if(isStalking())
      goal(OnLandRunForward);
    else if(isBored())
      goal(OnLandThink);
    else if(isAttacking() && enemyLocation.distance > util::square(3_sectors)
            && abs(enemyLocation.visualAngleToLara) > 90_deg)
      goal(OnLandThink);
    break;
  case OnLandRunForward.get():
    if(enemyLocation.laraInView && touched(0x03fcUL))
      goal(OnLandThink);
    else if(isAttacking() || isEscaping())
      goal(OnLandPrepareAttack);
    else if(isBored())
      goal(OnLandThink);
    break;
  case OnLandTurnRight.get():
    if(abs(enemyLocation.visualAngleToLara) < 90_deg)
      goal(OnLandRunForward);
    break;
  case OnLandBite.get():
    if(m_state.required_anim_state == 0_as)
    {
      emitParticle({5_len, -21_len, 467_len}, 9, &createBloodSplat);
      hitLara(100_hp);
      require(OnLandThink);
    }
    break;
  default:
    break;
  }

  return {headRot, turnRot};
}

void Crocodile::updateOnLandDead()
{
  if(m_state.current_anim_state == OnLandDead)
    return;

  getSkeleton()->setAnim(
    gsl::not_null{&getWorld().getWorldGeometry().findAnimatedModelForType(TR1ItemId::CrocodileOnLand)->animations[11]});
  m_state.current_anim_state = OnLandDead;
  m_state.health = core::DeadHealth;
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
    getSkeleton()->setAnim(gsl::not_null{
      &getWorld().getWorldGeometry().findAnimatedModelForType(TR1ItemId::CrocodileInWater)->animations[0]});
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
