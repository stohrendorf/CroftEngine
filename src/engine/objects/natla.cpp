#include "natla.h"

#include "aiagent.h"
#include "core/angle.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/vec.h"
#include "engine/ai/ai.h"
#include "engine/ai/pathfinder.h"
#include "engine/audioengine.h"
#include "engine/engine.h"
#include "engine/floordata/floordata.h"
#include "engine/location.h"
#include "engine/particle.h"
#include "engine/script/scriptengine.h"
#include "engine/soundeffects_tr1.h"
#include "engine/tracks_tr1.h"
#include "engine/world/world.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "util/helpers.h"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <utility>

namespace engine::objects
{
void Natla::update()
{
  static constexpr const core::TRVec bulletEmissionPos{5_len, 220_len, 7_len};
  static constexpr const size_t bulletEmissionBoneIdx = 4;

  activateAi();

  static const constexpr auto AimDispatch = 1_as;
  static const constexpr auto AimFlying = 2_as;
  static const constexpr auto Running = 3_as;
  static const constexpr auto AimWalking = 4_as;
  static const constexpr auto Injured = 5_as;
  static const constexpr auto Shoot = 6_as;
  static const constexpr auto Falling = 7_as;
  static const constexpr auto Standing = 8_as;
  static const constexpr auto Dying = 9_as;

  auto tiltRot = 0_deg;
  auto turn = 0_deg;
  auto neckRot = 0_deg;
  auto headRot = getCreatureInfo()->headRotation * 7 / 8;
  if(m_state.health <= 0_hp && m_state.health > core::DeadHealth)
  {
    goal(Dying);
  }
  else if(m_state.health > 200_hp)
  {
    getCreatureInfo()->pathFinder.setLimits(getWorld(), m_state.getCurrentBox(), 256_len, -256_len, 0_len);
    ai::EnemyLocation enemyLocation{*this};

    const auto canShoot = abs(enemyLocation.visualAngleToLara) < 30_deg && canShootAtLara(enemyLocation);
    if(m_state.current_anim_state == AimFlying && m_attemptToFly)
    {
      if(canShoot && util::rand15() < 256)
        m_attemptToFly = false;

      if(!m_attemptToFly)
        ai::updateMood(*this, enemyLocation, true);

      getCreatureInfo()->pathFinder.setLimits(getWorld(), m_state.getCurrentBox(), 20_sectors, -20_sectors, 32_len);
      enemyLocation = ai::EnemyLocation{*this};
    }
    else
    {
      if(!canShoot)
      {
        m_attemptToFly = true;
      }
    }

    if(enemyLocation.laraInView)
      neckRot = enemyLocation.visualAngleToLara;

    if(m_state.current_anim_state != AimFlying || m_attemptToFly)
      ai::updateMood(*this, enemyLocation, false);

    m_state.rotation.Y -= m_pitchDelta;
    turn = rotateTowardsTarget(5_deg / 1_frame);
    if(m_state.current_anim_state == AimFlying)
    {
      m_pitchDelta += std::clamp(enemyLocation.visualAngleToLara, -5_deg, 5_deg);
      m_state.rotation.Y += m_pitchDelta;
    }
    else
    {
      m_state.rotation.Y += std::exchange(m_pitchDelta, 0_deg) - turn;
    }

    switch(m_state.current_anim_state.get())
    {
    case AimDispatch.get():
      m_flyTime = 0_frame;
      goal(m_attemptToFly ? AimFlying : AimWalking);
      break;
    case AimFlying.get():
      if(!m_attemptToFly && m_state.location.position.Y == m_state.floor)
        goal(AimDispatch);

      if(m_flyTime >= 1_sec * core::FrameRate)
      {
        auto particle = emitParticle(bulletEmissionPos, bulletEmissionBoneIdx, &createMutantGrenade);
        headRot = particle->angle.X;
        getWorld().getAudioEngine().playSoundEffect(TR1SoundEffect::MutantShootingBullet,
                                                    particle->location.position.toRenderSystem());
        m_flyTime = 0_frame;
      }
      break;
    case AimWalking.get():
      if(m_state.required_anim_state != 0_as)
        goal(m_state.required_anim_state);
      else
        goal(canShoot ? Shoot : AimDispatch);
      break;
    case Shoot.get():
      if(m_state.required_anim_state == 0_as)
      {
        auto particle = emitParticle(bulletEmissionPos, bulletEmissionBoneIdx, &createMutantGrenade);
        headRot = particle->angle.X;
        particle = emitParticle(bulletEmissionPos, bulletEmissionBoneIdx, &createMutantGrenade);
        particle->angle.Y += util::rand15s(45_deg);
        particle = emitParticle(bulletEmissionPos, bulletEmissionBoneIdx, &createMutantGrenade);
        particle->angle.Y += util::rand15s(45_deg);
        require(AimDispatch);
      }
      break;
    default:
      break;
    }
  }
  else
  {
    getCreatureInfo()->pathFinder.setLimits(getWorld(), m_state.getCurrentBox(), 256_len, -256_len, 0_len);
    const ai::EnemyLocation enemyLocation{*this};

    if(enemyLocation.laraInView)
      neckRot = enemyLocation.visualAngleToLara;

    ai::updateMood(*this, enemyLocation, true);
    turn = rotateTowardsTarget(6_deg / 1_frame);
    const auto canShoot = abs(enemyLocation.visualAngleToLara) < 30_deg && canShootAtLara(enemyLocation);
    m_state.rotation.Y += std::exchange(m_pitchDelta, 0_deg);
    switch(m_state.current_anim_state.get())
    {
    case AimDispatch.get():
    case AimWalking.get():
    case Shoot.get():
      goal(Injured);
      m_state.activationState.fullyDeactivate();
      m_flyTime = 0_frame;
      break;
    case AimFlying.get():
      goal(Falling);
      m_flyTime = 0_frame;
      break;
    case Running.get():
      tiltRot = turn;
      if(m_flyTime >= 1_sec * core::FrameRate * 2 / 3)
      {
        auto particle = emitParticle(bulletEmissionPos, bulletEmissionBoneIdx, &createMutantBullet);
        headRot = particle->angle.X;
        getWorld().getAudioEngine().playSoundEffect(TR1SoundEffect::MutantShootingBullet,
                                                    particle->location.position.toRenderSystem());
        m_flyTime = 0_frame;
      }

      if(canShoot)
        goal(Standing);
      break;
    case Injured.get():
      if(m_flyTime == 16_sec * core::FrameRate)
      {
        goal(Standing);
        m_attemptToFly = false;
        m_flyTime = 0_frame;
        m_state.health = 200_hp;
        getWorld().getAudioEngine().playStopCdTrack(
          getWorld().getEngine().getScriptEngine().getGameflow(), TR1TrackId::LaraTalk28, false);
      }
      else
      {
        m_state.health = core::DeadHealth;
      }
      break;
    case Falling.get():
      if(m_state.location.position.Y >= m_state.floor)
      {
        m_state.falling = false;
        goal(Injured);
        m_state.location.position.Y = m_state.floor;
        m_flyTime = 0_frame;
      }
      else
      {
        m_state.falling = true;
        m_state.speed = 0_spd;
      }
      break;
    case Standing.get():
      if(!canShoot)
        goal(Running);

      if(m_flyTime >= 1_sec * core::FrameRate * 2 / 3)
      {
        auto particle = emitParticle(bulletEmissionPos, bulletEmissionBoneIdx, &createMutantBullet);
        headRot = particle->angle.X;
        getWorld().getAudioEngine().playSoundEffect(TR1SoundEffect::MutantShootingBullet,
                                                    particle->location.position.toRenderSystem());
        m_flyTime = 0_frame;
      }
      break;
    default:
      break;
    }
  }

  rotateCreatureTilt(tiltRot);
  getCreatureInfo()->neckRotation = -neckRot;
  if(headRot != 0_deg)
    getCreatureInfo()->headRotation = headRot;
  m_flyTime += 1_frame;
  m_state.rotation.Y -= m_pitchDelta;
  animateCreature(turn, 0_deg);
  m_state.rotation.Y += m_pitchDelta;
}

void Natla::serialize(const serialization::Serializer<world::World>& ser) const
{
  AIAgent::serialize(ser);
  ser(S_NV("attemptToFly", m_attemptToFly), S_NV("flyTime", m_flyTime), S_NV("pitchDelta", m_pitchDelta));
}

void Natla::deserialize(const serialization::Deserializer<world::World>& ser)
{
  AIAgent::deserialize(ser);
  ser(S_NV("attemptToFly", m_attemptToFly), S_NV("flyTime", m_flyTime), S_NV("pitchDelta", m_pitchDelta));
}
} // namespace engine::objects
