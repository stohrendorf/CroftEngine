#include "mutant.h"

#include "aiagent.h"
#include "core/angle.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/vec.h"
#include "engine/ai/ai.h"
#include "engine/ai/pathfinder.h"
#include "engine/cameracontroller.h"
#include "engine/heightinfo.h"
#include "engine/items_tr1.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/particle.h"
#include "engine/player.h"
#include "engine/skeletalmodelnode.h"
#include "engine/soundeffects_tr1.h"
#include "engine/weapontype.h"
#include "engine/world/animation.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "loader/file/larastateid.h"
#include "modelobject.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "shatter.h"
#include "util/helpers.h"

#include <algorithm>
#include <bitset>
#include <cstddef>
#include <exception>
#include <memory>
#include <utility>

namespace engine::objects
{
void FlyingMutant::update()
{
  activateAi();

  static constexpr auto CloseRange = 5_sectors - 1_sectors / 2;
  static constexpr auto AttackRange = 600_len;

  static constexpr auto DoPrepareAttack = 1_as;
  static constexpr auto DoWalk = 2_as;
  static constexpr auto DoRun = 3_as;
  static constexpr auto DoHit150 = 4_as;
  static constexpr auto DoPlan = 6_as;
  static constexpr auto DoHit100 = 7_as;
  static constexpr auto DoHit200 = 8_as;
  static constexpr auto DoShootBullet = 9_as;
  static constexpr auto DoThrowGrenade = 10_as;
  static constexpr auto DoAttack = 11_as;
  static constexpr auto DoFly = 13_as;

  core::Angle turn = 0_deg;
  core::Angle headRot = 0_deg;
  if(!alive())
  {
    if(shatterModel(*this, 0xffffffffu, 100_len))
    {
      playSoundEffect(TR1SoundEffect::Mummy);
      freeCreatureInfo();
      kill();
      m_state.triggerState = TriggerState::Deactivated;
      return;
    }
  }
  else
  {
    getCreatureInfo()->pathFinder.setLimits(getWorld(), 256_len, -256_len, 0_len);
    ai::EnemyLocation enemyLocation{*this};
    bool frontLeft = false;
    bool frontRight = false;
    if(m_state.type != TR1ItemId::WalkingMutant2)
    {
      if(canShootAtLara(enemyLocation)
         && (enemyLocation.zoneId != enemyLocation.laraZoneId || enemyLocation.distance > util::square(3840_len)))
      {
        if(enemyLocation.visualAngleToLara > 0_deg && enemyLocation.visualAngleToLara < 45_deg)
        {
          frontRight = true;
        }
        else if(enemyLocation.visualAngleToLara < 0_deg && enemyLocation.visualAngleToLara > -45_deg)
        {
          frontLeft = true;
        }
      }
    }
    if(m_state.type == TR1ItemId::FlyingMutant)
    {
      if(m_state.current_anim_state == DoFly)
      {
        if(m_flying && !isEscaping() && enemyLocation.zoneId == enemyLocation.laraZoneId)
        {
          m_flying = false;
        }
        if(!m_flying)
        {
          updateMood(*this, enemyLocation, true);
        }
        getCreatureInfo()->pathFinder.setLimits(getWorld(), 30_sectors, -30_sectors, 32_len);
        enemyLocation = ai::EnemyLocation{*this};
      }
      else if(isEscaping()
              || (enemyLocation.zoneId != enemyLocation.laraZoneId && !frontRight && !frontLeft
                  && (!enemyLocation.laraInView || isBored())))
      {
        m_flying = true;
      }
    }

    if(enemyLocation.laraInView)
    {
      headRot = enemyLocation.visualAngleToLara;
    }
    if(m_state.current_anim_state != DoFly)
    {
      updateMood(*this, enemyLocation, false);
    }
    else if(m_flying)
    {
      updateMood(*this, enemyLocation, true);
    }

    turn = rotateTowardsTarget(getCreatureInfo()->maxTurnSpeed);
    switch(m_state.current_anim_state.get())
    {
    case DoPrepareAttack.get():
      m_shootBullet = false;
      m_throwGrenade = false;
      m_lookingAround = false;
      if(m_flying)
        goal(DoFly);
      else if(touched(0x678u)
              || (enemyLocation.canAttackLara && enemyLocation.distance < util::square(AttackRange / 2)))
        goal(DoHit200);
      else if(enemyLocation.canAttackLara && enemyLocation.distance < util::square(AttackRange))
        goal(DoHit150);
      else if(frontRight)
        goal(DoShootBullet);
      else if(frontLeft)
        goal(DoThrowGrenade);
      else if(isBored() || (isStalking() && enemyLocation.distance < util::square(CloseRange)))
        goal(DoPlan);
      else
        goal(DoRun);
      break;
    case DoWalk.get():
      getCreatureInfo()->maxTurnSpeed = 2_deg / 1_frame;
      if(frontLeft || frontRight || m_flying || isAttacking() || isEscaping())
      {
        goal(DoPrepareAttack);
      }
      else if(isBored() || (isStalking() && enemyLocation.zoneId != enemyLocation.laraZoneId))
      {
        if(util::rand15() < 80)
        {
          goal(DoPlan);
        }
      }
      else if(isStalking() && enemyLocation.distance > util::square(CloseRange))
      {
        goal(DoPrepareAttack);
        break;
      }
      break;
    case DoRun.get():
      getCreatureInfo()->maxTurnSpeed = 6_deg / 1_frame;
      if(m_flying || touched(0x678u)
         || (enemyLocation.canAttackLara && enemyLocation.distance < util::square(AttackRange)))
      {
        goal(DoPrepareAttack);
      }
      else if(enemyLocation.laraInView && enemyLocation.distance < util::square(5_sectors / 2))
      {
        goal(DoHit100);
      }
      else if(frontLeft || frontRight || isBored()
              || (isStalking() && enemyLocation.distance < util::square(CloseRange)))
      {
        goal(DoPrepareAttack);
      }
      break;
    case DoHit150.get():
      if(m_state.required_anim_state == 0_as && touched(0x678u))
      {
        emitParticle(core::TRVec{-27_len, 98_len, 0_len}, 10, &createBloodSplat);
        hitLara(150_hp);
        require(DoPrepareAttack);
      }
      break;
    case DoPlan.get():
      headRot = 0_deg;
      if(frontRight || frontLeft || m_flying)
      {
        goal(DoPrepareAttack);
      }
      else if(isStalking())
      {
        if(enemyLocation.distance >= util::square(CloseRange))
          goal(DoPrepareAttack);
        else if(enemyLocation.zoneId == enemyLocation.laraZoneId || util::rand15() < 256)
          goal(DoWalk);
      }
      else if(isBored() && util::rand15() < 256)
      {
        goal(DoWalk);
      }
      else if(isEscaping() || isAttacking())
      {
        goal(DoPrepareAttack);
      }
      break;
    case DoHit100.get():
      if(m_state.required_anim_state == 0_as && touched(0x678u))
      {
        emitParticle(core::TRVec{-27_len, 98_len, 0_len}, 10, &createBloodSplat);
        hitLara(100_hp);
        require(DoRun);
      }
      break;
    case DoHit200.get():
      if(m_state.required_anim_state == 0_as && touched(0x678u))
      {
        emitParticle(core::TRVec{-27_len, 98_len, 0_len}, 10, &createBloodSplat);
        hitLara(200_hp);
        require(DoPrepareAttack);
      }
      break;
    case DoShootBullet.get():
      m_shootBullet = true;
      m_lookingAround = true;
      if(frontRight)
        goal(DoAttack);
      else
        goal(DoPrepareAttack);
      break;
    case DoThrowGrenade.get():
      m_throwGrenade = true;
      if(frontLeft)
        goal(DoAttack);
      else
        goal(DoPrepareAttack);
      break;
    case DoAttack.get():
      if(m_shootBullet)
      {
        m_shootBullet = false;
        emitParticle({-35_len, 269_len, 0_len}, 9, &createMutantBullet);
      }
      else if(m_throwGrenade)
      {
        m_throwGrenade = false;
        emitParticle({51_len, 213_len, 0_len}, 14, &createMutantGrenade);
      }
      break;
    case 12:
      goal(DoPrepareAttack);
      break;
    case DoFly.get():
      if(!m_flying && m_state.location.position.Y == m_state.floor)
      {
        goal(DoPrepareAttack);
      }
      break;
    }
  }

  if(!m_lookingAround)
  {
    getCreatureInfo()->headRotation = getCreatureInfo()->neckRotation;
  }
  rotateCreatureHead(headRot);
  if(!m_lookingAround)
  {
    getCreatureInfo()->neckRotation = std::exchange(getCreatureInfo()->headRotation, 0_deg);
  }
  else
  {
    getCreatureInfo()->neckRotation = 0_deg;
  }
  if(getSkeleton()->getBoneCount() >= 2)
    getSkeleton()->patchBone(1, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
  if(getSkeleton()->getBoneCount() >= 3)
    getSkeleton()->patchBone(2, core::TRRotation{0_deg, getCreatureInfo()->neckRotation, 0_deg}.toMatrix());
  animateCreature(turn, 0_deg);
}

void FlyingMutant::serialize(const serialization::Serializer<world::World>& ser) const
{
  AIAgent::serialize(ser);
  ser(S_NV("shootBullet", m_shootBullet),
      S_NV("throwGrenade", m_throwGrenade),
      S_NV("flying", m_flying),
      S_NV("lookingAround", m_lookingAround));
}

void FlyingMutant::deserialize(const serialization::Deserializer<world::World>& ser)
{
  AIAgent::deserialize(ser);
  ser(S_NV("shootBullet", m_shootBullet),
      S_NV("throwGrenade", m_throwGrenade),
      S_NV("flying", m_flying),
      S_NV("lookingAround", m_lookingAround));
}

void CentaurMutant::update()
{
  activateAi();

  core::Angle turn = 0_deg;
  core::Angle headRot = 0_deg;
  if(getHealth() > 0_hp)
  {
    const ai::EnemyLocation enemyLocation{*this};
    if(enemyLocation.laraInView)
    {
      headRot = enemyLocation.visualAngleToLara;
    }
    updateMood(*this, enemyLocation, true);
    turn = rotateTowardsTarget(4_deg / 1_frame);
    switch(m_state.current_anim_state.get())
    {
    case 1:
      getCreatureInfo()->neckRotation = 0_deg;
      if(m_state.required_anim_state != 0_as)
        goal(m_state.required_anim_state);
      else if((enemyLocation.canAttackLara && enemyLocation.distance < util::square(1536_len))
              || !canShootAtLara(enemyLocation))
        goal(3_as);
      else
        goal(4_as);
      break;
    case 2:
      if(m_state.required_anim_state == 0_as)
      {
        require(4_as);
        getCreatureInfo()->neckRotation = emitParticle({11_len, 415_len, 41_len}, 13, &createMutantGrenade)->angle.X;
      }
      break;
    case 3:
      if(enemyLocation.canAttackLara && enemyLocation.distance < util::square(1536_len))
        goal(1_as, 6_as);
      else if(canShootAtLara(enemyLocation))
        goal(1_as, 4_as);
      else if(util::rand15() < 96)
        goal(1_as, 6_as);
      break;
    case 4:
      if(m_state.required_anim_state != 0_as)
        goal(m_state.required_anim_state);
      else if(canShootAtLara(enemyLocation))
        goal(2_as);
      else
        goal(1_as);
      break;
    case 6:
      if(m_state.required_anim_state == 0_as)
      {
        if(touched(0x30199u))
        {
          emitParticle({50_len, 30_len, 0_len}, 5, &createBloodSplat);
          hitLara(200_hp);
          require(1_as);
        }
      }
      break;
    default:
      break;
    }
  }
  else if(m_state.current_anim_state != 5_as)
  {
    m_state.current_anim_state = 5_as;
    getSkeleton()->setAnim(
      gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::CentaurMutant)->animations[8]});
  }

  rotateCreatureHead(headRot);
  getSkeleton()->patchBone(
    11, core::TRRotation{getCreatureInfo()->neckRotation, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
  animateCreature(turn, 0_deg);
  if(m_state.triggerState == TriggerState::Deactivated)
  {
    playSoundEffect(TR1SoundEffect::Mummy);
    shatterModel(*this, ~std::bitset<32>{0}, 100_len);
    kill();
    m_state.triggerState = TriggerState::Deactivated;
  }
}

void TorsoBoss::update()
{
  static constexpr auto Think = 1_as;
  static constexpr auto TurnLeft = 2_as;
  static constexpr auto TurnRight = 3_as;
  static constexpr auto Attack1 = 4_as;
  static constexpr auto Attack2 = 5_as;
  static constexpr auto KillOnTouch = 6_as;
  static constexpr auto Approach = 7_as;
  static constexpr auto Initial = 8_as;
  static constexpr auto Falling = 9_as;
  static constexpr auto BossKilled = 10_as;
  static constexpr auto LaraKilled = 11_as;

  activateAi();

  core::Angle headRot = 0_deg;
  if(alive())
  {
    const ai::EnemyLocation enemyLocation{*this};
    if(enemyLocation.laraInView)
    {
      headRot = enemyLocation.visualAngleToLara;
    }
    updateMood(*this, enemyLocation, true);
    const auto angleToTarget = angleFromAtan(getCreatureInfo()->target.X - m_state.location.position.X,
                                             getCreatureInfo()->target.Z - m_state.location.position.Z)
                               - m_state.rotation.Y;

    if(touched())
    {
      hitLara(5_hp);
    }
    switch(m_state.current_anim_state.get())
    {
    case Think.get():
      if(getWorld().getObjectManager().getLara().isDead())
      {
        break;
      }

      m_hasHitLara = false;
      m_turnStartFrame = 0_frame;
      if(angleToTarget > 45_deg)
      {
        goal(TurnRight);
        break;
      }
      if(angleToTarget < -45_deg)
      {
        goal(TurnLeft);
        break;
      }
      if(enemyLocation.distance >= util::square(2600_len))
      {
        goal(Approach);
      }
      else if(getWorld().getObjectManager().getLara().m_state.health > core::LaraHealth / 2)
      {
        if(util::rand15(2) == 0)
          goal(Attack2);
        else
          goal(Attack1);
      }
      else if(enemyLocation.distance >= util::square(2250_len))
      {
        goal(Approach);
      }
      else
      {
        goal(KillOnTouch);
      }
      break;
    case TurnLeft.get():
      if(m_turnStartFrame == 0_frame)
      {
        m_turnStartFrame = getSkeleton()->getFrame();
      }
      else
      {
        const auto frameDelta = getSkeleton()->getFrame() - m_turnStartFrame;
        if(frameDelta > 13_frame && frameDelta < 23_frame)
        {
          m_state.rotation.Y -= 9_deg;
        }
      }

      if(angleToTarget > -45_deg)
        goal(Think);
      break;
    case TurnRight.get():
      if(m_turnStartFrame == 0_frame)
      {
        m_turnStartFrame = getSkeleton()->getFrame();
      }
      else
      {
        const auto frameDelta = getSkeleton()->getFrame() - m_turnStartFrame;
        if(frameDelta > 16_frame && frameDelta < 23_frame)
        {
          m_state.rotation.Y += 14_deg;
        }
      }

      if(angleToTarget < 45_deg)
        goal(Think);
      break;
    case Attack1.get():
      if(!m_hasHitLara && touched(0x3ff8000u))
      {
        hitLara(500_hp);
        m_hasHitLara = true;
      }
      break;
    case Attack2.get():
      if(!m_hasHitLara && touched(0x3fffff0u))
      {
        hitLara(500_hp);
        m_hasHitLara = true;
      }
      break;
    case KillOnTouch.get():
      if(touched(0x3ff8000u) || getWorld().getObjectManager().getLara().isDead())
      {
        goal(LaraKilled);
        auto& lara = getWorld().getObjectManager().getLara();
        lara.getSkeleton()->setAnim(
          gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations[0]});
        lara.setGoalAnimState(loader::file::LaraStateId::BoulderDeath);
        lara.setCurrentAnimState(loader::file::LaraStateId::BoulderDeath);
        lara.setCurrentRoom(m_state.location.room);
        lara.m_state.location = m_state.location;
        lara.m_state.rotation = {0_deg, m_state.rotation.Y, 0_deg};
        lara.m_state.health = core::DeadHealth;
        lara.setAir(-1_frame);
        getWorld().getPlayer().selectedWeaponType = WeaponType::None;
        lara.setHandStatus(HandStatus::Grabbing);
        lara.m_state.falling = false;
        getWorld().getCameraController().setDistance(2048_len);
        getWorld().getCameraController().setModifier(CameraModifier::FollowCenter);
      }
      break;
    case Approach.get():
      // TODO this is just weird, but it's just like the original... seems to be a copy-pasta error
      goal(m_state.goal_anim_state.get() + core::toAu(std::clamp(angleToTarget, -3_deg, 3_deg)));

      if(abs(angleToTarget) > 45_deg || enemyLocation.distance < util::square(2600_len))
        goal(Think);

      break;
    case Initial.get():
      goal(Falling);
      m_state.falling = true;
      break;
    case LaraKilled.get():
      getWorld().getCameraController().setDistance(2048_len);
      getWorld().getCameraController().setModifier(CameraModifier::FollowCenter);
      break;
    default:
      break;
    }
  }
  else
  {
    if(m_state.current_anim_state != BossKilled)
    {
      getSkeleton()->setAnim(gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::TorsoBoss)->animations[13]});
      m_state.current_anim_state = BossKilled;
    }
  }

  rotateCreatureHead(headRot);
  if(m_state.current_anim_state == Falling)
  {
    ModelObject::update();
    if(m_state.location.position.Y > m_state.floor)
    {
      goal(Think);
      settle();
      getWorld().getCameraController().setBounce(500_len);
    }
  }
  else
  {
    animateCreature(0_deg, 0_deg);
  }

  if(m_state.triggerState == TriggerState::Deactivated)
  {
    playSoundEffect(TR1SoundEffect::Mummy);
    shatterModel(*this, ~0u, 250_len);
    const auto sector = m_state.location.updateRoom();
    getWorld().handleCommandSequence(
      HeightInfo::fromFloor(sector, m_state.location.position, getWorld().getObjectManager().getObjects())
        .lastCommandSequenceOrDeath,
      true);
    kill();
    m_state.triggerState = TriggerState::Deactivated;
  }
}

void TorsoBoss::serialize(const serialization::Serializer<world::World>& ser) const
{
  AIAgent::serialize(ser);
  ser(S_NV("hasHitLara", m_hasHitLara), S_NV("turnStartFrame", m_turnStartFrame));
}

void TorsoBoss::deserialize(const serialization::Deserializer<world::World>& ser)
{
  AIAgent::deserialize(ser);
  ser(S_NV("hasHitLara", m_hasHitLara), S_NV("turnStartFrame", m_turnStartFrame));
}

WalkingMutant::WalkingMutant(const std::string& name,
                             const gsl::not_null<world::World*>& world,
                             const gsl::not_null<const world::Room*>& room,
                             const loader::file::Item& item,
                             const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : FlyingMutant{name, world, room, item, animatedModel}
{
  for(size_t i = 0; i < getSkeleton()->getBoneCount(); ++i)
  {
    getSkeleton()->setVisible(i, (0xffe07fffu >> i) & 1u);
  }
  getSkeleton()->rebuildMesh();
}
} // namespace engine::objects
