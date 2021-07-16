#include "crocodile.h"

#include "engine/particle.h"
#include "engine/world/animation.h"
#include "engine/world/world.h"
#include "laraobject.h"

namespace engine::objects
{
void Crocodile::updateInWater()
{
  core::Angle headRot = 0_deg;
  if(alive())
  {
    const ai::EnemyLocation enemyLocation{getWorld(), m_state};
    if(enemyLocation.enemyAhead)
    {
      headRot = enemyLocation.angleToEnemy;
    }
    updateMood(getWorld(), m_state, enemyLocation, true);
    rotateTowardsTarget(3_deg / 1_frame);
    if(m_state.current_anim_state == 1_as)
    {
      if(enemyLocation.canAttackForward && touched())
        goal(2_as);
    }
    else if(m_state.current_anim_state == 2_as)
    {
      if(getSkeleton()->getLocalFrame() == 0_frame)
      {
        require(0_as);
      }
      if(enemyLocation.canAttackForward && touched())
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
      if(*waterSurfaceHeight > m_state.position.position.Y)
      {
        m_state.position.position.Y = *waterSurfaceHeight;
      }
    }
    else
    {
      m_state.type = TR1ItemId::CrocodileOnLand;
      getSkeleton()->setAnim(&getWorld().findAnimatedModelForType(TR1ItemId::CrocodileOnLand)->animations[0]);
      goal(getSkeleton()->getAnim()->state_id);
      m_state.current_anim_state = getSkeleton()->getAnim()->state_id;
      m_state.rotation.X = 0_deg;
      m_state.position.position.Y = m_state.floor;
      m_state.creatureInfo->pathFinder.step = 256_len;
      m_state.creatureInfo->pathFinder.drop = -256_len;
      m_state.creatureInfo->pathFinder.fly = 0_len;

      loadObjectInfo(true);
    }
    getSkeleton()->patchBone(8, core::TRRotation{0_deg, m_state.creatureInfo->headRotation, 0_deg}.toMatrix());
    animateCreature(0_deg, 0_deg);
  }
  else
  {
    if(m_state.current_anim_state != 3_as)
    {
      getSkeleton()->setAnim(&getWorld().findAnimatedModelForType(TR1ItemId::CrocodileInWater)->animations[4]);
      m_state.current_anim_state = 3_as;
      m_state.health = core::DeadHealth;
    }
    if(const auto waterSurfaceHeight = getWaterSurfaceHeight())
    {
      if(*waterSurfaceHeight + 32_len < m_state.position.position.Y)
      {
        m_state.position.position.Y = m_state.position.position.Y - 32_len;
      }
      else if(*waterSurfaceHeight > m_state.position.position.Y)
      {
        m_state.position.position.Y = *waterSurfaceHeight;
        m_state.creatureInfo.reset();
      }
    }
    else
    {
      getSkeleton()->setAnim(&getWorld().findAnimatedModelForType(TR1ItemId::CrocodileOnLand)->animations[11]);
      m_state.type = TR1ItemId::CrocodileOnLand;
      goal(7_as);
      m_state.current_anim_state = m_state.goal_anim_state;
      auto room = m_state.position.room;
      auto sector = findRealFloorSector(m_state.position.position, &room);
      m_state.position.position.Y
        = HeightInfo::fromFloor(sector, m_state.position.position, getWorld().getObjectManager().getObjects()).y;
      m_state.rotation.X = 0_deg;

      loadObjectInfo(true);
    }
    ModelObject::update();
    auto room = m_state.position.room;
    auto sector = findRealFloorSector(m_state.position.position, &room);
    m_state.floor
      = HeightInfo::fromFloor(sector, m_state.position.position, getWorld().getObjectManager().getObjects()).y;
    setCurrentRoom(room);
  }
}

void Crocodile::updateOnLand()
{
  BOOST_ASSERT(m_state.type == TR1ItemId::CrocodileOnLand);
  core::Angle turnRot = 0_deg;
  core::Angle headRot = 0_deg;
  if(alive())
  {
    const ai::EnemyLocation enemyLocation{getWorld(), m_state};
    if(enemyLocation.enemyAhead)
    {
      headRot = enemyLocation.angleToEnemy;
    }
    updateMood(getWorld(), m_state, enemyLocation, true);
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
      if(enemyLocation.canAttackForward && enemyLocation.enemyDistance < util::square(435_len))
      {
        goal(5_as);
        break;
      }
      switch(m_state.creatureInfo->mood)
      {
      case ai::Mood::Escape: goal(2_as); break;
      case ai::Mood::Attack:
        if((enemyLocation.angleToEnemy >= -90_deg && enemyLocation.angleToEnemy <= 90_deg)
           || enemyLocation.enemyDistance <= util::square(3 * core::SectorSize))
          goal(2_as);
        else
          goal(4_as);
        break;
      case ai::Mood::Stalk: goal(3_as); break;
      default:
        // silence compiler
        break;
      }
      break;
    case 2:
      if(enemyLocation.enemyAhead && touched(0x3fcUL))
        goal(1_as);
      else if(isStalking())
        goal(3_as);
      else if(isBored())
        goal(1_as);
      else if(isAttacking() && enemyLocation.enemyDistance > util::square(3 * core::SectorSize)
              && (enemyLocation.angleToEnemy < -90_deg || enemyLocation.angleToEnemy > 90_deg))
        goal(1_as);
      break;
    case 3:
      if(enemyLocation.enemyAhead && touched(0x03fcUL))
        goal(1_as);
      else if(isAttacking() || isEscaping())
        goal(2_as);
      else if(isBored())
        goal(1_as);
      break;
    case 4:
      if(enemyLocation.angleToEnemy > -90_deg && enemyLocation.angleToEnemy < 90_deg)
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
    default: break;
    }
  }
  else
  {
    if(m_state.current_anim_state != 7_as)
    {
      getSkeleton()->setAnim(&getWorld().findAnimatedModelForType(TR1ItemId::CrocodileOnLand)->animations[11]);
      m_state.current_anim_state = 7_as;
    }
  }
  if(m_state.creatureInfo != nullptr)
  {
    rotateCreatureHead(headRot);
  }
  if(m_state.position.room->isWaterRoom)
  {
    getSkeleton()->setAnim(&getWorld().findAnimatedModelForType(TR1ItemId::CrocodileInWater)->animations[0]);
    goal(getSkeleton()->getAnim()->state_id);
    m_state.current_anim_state = getSkeleton()->getAnim()->state_id;
    m_state.type = TR1ItemId::CrocodileInWater;
    if(m_state.creatureInfo != nullptr)
    {
      m_state.creatureInfo->pathFinder.step = 20 * core::SectorSize;
      m_state.creatureInfo->pathFinder.drop = -20 * core::SectorSize;
      m_state.creatureInfo->pathFinder.fly = 16_len;
    }

    loadObjectInfo(true);
  }
  if(m_state.creatureInfo != nullptr)
  {
    getSkeleton()->patchBone(8, core::TRRotation{0_deg, m_state.creatureInfo->headRotation, 0_deg}.toMatrix());
    animateCreature(turnRot, 0_deg);
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
