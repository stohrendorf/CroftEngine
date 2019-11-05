#include "mutant.h"

#include "engine/laranode.h"
#include "engine/particle.h"
#include "mutantegg.h"

namespace engine
{
namespace items
{
namespace
{
gsl::not_null<std::shared_ptr<Particle>> createMutantGrenade(Engine& engine,
                                                             const core::RoomBoundPosition& pos,
                                                             const core::Speed& speed,
                                                             const core::Angle& angle)
{
  auto particle = std::make_shared<MutantGrenadeParticle>(pos, engine, angle);
  setParent(particle, pos.room->node);
  return particle;
}
gsl::not_null<std::shared_ptr<Particle>> createMutantBullet(Engine& engine,
                                                            const core::RoomBoundPosition& pos,
                                                            const core::Speed& speed,
                                                            const core::Angle& angle)
{
  auto particle = std::make_shared<MutantBulletParticle>(pos, engine, angle);
  setParent(particle, pos.room->node);
  return particle;
}
} // namespace

void FlyingMutant::update()
{
  if(m_state.triggerState == TriggerState::Invisible)
  {
    m_state.triggerState = TriggerState::Active;
  }

  m_state.initCreatureInfo(getEngine());

  static const constexpr uint16_t ShootBullet = 1;
  static const constexpr uint16_t ThrowGrenade = 2;
  static const constexpr uint16_t Flying = 4;

  static const constexpr auto DoPrepareAttack = 1_as;
  static const constexpr auto DoWalk = 2_as;
  static const constexpr auto DoRun = 3_as;
  static const constexpr auto DoHit150 = 4_as;
  static const constexpr auto DoHit100 = 7_as;
  static const constexpr auto DoHit200 = 8_as;
  static const constexpr auto DoShootBullet = 9_as;
  static const constexpr auto DoThrowGrenade = 10_as;
  static const constexpr auto DoAttack = 11_as;
  static const constexpr auto DoFly = 13_as;

  core::Angle turnRot = 0_deg;
  core::Angle headRot = 0_deg;
  if(getHealth() <= 0_hp)
  {
    if(shatterModel(*this, 0xffffffffu, 100_len))
    {
      playSoundEffect(TR1SoundId::Mummy);
      m_state.creatureInfo.reset();
      kill();
      m_state.triggerState = TriggerState::Deactivated;
      return;
    }
  }
  else
  {
    m_state.creatureInfo->pathFinder.step = 256_len;
    m_state.creatureInfo->pathFinder.fly = 0_len;
    m_state.creatureInfo->pathFinder.drop = -256_len;
    ai::AiInfo aiInfo{getEngine(), m_state};
    bool frontLeft = false;
    bool frontRight = false;
    if(m_state.type != TR1ItemId::WalkingMutant2)
    {
      if(canShootAtLara(aiInfo)
         && (aiInfo.zone_number != aiInfo.enemy_zone || aiInfo.distance > util::square(3840_len)))
      {
        if(aiInfo.angle > 0_deg && aiInfo.angle < 45_deg)
        {
          frontRight = true;
        }
        else if(aiInfo.angle < 0_deg && aiInfo.angle > -45_deg)
        {
          frontLeft = true;
        }
      }
    }
    if(m_state.type == TR1ItemId::FlyingMutant)
    {
      if(m_state.current_anim_state == DoFly)
      {
        if((m_state.creatureInfo->flags & Flying) && m_state.creatureInfo->mood != ai::Mood::Escape
           && aiInfo.zone_number == aiInfo.enemy_zone)
        {
          m_state.creatureInfo->flags &= ~Flying;
        }
        if(!(m_state.creatureInfo->flags & Flying))
        {
          updateMood(getEngine(), m_state, aiInfo, true);
        }
        m_state.creatureInfo->pathFinder.step = 30720_len;
        m_state.creatureInfo->pathFinder.fly = 0_len;
        m_state.creatureInfo->pathFinder.drop = -30720_len;
        aiInfo = ai::AiInfo{getEngine(), m_state};
      }
      else if(m_state.creatureInfo->mood == ai::Mood::Escape
              || (aiInfo.zone_number != aiInfo.enemy_zone && !frontRight && !frontLeft
                  && (!aiInfo.ahead || m_state.creatureInfo->mood == ai::Mood::Bored)))
      {
        m_state.creatureInfo->flags |= Flying;
      }
    }

    if(aiInfo.ahead)
    {
      headRot = aiInfo.angle;
    }
    if(m_state.current_anim_state != DoFly)
    {
      updateMood(getEngine(), m_state, aiInfo, false);
    }
    else if(m_state.creatureInfo->flags & Flying)
    {
      updateMood(getEngine(), m_state, aiInfo, true);
    }

    turnRot = rotateTowardsTarget(m_state.creatureInfo->maximum_turn);
    switch(m_state.current_anim_state.get())
    {
    case DoPrepareAttack.get():
      m_state.creatureInfo->flags &= ~(8u | ShootBullet | ThrowGrenade);
      if(m_state.creatureInfo->flags & Flying)
        m_state.goal_anim_state = DoFly;
      else if((m_state.touch_bits.to_ulong() & 0x678u) || (aiInfo.bite && aiInfo.distance < util::square(300_len)))
        m_state.goal_anim_state = DoHit200;
      else if(aiInfo.bite && aiInfo.distance < util::square(600_len))
        m_state.goal_anim_state = DoHit150;
      else if(frontRight)
        m_state.goal_anim_state = DoShootBullet;
      else if(frontLeft)
        m_state.goal_anim_state = DoThrowGrenade;
      else if(m_state.creatureInfo->mood == ai::Mood::Bored
              || (m_state.creatureInfo->mood == ai::Mood::Stalk && aiInfo.distance < util::square(4608_len)))
        m_state.goal_anim_state = 6;
      else
        m_state.goal_anim_state = DoRun;
      break;
    case DoWalk.get():
      m_state.creatureInfo->maximum_turn = 2_deg;
      if(frontLeft || frontRight || (m_state.creatureInfo->flags & Flying)
         || m_state.creatureInfo->mood == ai::Mood::Attack || m_state.creatureInfo->mood == ai::Mood::Escape)
      {
        m_state.goal_anim_state = DoPrepareAttack;
      }
      else if(m_state.creatureInfo->mood == ai::Mood::Bored
              || (m_state.creatureInfo->mood == ai::Mood::Stalk && aiInfo.zone_number != aiInfo.enemy_zone))
      {
        if(util::rand15() < 80)
        {
          m_state.goal_anim_state = 6_as;
        }
      }
      else if(m_state.creatureInfo->mood == ai::Mood::Stalk && aiInfo.distance > util::square(4608_len))
      {
        m_state.goal_anim_state = DoPrepareAttack;
        break;
      }
      break;
    case DoRun.get():
      m_state.creatureInfo->maximum_turn = 6_deg;
      if((m_state.creatureInfo->flags & Flying) || (m_state.touch_bits.to_ulong() & 0x678u)
         || (aiInfo.bite && aiInfo.distance < util::square(600_len)))
      {
        m_state.goal_anim_state = DoPrepareAttack;
      }
      else if(aiInfo.ahead && aiInfo.distance < util::square(2560_len))
      {
        m_state.goal_anim_state = DoHit100;
      }
      else if(frontLeft || frontRight || m_state.creatureInfo->mood == ai::Mood::Bored
              || (m_state.creatureInfo->mood == ai::Mood::Stalk && aiInfo.distance < util::square(4608_len)))
      {
        m_state.goal_anim_state = DoPrepareAttack;
      }
      break;
    case DoHit150.get():
      if(m_state.required_anim_state == 0_as && (m_state.touch_bits.to_ulong() & 0x678u))
      {
        emitParticle(core::TRVec{-27_len, 98_len, 0_len}, 10, &createBloodSplat);
        getEngine().getLara().m_state.health -= 150_hp;
        getEngine().getLara().m_state.is_hit = true;
        m_state.required_anim_state = DoPrepareAttack;
      }
      break;
    case 6:
      headRot = 0_deg;
      if(frontRight || frontLeft || (m_state.creatureInfo->flags & Flying))
      {
        m_state.goal_anim_state = DoPrepareAttack;
      }
      else if(m_state.creatureInfo->mood == ai::Mood::Stalk)
      {
        if(aiInfo.distance >= util::square(4608_len))
        {
          m_state.goal_anim_state = DoPrepareAttack;
        }
        else if(aiInfo.zone_number == aiInfo.enemy_zone || util::rand15() < 256)
        {
          m_state.goal_anim_state = DoWalk;
        }
      }
      else if(m_state.creatureInfo->mood == ai::Mood::Bored && util::rand15() < 256)
      {
        m_state.goal_anim_state = DoWalk;
      }
      else if(m_state.creatureInfo->mood == ai::Mood::Escape || m_state.creatureInfo->mood == ai::Mood::Attack)
      {
        m_state.goal_anim_state = DoPrepareAttack;
      }
      break;
    case DoHit100.get():
      if(m_state.required_anim_state == 0_as && (m_state.touch_bits.to_ulong() & 0x678u))
      {
        emitParticle(core::TRVec{-27_len, 98_len, 0_len}, 10, &createBloodSplat);
        getEngine().getLara().m_state.health -= 100_hp;
        getEngine().getLara().m_state.is_hit = true;
        m_state.required_anim_state = DoRun;
      }
      break;
    case DoHit200.get():
      if(m_state.required_anim_state == 0_as && (m_state.touch_bits.to_ulong() & 0x678u))
      {
        emitParticle(core::TRVec{-27_len, 98_len, 0_len}, 10, &createBloodSplat);
        getEngine().getLara().m_state.health -= 200_hp;
        getEngine().getLara().m_state.is_hit = true;
        m_state.required_anim_state = DoPrepareAttack;
      }
      break;
    case DoShootBullet.get():
      m_state.creatureInfo->flags |= (8u | ShootBullet);
      if(frontRight)
      {
        m_state.goal_anim_state = DoAttack;
      }
      else
      {
        m_state.goal_anim_state = DoPrepareAttack;
      }
      break;
    case DoThrowGrenade.get():
      m_state.creatureInfo->flags |= ThrowGrenade;
      if(frontLeft)
      {
        m_state.goal_anim_state = DoAttack;
      }
      else
      {
        m_state.goal_anim_state = DoPrepareAttack;
      }
      break;
    case DoAttack.get():
      if(m_state.creatureInfo->flags & ShootBullet)
      {
        m_state.creatureInfo->flags &= ~ShootBullet;
        emitParticle({-35_len, 269_len, 0_len}, 9, &createMutantBullet);
      }
      else if(m_state.creatureInfo->flags & ThrowGrenade)
      {
        m_state.creatureInfo->flags &= ~ThrowGrenade;
        emitParticle({51_len, 213_len, 0_len}, 14, &createMutantGrenade);
      }
      break;
    case 12: m_state.goal_anim_state = DoPrepareAttack; break;
    case DoFly.get():
      if(!(m_state.creatureInfo->flags & Flying) && m_state.position.position.Y == m_state.floor)
      {
        m_state.goal_anim_state = DoPrepareAttack;
      }
      break;
    }
  }

  if(!(m_state.creatureInfo->flags & 8u))
  {
    m_state.creatureInfo->head_rotation = m_state.creatureInfo->neck_rotation;
  }
  rotateCreatureHead(headRot);
  if(m_state.creatureInfo->flags & 8u)
  {
    m_state.creatureInfo->neck_rotation = 0_deg;
  }
  else
  {
    m_state.creatureInfo->neck_rotation = std::exchange(m_state.creatureInfo->head_rotation, 0_deg);
  }
  if(getSkeleton()->getChildren().size() >= 2)
    getSkeleton()->patchBone(1, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix());
  if(getSkeleton()->getChildren().size() >= 3)
    getSkeleton()->patchBone(2, core::TRRotation{0_deg, m_state.creatureInfo->neck_rotation, 0_deg}.toMatrix());
  animateCreature(turnRot, 0_deg);
}

void CentaurMutant::update()
{
  if(m_state.triggerState == TriggerState::Invisible)
  {
    m_state.triggerState = TriggerState::Active;
  }

  m_state.initCreatureInfo(getEngine());

  core::Angle turnRot = 0_deg;
  core::Angle headRot = 0_deg;
  if(getHealth() > 0_hp)
  {
    ai::AiInfo aiInfo{getEngine(), m_state};
    if(aiInfo.ahead)
    {
      headRot = aiInfo.angle;
    }
    updateMood(getEngine(), m_state, aiInfo, true);
    turnRot = rotateTowardsTarget(4_deg);
    switch(m_state.current_anim_state.get())
    {
    case 1:
      m_state.creatureInfo->neck_rotation = 0_deg;
      if(m_state.required_anim_state != 0_as)
      {
        m_state.goal_anim_state = m_state.required_anim_state;
      }
      else if((aiInfo.bite && aiInfo.distance < util::square(1536_len)) || !canShootAtLara(aiInfo))
      {
        m_state.goal_anim_state = 3_as;
      }
      else
      {
        m_state.goal_anim_state = 4_as;
      }
      break;
    case 2:
      if(m_state.required_anim_state == 0_as)
      {
        m_state.required_anim_state = 4_as;
        m_state.creatureInfo->neck_rotation
          = emitParticle({11_len, 415_len, 41_len}, 13, &createMutantGrenade)->angle.X;
      }
      break;
    case 3:
      if(aiInfo.bite && aiInfo.distance < util::square(1536_len))
      {
        m_state.required_anim_state = 6_as;
        m_state.goal_anim_state = 1_as;
      }
      else if(canShootAtLara(aiInfo))
      {
        m_state.required_anim_state = 4_as;
        m_state.goal_anim_state = 1_as;
      }
      else if(util::rand15() < 96)
      {
        m_state.required_anim_state = 6_as;
        m_state.goal_anim_state = 1_as;
      }
      break;
    case 4:
      if(m_state.required_anim_state != 0_as)
      {
        m_state.goal_anim_state = m_state.required_anim_state;
      }
      else if(canShootAtLara(aiInfo))
      {
        m_state.goal_anim_state = 2_as;
      }
      else
      {
        m_state.goal_anim_state = 1_as;
      }
      break;
    case 6:
      if(m_state.required_anim_state == 0_as)
      {
        if(m_state.touch_bits.to_ulong() & 0x30199u)
        {
          emitParticle({50_len, 30_len, 0_len}, 5, &createBloodSplat);
          getEngine().getLara().m_state.health -= 200_hp;
          getEngine().getLara().m_state.is_hit = true;
          m_state.required_anim_state = 1_as;
        }
      }
      break;
    default: break;
    }
  }
  else if(m_state.current_anim_state != 5_as)
  {
    m_state.current_anim_state = 5_as;
    m_state.anim = &getEngine().findAnimatedModelForType(TR1ItemId::CentaurMutant)->animations[8];
    m_state.frame_number = m_state.anim->firstFrame;
  }

  rotateCreatureHead(headRot);
  getSkeleton()->patchBone(
    11, core::TRRotation{m_state.creatureInfo->neck_rotation, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix());
  animateCreature(turnRot, 0_deg);
  if(m_state.triggerState == TriggerState::Deactivated)
  {
    playSoundEffect(TR1SoundId::Mummy);
    shatterModel(*this, -1, 100_len);
    kill();
    m_state.triggerState = TriggerState::Deactivated;
  }
}

void TorsoBoss::update()
{
  if(m_state.triggerState == TriggerState::Invisible)
  {
    m_state.triggerState = TriggerState::Active;
  }

  m_state.initCreatureInfo(getEngine());

  core::Angle headRot = 0_deg;
  if(getHealth() > 0_hp)
  {
    ai::AiInfo aiInfo{getEngine(), m_state};
    if(aiInfo.ahead)
    {
      headRot = aiInfo.angle;
    }
    updateMood(getEngine(), m_state, aiInfo, true);
    const auto angleToTarget = core::angleFromAtan(m_state.creatureInfo->target.X - m_state.position.position.X,
                                                   m_state.creatureInfo->target.Z - m_state.position.position.Z)
                               - m_state.rotation.Y;
    if(m_state.touch_bits.any())
    {
      getEngine().getLara().m_state.health -= 5_hp;
      getEngine().getLara().m_state.is_hit = true;
    }
    switch(m_state.current_anim_state.get())
    {
    case 1:
      if(getEngine().getLara().m_state.health <= 0_hp)
      {
        break;
      }

      m_state.creatureInfo->flags = 0;
      if(angleToTarget > 45_deg)
      {
        m_state.goal_anim_state = 3_as;
        break;
      }
      if(angleToTarget < -45_deg)
      {
        m_state.goal_anim_state = 2_as;
        break;
      }
      if(aiInfo.distance >= util::square(2600_len))
      {
        m_state.goal_anim_state = 7_as;
      }
      else if(getEngine().getLara().m_state.health > 500_hp)
      {
        if(util::rand15(2) == 0)
        {
          m_state.goal_anim_state = 5_as;
        }
        else
        {
          m_state.goal_anim_state = 4_as;
        }
      }
      else if(aiInfo.distance >= util::square(2250_len))
      {
        m_state.goal_anim_state = 7_as;
      }
      else
      {
        m_state.goal_anim_state = 6_as;
      }
      break;
    case 2:
      if(m_state.creatureInfo->flags == 0)
      {
        m_state.creatureInfo->flags = m_state.frame_number.get();
      }
      else
      {
        const auto frameDelta = m_state.frame_number.get() - m_state.creatureInfo->flags;
        if(frameDelta > 13 && frameDelta < 23)
        {
          m_state.rotation.Y -= 9_deg;
        }
      }

      if(angleToTarget > -45_deg)
      {
        m_state.goal_anim_state = 1_as;
      }
      break;
    case 3:
      if(m_state.creatureInfo->flags == 0)
      {
        m_state.creatureInfo->flags = m_state.frame_number.get();
      }
      else
      {
        const auto frameDelta = m_state.frame_number.get() - m_state.creatureInfo->flags;
        if(frameDelta > 16 && frameDelta < 23)
        {
          m_state.rotation.Y += 14_deg;
        }
      }

      if(angleToTarget < 45_deg)
      {
        m_state.goal_anim_state = 1_as;
      }
      break;
    case 4:
      if(m_state.creatureInfo->flags == 0)
      {
        if(m_state.touch_bits.to_ulong() & 0x3ff8000u)
        {
          getEngine().getLara().m_state.health -= 500_hp;
          getEngine().getLara().m_state.is_hit = true;
          m_state.creatureInfo->flags = 1;
        }
      }
      break;
    case 5:
      if(m_state.creatureInfo->flags == 0)
      {
        if(m_state.touch_bits.to_ulong() & 0x3fffff0u)
        {
          getEngine().getLara().m_state.health -= 500_hp;
          getEngine().getLara().m_state.is_hit = true;
          m_state.creatureInfo->flags = 1;
        }
      }
      break;
    case 6:
      if((m_state.touch_bits.to_ulong() & 0x3ff8000u) || getEngine().getLara().m_state.health <= 0_hp)
      {
        m_state.goal_anim_state = 11_as;
        auto& lara = getEngine().getLara();
        lara.m_state.anim = &getEngine().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations[0];
        lara.m_state.frame_number = m_state.anim->firstFrame;
        lara.setGoalAnimState(LaraStateId::BoulderDeath);
        lara.setCurrentAnimState(LaraStateId::BoulderDeath);
        lara.setCurrentRoom(m_state.position.room);
        lara.m_state.position = m_state.position;
        lara.m_state.rotation = {0_deg, m_state.rotation.Y, 0_deg};
        lara.m_state.health = -1_hp;
        lara.setAir(-1_frame);
        lara.gunType = LaraNode::WeaponId::None;
        lara.setHandStatus(HandStatus::Grabbing);
        lara.m_state.falling = false;
        getEngine().getCameraController().setMode(CameraMode::Fixed);
        getEngine().getCameraController().setEyeCenterDistance(2048_len);
      }
      break;
    case 7:
      // TODO this is just weird, but it's just like the original...
      m_state.goal_anim_state = m_state.goal_anim_state.get() + util::clamp(angleToTarget, -3_deg, 3_deg).get();

      if(abs(angleToTarget) > 45_deg || aiInfo.distance < util::square(2600_len))
      {
        m_state.goal_anim_state = 1_as;
      }

      break;
    case 8:
      m_state.goal_anim_state = 9_as;
      m_state.falling = true;
      break;
    case 11:
      getEngine().getCameraController().setEyeCenterDistance(2048_len);
      getEngine().getCameraController().setMode(CameraMode::Fixed);
      break;
    default: break;
    }
  }
  else if(m_state.current_anim_state != 10_as)
  {
    m_state.anim = &getEngine().findAnimatedModelForType(TR1ItemId::TorsoBoss)->animations[13];
    m_state.frame_number = m_state.anim->firstFrame;
    m_state.current_anim_state = 10_as;
  }
  rotateCreatureHead(headRot);
  if(m_state.current_anim_state == 9_as)
  {
    ModelItemNode::update();
    if(m_state.position.position.Y > m_state.floor)
    {
      m_state.falling = false;
      m_state.goal_anim_state = 1_as;
      m_state.position.position.Y = m_state.floor;
      getEngine().getCameraController().setBounce(500_len);
    }
  }
  else
  {
    animateCreature(0_deg, 0_deg);
  }
  if(m_state.triggerState == TriggerState::Deactivated)
  {
    playSoundEffect(TR1SoundId::Mummy);
    shatterModel(*this, -1, 250_len);
    const auto sector = loader::file::findRealFloorSector(m_state.position);
    getEngine().getLara().handleCommandSequence(
      HeightInfo::fromFloor(sector, m_state.position.position, getEngine().getItemNodes()).lastCommandSequenceOrDeath,
      true);
    kill();
    m_state.triggerState = TriggerState::Deactivated;
  }
}
} // namespace items
} // namespace engine
