#include "laraobject.h"

#include "block.h"
#include "engine/cameracontroller.h"
#include "engine/lara/abstractstatehandler.h"
#include "engine/particle.h"
#include "engine/tracks_tr1.h"
#include "hid/inputhandler.h"
#include "render/textureanimator.h"
#include "serialization/animframe_ptr.h"
#include "serialization/objectreference.h"
#include "serialization/optional.h"
#include "serialization/quantity.h"
#include "serialization/unordered_map.h"

#include <boost/range/adaptors.hpp>
#include <glm/gtx/norm.hpp>
#include <stack>

namespace
{
core::TRRotationXY getVectorAngles(const core::TRVec& co)
{
  return core::getVectorAngles(co.X, co.Y, co.Z);
}
} // namespace

namespace engine::objects
{
void LaraObject::setAnimation(AnimationId anim, const std::optional<core::Frame>& firstFrame)
{
  getSkeleton()->setAnimation(m_state, &getEngine().getAnimation(anim), firstFrame.value_or(0_frame));
}

void LaraObject::handleLaraStateOnLand()
{
  CollisionInfo collisionInfo;
  collisionInfo.oldPosition = m_state.position.position;
  collisionInfo.collisionRadius = core::DefaultCollisionRadius;
  collisionInfo.policyFlags = CollisionInfo::SpazPushPolicy;

  lara::AbstractStateHandler::create(getCurrentAnimState(), *this)->handleInput(collisionInfo);

  if(getEngine().getCameraController().getMode() != CameraMode::FreeLook)
  {
    const auto headX = m_headRotation.X;
    if(headX <= -2_deg || headX >= 2_deg)
    {
      m_headRotation.X = headX - headX / 8;
    }
    else
    {
      m_headRotation.X = 0_deg;
    }
    const auto headY = m_headRotation.Y;
    if(headY <= -2_deg || headY >= 2_deg)
    {
      m_headRotation.Y = headY - headY / 8;
    }
    else
    {
      m_headRotation.Y = 0_deg;
    }
    m_torsoRotation = m_headRotation;
  }

  // "slowly" revert rotations to zero
  if(m_state.rotation.Z < -1_deg)
  {
    m_state.rotation.Z += 1_deg;
    if(m_state.rotation.Z >= 0_deg)
    {
      m_state.rotation.Z = 0_deg;
    }
  }
  else if(m_state.rotation.Z > 1_deg)
  {
    m_state.rotation.Z -= 1_deg;
    if(m_state.rotation.Z <= 0_deg)
    {
      m_state.rotation.Z = 0_deg;
    }
  }
  else
  {
    m_state.rotation.Z = 0_deg;
  }

  if(m_yRotationSpeed > 2_deg)
  {
    m_yRotationSpeed -= 2_deg;
  }
  else if(m_yRotationSpeed < -2_deg)
  {
    m_yRotationSpeed += 2_deg;
  }
  else
  {
    m_yRotationSpeed = 0_deg;
  }

  m_state.rotation.Y += m_yRotationSpeed;

  updateImpl();

  testInteractions(collisionInfo);

  lara::AbstractStateHandler::create(getCurrentAnimState(), *this)->postprocessFrame(collisionInfo);

  updateFloorHeight(-381_len);

  updateLarasWeaponsStatus();
  getEngine().handleCommandSequence(collisionInfo.mid.floorSpace.lastCommandSequenceOrDeath, false);

  applyTransform();

#ifndef NDEBUG
  lastUsedCollisionInfo = collisionInfo;
#endif
}

void LaraObject::handleLaraStateDiving()
{
  CollisionInfo collisionInfo;
  collisionInfo.oldPosition = m_state.position.position;
  collisionInfo.collisionRadius = core::DefaultCollisionRadiusUnderwater;
  collisionInfo.policyFlags.reset_all();
  collisionInfo.badCeilingDistance = core::LaraDiveHeight;
  collisionInfo.badPositiveDistance = core::HeightLimit;
  collisionInfo.badNegativeDistance = -core::LaraDiveHeight;

  lara::AbstractStateHandler::create(getCurrentAnimState(), *this)->handleInput(collisionInfo);

  // "slowly" revert rotations to zero
  if(m_state.rotation.Z < -2_deg)
  {
    m_state.rotation.Z += +2_deg;
  }
  else
  {
    if(m_state.rotation.Z > 2_deg)
    {
      m_state.rotation.Z += -2_deg;
    }
    else
    {
      m_state.rotation.Z = 0_deg;
    }
  }
  const core::Angle x = util::clamp(m_state.rotation.X, -100_deg, +100_deg);
  m_state.rotation.X = x;
  const core::Angle z = util::clamp(m_state.rotation.Z, -22_deg, +22_deg);
  m_state.rotation.Z = z;

  if(m_underwaterCurrentStrength != 0_len)
  {
    handleUnderwaterCurrent(collisionInfo);
  }

  updateImpl();

  m_state.position.position += util::yawPitch(m_state.fallspeed * 1_frame / 4, m_state.rotation);

  testInteractions(collisionInfo);

  lara::AbstractStateHandler::create(getCurrentAnimState(), *this)->postprocessFrame(collisionInfo);

  updateFloorHeight(0_len);
  updateLarasWeaponsStatus();
  getEngine().handleCommandSequence(collisionInfo.mid.floorSpace.lastCommandSequenceOrDeath, false);
#ifndef NDEBUG
  lastUsedCollisionInfo = collisionInfo;
#endif
}

void LaraObject::handleLaraStateSwimming()
{
  CollisionInfo collisionInfo;
  collisionInfo.oldPosition = m_state.position.position;
  collisionInfo.collisionRadius = core::DefaultCollisionRadius;
  collisionInfo.policyFlags.reset_all();
  collisionInfo.badCeilingDistance = core::DefaultCollisionRadius;
  collisionInfo.badPositiveDistance = core::HeightLimit;
  collisionInfo.badNegativeDistance = -core::DefaultCollisionRadius;

  setCameraRotationAroundLaraX(-22_deg);

  lara::AbstractStateHandler::create(getCurrentAnimState(), *this)->handleInput(collisionInfo);

  // "slowly" revert rotations to zero
  if(m_state.rotation.Z < 0_deg)
  {
    m_state.rotation.Z += +2_deg;
  }
  else if(m_state.rotation.Z > 2_deg)
  {
    m_state.rotation.Z += -2_deg;
  }
  else
  {
    m_state.rotation.Z = 0_deg;
  }

  if(getEngine().getCameraController().getMode() != CameraMode::FreeLook)
  {
    m_headRotation.X -= m_headRotation.X / 8;
    m_headRotation.Y -= m_headRotation.Y / 8;
    m_torsoRotation.X = 0_deg;
    m_torsoRotation.Y /= 2;
  }

  if(m_underwaterCurrentStrength != 0_len)
  {
    handleUnderwaterCurrent(collisionInfo);
  }

  updateImpl();

  move(util::pitch(m_state.fallspeed * 1_frame / 4, getMovementAngle()).toRenderSystem());

  testInteractions(collisionInfo);

  lara::AbstractStateHandler::create(getCurrentAnimState(), *this)->postprocessFrame(collisionInfo);

  updateFloorHeight(core::DefaultCollisionRadius);
  updateLarasWeaponsStatus();
  getEngine().handleCommandSequence(collisionInfo.mid.floorSpace.lastCommandSequenceOrDeath, false);
#ifndef NDEBUG
  lastUsedCollisionInfo = collisionInfo;
#endif
}

void LaraObject::placeOnFloor(const CollisionInfo& collisionInfo)
{
  m_state.position.position.Y += collisionInfo.mid.floorSpace.y;
}

LaraObject::~LaraObject() = default;

void LaraObject::update()
{
  if(getEngine().getInputHandler().getInputState()._1.justPressed())
    getEngine().getInventory().tryUse(TR1ItemId::Pistols);
  else if(getEngine().getInputHandler().getInputState()._2.justPressed())
    getEngine().getInventory().tryUse(TR1ItemId::Shotgun);
  else if(getEngine().getInputHandler().getInputState()._3.justPressed())
    getEngine().getInventory().tryUse(TR1ItemId::Uzis);
  else if(getEngine().getInputHandler().getInputState()._4.justPressed())
    getEngine().getInventory().tryUse(TR1ItemId::Magnums);
  else if(getEngine().getInputHandler().getInputState()._5.justPressed())
    getEngine().getInventory().tryUse(TR1ItemId::SmallMedipack);
  else if(getEngine().getInputHandler().getInputState()._6.justPressed())
    getEngine().getInventory().tryUse(TR1ItemId::LargeMedipack);

  if(m_underwaterState == UnderwaterState::OnLand && m_state.position.room->isWaterRoom())
  {
    m_air = core::LaraAir;
    m_underwaterState = UnderwaterState::Diving;
    m_state.falling = false;
    m_state.position.position.Y += 100_len;
    updateFloorHeight(0_len);
    getEngine().getAudioEngine().stopSound(TR1SoundId::LaraScream, &m_state);
    if(getCurrentAnimState() == LaraStateId::SwandiveBegin)
    {
      m_state.rotation.X = -45_deg;
      setGoalAnimState(LaraStateId::UnderwaterDiving);
      updateImpl();
      m_state.fallspeed *= 2;
    }
    else if(getCurrentAnimState() == LaraStateId::SwandiveEnd)
    {
      m_state.rotation.X = -85_deg;
      setGoalAnimState(LaraStateId::UnderwaterDiving);
      updateImpl();
      m_state.fallspeed *= 2;
    }
    else
    {
      m_state.rotation.X = -45_deg;
      setAnimation(AnimationId::FREE_FALL_TO_UNDERWATER, 1895_frame);
      setGoalAnimState(LaraStateId::UnderwaterForward);
      m_state.fallspeed += m_state.fallspeed / 2;
    }

    resetHeadTorsoRotation();

    if(const auto waterSurfaceHeight = getWaterSurfaceHeight())
    {
      playSoundEffect(TR1SoundId::LaraFallIntoWater);

      auto room = m_state.position.room;
      findRealFloorSector(m_state.position.position, &room);
      for(int i = 0; i < 10; ++i)
      {
        core::RoomBoundPosition surfacePos{room};
        surfacePos.position.X = m_state.position.position.X;
        surfacePos.position.Y = *waterSurfaceHeight;
        surfacePos.position.Z = m_state.position.position.Z;

        auto particle = std::make_shared<SplashParticle>(surfacePos, getEngine(), false);
        setParent(particle, surfacePos.room->node);
        getEngine().getParticles().emplace_back(particle);
      }
    }
  }
  else if(m_underwaterState == UnderwaterState::Diving && !m_state.position.room->isWaterRoom())
  {
    auto waterSurfaceHeight = getWaterSurfaceHeight();
    m_state.fallspeed = 0_spd;
    m_state.rotation.X = 0_deg;
    m_state.rotation.Z = 0_deg;
    resetHeadTorsoRotation();
    m_handStatus = HandStatus::None;

    if(!waterSurfaceHeight || abs(*waterSurfaceHeight - m_state.position.position.Y) >= core::QuarterSectorSize)
    {
      m_underwaterState = UnderwaterState::OnLand;
      setAnimation(AnimationId::FREE_FALL_FORWARD, 492_frame);
      setGoalAnimState(LaraStateId::JumpForward);
      m_state.speed = std::exchange(m_state.fallspeed, 0_spd) / 4;
      m_state.falling = true;
    }
    else
    {
      m_underwaterState = UnderwaterState::Swimming;
      setAnimation(AnimationId::UNDERWATER_TO_ONWATER, 1937_frame);
      setGoalAnimState(LaraStateId::OnWaterStop);
      m_state.position.position.Y = *waterSurfaceHeight + 1_len;
      m_swimToDiveKeypressDuration = 11_frame;
      updateFloorHeight(-381_len);
      playSoundEffect(TR1SoundId::LaraCatchingAir);
    }
  }
  else if(m_underwaterState == UnderwaterState::Swimming && !m_state.position.room->isWaterRoom())
  {
    m_underwaterState = UnderwaterState::OnLand;
    setAnimation(AnimationId::FREE_FALL_FORWARD, 492_frame);
    setGoalAnimState(LaraStateId::JumpForward);
    m_state.speed = std::exchange(m_state.fallspeed, 0_spd) / 4;
    m_state.falling = true;
    m_handStatus = HandStatus::None;
    m_state.rotation.X = 0_deg;
    m_state.rotation.Z = 0_deg;
    resetHeadTorsoRotation();
  }

  if(m_underwaterState == UnderwaterState::OnLand)
  {
    m_air = core::LaraAir;
    handleLaraStateOnLand();
  }
  else if(m_underwaterState == UnderwaterState::Diving)
  {
    if(m_state.health >= 0_hp)
    {
      m_air -= 1_frame;
      if(m_air < 0_frame)
      {
        m_air = -1_frame;
        m_state.health -= 5_hp;
      }
    }
    handleLaraStateDiving();
  }
  else if(m_underwaterState == UnderwaterState::Swimming)
  {
    if(m_state.health >= 0_hp)
    {
      m_air = std::min(m_air + 10_frame, core::LaraAir);
    }
    handleLaraStateSwimming();
  }
}

void LaraObject::updateImpl()
{
  const auto endOfAnim = getSkeleton()->advanceFrame(m_state);

  Expects(getSkeleton()->anim != nullptr);
  if(endOfAnim)
  {
    if(getSkeleton()->anim->animCommandCount > 0)
    {
      const auto* cmd = &getEngine().getAnimCommands().at(getSkeleton()->anim->animCommandIndex);
      for(uint16_t i = 0; i < getSkeleton()->anim->animCommandCount; ++i)
      {
        Expects(cmd < &getEngine().getAnimCommands().back());
        const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
        ++cmd;
        switch(opcode)
        {
        case AnimCommandOpcode::SetPosition:
          moveLocal(core::TRVec{core::Length{static_cast<core::Length::type>(cmd[0])},
                                core::Length{static_cast<core::Length::type>(cmd[1])},
                                core::Length{static_cast<core::Length::type>(cmd[2])}});
          cmd += 3;
          break;
        case AnimCommandOpcode::StartFalling:
          if(m_fallSpeedOverride != 0_spd)
          {
            m_state.fallspeed = std::exchange(m_fallSpeedOverride, 0_spd);
          }
          else
          {
            m_state.fallspeed = core::Speed{static_cast<core::Speed::type>(cmd[0])};
          }
          m_state.speed = core::Speed{static_cast<core::Speed::type>(cmd[1])};
          m_state.falling = true;
          cmd += 2;
          break;
        case AnimCommandOpcode::EmptyHands: setHandStatus(HandStatus::None); break;
        case AnimCommandOpcode::PlaySound: cmd += 2; break;
        case AnimCommandOpcode::PlayEffect: cmd += 2; break;
        default: break;
        }
      }
    }

    getSkeleton()->setAnimation(m_state, getSkeleton()->anim->nextAnimation, getSkeleton()->anim->nextFrame);
  }

  if(getSkeleton()->anim->animCommandCount > 0)
  {
    const auto* cmd = &getEngine().getAnimCommands().at(getSkeleton()->anim->animCommandIndex);
    for(uint16_t i = 0; i < getSkeleton()->anim->animCommandCount; ++i)
    {
      Expects(cmd < &getEngine().getAnimCommands().back());
      const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
      ++cmd;
      switch(opcode)
      {
      case AnimCommandOpcode::SetPosition: cmd += 3; break;
      case AnimCommandOpcode::StartFalling: cmd += 2; break;
      case AnimCommandOpcode::PlaySound:
        if(getSkeleton()->frame_number.get() == cmd[0])
        {
          playSoundEffect(static_cast<TR1SoundId>(cmd[1]));
        }
        cmd += 2;
        break;
      case AnimCommandOpcode::PlayEffect:
        if(getSkeleton()->frame_number.get() == cmd[0])
        {
          BOOST_LOG_TRIVIAL(debug) << "Anim effect: " << int(cmd[1]);
          getEngine().runEffect(cmd[1], this);
        }
        cmd += 2;
        break;
      default: break;
      }
    }
  }

  applyMovement(true);

  drawRoutine();
}

void LaraObject::updateFloorHeight(const core::Length& dy)
{
  auto pos = m_state.position.position;
  pos.Y += dy;
  auto room = m_state.position.room;
  const auto sector = findRealFloorSector(pos, &room);
  setCurrentRoom(room);
  const HeightInfo hi = HeightInfo::fromFloor(sector, pos, getEngine().getObjects());
  m_state.floor = hi.y;
}

void LaraObject::setCameraRotationAroundLara(const core::Angle& x, const core::Angle& y)
{
  getEngine().getCameraController().setRotationAroundLara(x, y);
}

void LaraObject::setCameraRotationAroundLaraY(const core::Angle& y)
{
  getEngine().getCameraController().setRotationAroundLaraY(y);
}

void LaraObject::setCameraRotationAroundLaraX(const core::Angle& x)
{
  getEngine().getCameraController().setRotationAroundLaraX(x);
}

void LaraObject::setCameraDistance(const core::Length& d)
{
  getEngine().getCameraController().setDistance(d);
}

void LaraObject::setCameraModifier(const CameraModifier k)
{
  getEngine().getCameraController().setModifier(k);
}

void LaraObject::testInteractions(CollisionInfo& collisionInfo)
{
  m_state.is_hit = false;
  hit_direction.reset();

  if(m_state.health < 0_hp)
    return;

  std::set<gsl::not_null<const loader::file::Room*>> rooms;
  rooms.insert(m_state.position.room);
  for(const loader::file::Portal& p : m_state.position.room->portals)
    rooms.insert(&getEngine().getRooms().at(p.adjoining_room.get()));

  for(const auto& object : getEngine().getObjects() | boost::adaptors::map_values)
  {
    if(rooms.find(object->m_state.position.room) == rooms.end())
      continue;

    if(!object->m_state.collidable)
      continue;

    if(object->m_state.triggerState == TriggerState::Invisible)
      continue;

    const auto d = m_state.position.position - object->m_state.position.position;
    if(abs(d.X) >= 4 * core::SectorSize || abs(d.Y) >= 4 * core::SectorSize || abs(d.Z) >= 4 * core::SectorSize)
      continue;

    object->collide(collisionInfo);
  }

  for(const auto& object : getEngine().getDynamicObjects())
  {
    if(rooms.find(object->m_state.position.room) == rooms.end())
      continue;

    if(!object->m_state.collidable)
      continue;

    if(object->m_state.triggerState == TriggerState::Invisible)
      continue;

    const auto d = m_state.position.position - object->m_state.position.position;
    if(abs(d.X) >= 4 * core::SectorSize || abs(d.Y) >= 4 * core::SectorSize || abs(d.Z) >= 4 * core::SectorSize)
      continue;

    object->collide(collisionInfo);
  }

  if(getEngine().getLara().explosionStumblingDuration != 0_frame)
  {
    getEngine().getLara().updateExplosionStumbling();
  }
  if(!getEngine().getLara().hit_direction.has_value())
  {
    getEngine().getLara().hit_frame = 0_frame;
  }
  // TODO selectedPuzzleKey = -1;
}

void LaraObject::handleUnderwaterCurrent(CollisionInfo& collisionInfo)
{
  m_state.box = m_state.getCurrentSector()->box;
  core::TRVec targetPos;
  if(!m_underwaterRoute.calculateTarget(getEngine(), targetPos, m_state))
    return;

  targetPos -= m_state.position.position;
  m_state.position.position.X += util::clamp(targetPos.X, -m_underwaterCurrentStrength, m_underwaterCurrentStrength);
  m_state.position.position.Y += util::clamp(targetPos.Y, -m_underwaterCurrentStrength, m_underwaterCurrentStrength);
  m_state.position.position.Z += util::clamp(targetPos.Z, -m_underwaterCurrentStrength, m_underwaterCurrentStrength);

  m_underwaterCurrentStrength = 0_len;
  collisionInfo.facingAngle = angleFromAtan(m_state.position.position.X - collisionInfo.oldPosition.X,
                                            m_state.position.position.Z - collisionInfo.oldPosition.Z);

  collisionInfo.initHeightInfo(m_state.position.position + core::TRVec{0_len, core::LaraDiveGroundElevation, 0_len},
                               getEngine(),
                               core::LaraDiveHeight);
  if(collisionInfo.collisionType == CollisionInfo::AxisColl::Front)
  {
    if(m_state.rotation.X > 35_deg)
      m_state.rotation.X += 2_deg;
    else if(m_state.rotation.X < -35_deg)
      m_state.rotation.X -= 2_deg;
  }
  else if(collisionInfo.collisionType == CollisionInfo::AxisColl::Top)
    m_state.rotation.X -= 2_deg;
  else if(collisionInfo.collisionType == CollisionInfo::AxisColl::TopBottom)
    m_state.fallspeed = 0_spd;
  else if(collisionInfo.collisionType == CollisionInfo::AxisColl::Left)
    m_state.rotation.Y += 5_deg;
  else if(collisionInfo.collisionType == CollisionInfo::AxisColl::Right)
    m_state.rotation.Y -= 5_deg;

  if(collisionInfo.mid.floorSpace.y < 0_len)
  {
    m_state.position.position.Y += collisionInfo.mid.floorSpace.y;
    m_state.rotation.X += 2_deg;
  }
  applyShift(collisionInfo);
  collisionInfo.oldPosition = m_state.position.position;
}

void LaraObject::updateLarasWeaponsStatus()
{
  if(leftArm.flashTimeout > 0_frame)
  {
    leftArm.flashTimeout -= 1_frame;
  }
  if(rightArm.flashTimeout > 0_frame)
  {
    rightArm.flashTimeout -= 1_frame;
  }
  bool doHolsterUpdate = false;
  if(m_state.health <= 0_hp)
  {
    m_handStatus = HandStatus::None;
  }
  else
  {
    if(m_underwaterState != UnderwaterState::OnLand)
    {
      if(m_handStatus == HandStatus::Combat)
      {
        doHolsterUpdate = true;
      }
    }
    else if(requestedGunType == gunType)
    {
      if(getEngine().getInputHandler().getInputState().holster.justPressed())
      {
        doHolsterUpdate = true;
      }
    }
    else if(m_handStatus == HandStatus::Combat)
    {
      doHolsterUpdate = true;
    }
    else if(m_handStatus == HandStatus::None)
    {
      gunType = requestedGunType;
      unholster();
      doHolsterUpdate = true;
    }
  }

  if(doHolsterUpdate && gunType != WeaponId::None)
  {
    if(m_handStatus == HandStatus::None)
    {
      rightArm.frame = 0_frame;
      leftArm.frame = 0_frame;
      m_handStatus = HandStatus::Unholster;
    }
    else if(m_handStatus == HandStatus::Combat)
    {
      m_handStatus = HandStatus::Holster;
    }
  }

  if(m_handStatus == HandStatus::Unholster)
  {
    if(gunType >= WeaponId::Pistols)
    {
      if(gunType <= WeaponId::Uzi)
      {
        if(getEngine().getCameraController().getMode() != CameraMode::Cinematic
           && getEngine().getCameraController().getMode() != CameraMode::FreeLook)
        {
          getEngine().getCameraController().setMode(CameraMode::Combat);
        }
        unholsterGuns(gunType);
      }
      else if(gunType == WeaponId::Shotgun)
      {
        if(getEngine().getCameraController().getMode() != CameraMode::Cinematic
           && getEngine().getCameraController().getMode() != CameraMode::FreeLook)
        {
          getEngine().getCameraController().setMode(CameraMode::Combat);
        }
        unholsterShotgun();
      }
    }
  }
  else if(m_handStatus == HandStatus::Holster)
  {
    {
      const auto& normalLara = *getEngine().findAnimatedModelForType(TR1ItemId::Lara);
      BOOST_ASSERT(normalLara.bones.size() == getSkeleton()->getBoneCount());
      getSkeleton()->setMeshPart(14, normalLara.bones[14].mesh);
      getSkeleton()->rebuildMesh();
    }

    if(gunType >= WeaponId::Pistols)
    {
      if(gunType <= WeaponId::Uzi)
      {
        holsterGuns(gunType);
      }
      else if(gunType == WeaponId::Shotgun)
      {
        holsterShotgun();
      }
    }
  }
  else if(m_handStatus == HandStatus::Combat)
  {
    {
      const auto& normalLara = *getEngine().findAnimatedModelForType(TR1ItemId::Lara);
      BOOST_ASSERT(normalLara.bones.size() == getSkeleton()->getBoneCount());
      getSkeleton()->setMeshPart(14, normalLara.bones[14].mesh);
      getSkeleton()->rebuildMesh();
    }

    switch(gunType)
    {
    case WeaponId::Pistols:
      if(pistolsAmmo.ammo != 0)
      {
        if(getEngine().getInputHandler().getInputState().action)
        {
          const auto& uziLara = *getEngine().findAnimatedModelForType(TR1ItemId::LaraUzisAnim);
          BOOST_ASSERT(uziLara.bones.size() == getSkeleton()->getBoneCount());
          getSkeleton()->setMeshPart(14, uziLara.bones[14].mesh);
          getSkeleton()->rebuildMesh();
        }
      }
      if(getEngine().getCameraController().getMode() != CameraMode::Cinematic
         && getEngine().getCameraController().getMode() != CameraMode::FreeLook)
      {
        getEngine().getCameraController().setMode(CameraMode::Combat);
      }
      updateGuns(gunType);
      break;
    case WeaponId::AutoPistols:
      if(revolverAmmo.ammo != 0)
      {
        if(getEngine().getInputHandler().getInputState().action)
        {
          const auto& uziLara = *getEngine().findAnimatedModelForType(TR1ItemId::LaraUzisAnim);
          BOOST_ASSERT(uziLara.bones.size() == getSkeleton()->getBoneCount());
          getSkeleton()->setMeshPart(14, uziLara.bones[14].mesh);
          getSkeleton()->rebuildMesh();
        }
      }
      if(getEngine().getCameraController().getMode() != CameraMode::Cinematic
         && getEngine().getCameraController().getMode() != CameraMode::FreeLook)
      {
        getEngine().getCameraController().setMode(CameraMode::Combat);
      }
      updateGuns(gunType);
      break;
    case WeaponId::Uzi:
      if(uziAmmo.ammo != 0)
      {
        if(getEngine().getInputHandler().getInputState().action)
        {
          const auto& uziLara = *getEngine().findAnimatedModelForType(TR1ItemId::LaraUzisAnim);
          BOOST_ASSERT(uziLara.bones.size() == getSkeleton()->getBoneCount());
          getSkeleton()->setMeshPart(14, uziLara.bones[14].mesh);
          getSkeleton()->rebuildMesh();
        }
      }
      if(getEngine().getCameraController().getMode() != CameraMode::Cinematic
         && getEngine().getCameraController().getMode() != CameraMode::FreeLook)
      {
        getEngine().getCameraController().setMode(CameraMode::Combat);
      }
      updateGuns(gunType);
      break;
    case WeaponId::Shotgun:
      if(shotgunAmmo.ammo != 0)
      {
        if(getEngine().getInputHandler().getInputState().action)
        {
          const auto& uziLara = *getEngine().findAnimatedModelForType(TR1ItemId::LaraUzisAnim);
          BOOST_ASSERT(uziLara.bones.size() == getSkeleton()->getBoneCount());
          getSkeleton()->setMeshPart(14, uziLara.bones[14].mesh);
          getSkeleton()->rebuildMesh();
        }
      }
      if(getEngine().getCameraController().getMode() != CameraMode::Cinematic
         && getEngine().getCameraController().getMode() != CameraMode::FreeLook)
      {
        getEngine().getCameraController().setMode(CameraMode::Combat);
      }
      updateShotgun();
      break;
    default: return;
    }
  }
}

void LaraObject::updateShotgun()
{
  if(getEngine().getInputHandler().getInputState().action)
  {
    updateAimingState(weapons[WeaponId::Shotgun]);
  }
  else
  {
    target = nullptr;
  }
  if(target == nullptr)
  {
    findTarget(weapons[WeaponId::Shotgun]);
  }
  updateAimAngles(weapons[WeaponId::Shotgun], leftArm);
  if(leftArm.aiming)
  {
    m_torsoRotation.X = leftArm.aimRotation.X / 2;
    m_torsoRotation.Y = leftArm.aimRotation.Y / 2;
    m_headRotation.X = 0_deg;
    m_headRotation.Y = 0_deg;
  }
  updateAnimShotgun();
}

void LaraObject::updateGuns(const WeaponId weaponId)
{
  const auto& weapon = weapons.at(weaponId);
  if(getEngine().getInputHandler().getInputState().action)
  {
    updateAimingState(weapon);
  }
  else
  {
    target = nullptr;
  }
  if(target == nullptr)
  {
    findTarget(weapon);
  }
  updateAimAngles(weapon, leftArm);
  updateAimAngles(weapon, rightArm);
  if(leftArm.aiming && !rightArm.aiming)
  {
    m_headRotation.Y = m_torsoRotation.Y = leftArm.aimRotation.Y / 2;
    m_headRotation.X = m_torsoRotation.X = leftArm.aimRotation.X / 2;
  }
  else if(rightArm.aiming && !leftArm.aiming)
  {
    m_headRotation.Y = m_torsoRotation.Y = rightArm.aimRotation.Y / 2;
    m_headRotation.X = m_torsoRotation.X = rightArm.aimRotation.X / 2;
  }
  else if(leftArm.aiming && rightArm.aiming)
  {
    m_headRotation.Y = m_torsoRotation.Y = (leftArm.aimRotation.Y + rightArm.aimRotation.Y) / 4;
    m_headRotation.X = m_torsoRotation.X = (leftArm.aimRotation.X + rightArm.aimRotation.X) / 4;
  }

  updateAnimNotShotgun(weaponId);
}

void LaraObject::updateAimingState(const Weapon& weapon)
{
  if(target == nullptr)
  {
    rightArm.aiming = false;
    leftArm.aiming = false;
    m_weaponTargetVector.X = 0_deg;
    m_weaponTargetVector.Y = 0_deg;
    return;
  }

  core::RoomBoundPosition gunPosition{m_state.position};
  gunPosition.position.Y -= weapon.gunHeight;
  auto enemyChestPos = getUpperThirdBBoxCtr(*target);
  auto targetVector = getVectorAngles(enemyChestPos.position - gunPosition.position);
  targetVector.X -= m_state.rotation.X;
  targetVector.Y -= m_state.rotation.Y;
  if(!CameraController::clampPosition(gunPosition, enemyChestPos, getEngine()))
  {
    rightArm.aiming = false;
    leftArm.aiming = false;
  }
  else if(targetVector.Y < weapon.lockAngles.y.min || targetVector.Y > weapon.lockAngles.y.max
          || targetVector.X < weapon.lockAngles.x.min || targetVector.X > weapon.lockAngles.x.max)
  {
    if(leftArm.aiming)
    {
      if(targetVector.Y < weapon.leftAngles.y.min || targetVector.Y > weapon.leftAngles.y.max
         || targetVector.X < weapon.leftAngles.x.min || targetVector.X > weapon.leftAngles.x.max)
      {
        leftArm.aiming = false;
      }
    }
    if(rightArm.aiming)
    {
      if(targetVector.Y < weapon.rightAngles.y.min || targetVector.Y > weapon.rightAngles.y.max
         || targetVector.X < weapon.rightAngles.x.min || targetVector.X > weapon.rightAngles.x.max)
      {
        rightArm.aiming = false;
      }
    }
  }
  else
  {
    rightArm.aiming = true;
    leftArm.aiming = true;
  }
  m_weaponTargetVector = targetVector;
}

void LaraObject::unholster()
{
  rightArm.frame = 0_frame;
  leftArm.frame = 0_frame;
  leftArm.aimRotation.Y = 0_deg;
  leftArm.aimRotation.X = 0_deg;
  rightArm.aimRotation.Y = 0_deg;
  rightArm.aimRotation.X = 0_deg;
  rightArm.aiming = false;
  leftArm.aiming = false;
  rightArm.flashTimeout = 0_frame;
  leftArm.flashTimeout = 0_frame;
  target = nullptr;
  if(gunType == WeaponId::None)
  {
    const auto* positionData = getEngine().findAnimatedModelForType(TR1ItemId::Lara)->frames;

    rightArm.weaponAnimData = positionData;
    leftArm.weaponAnimData = positionData;
  }
  else if(gunType == WeaponId::Pistols || gunType == WeaponId::AutoPistols || gunType == WeaponId::Uzi)
  {
    const auto* positionData = getEngine().findAnimatedModelForType(TR1ItemId::LaraPistolsAnim)->frames;

    rightArm.weaponAnimData = positionData;
    leftArm.weaponAnimData = positionData;

    if(m_handStatus != HandStatus::None)
    {
      overrideLaraMeshesUnholsterGuns(gunType);
    }
  }
  else if(gunType == WeaponId::Shotgun)
  {
    const auto* positionData = getEngine().findAnimatedModelForType(TR1ItemId::LaraShotgunAnim)->frames;

    rightArm.weaponAnimData = positionData;
    leftArm.weaponAnimData = positionData;

    if(m_handStatus != HandStatus::None)
    {
      overrideLaraMeshesUnholsterShotgun();
    }
  }
  else
  {
    const auto* positionData = getEngine().findAnimatedModelForType(TR1ItemId::Lara)->frames;

    rightArm.weaponAnimData = positionData;
    leftArm.weaponAnimData = positionData;
  }
}

core::RoomBoundPosition LaraObject::getUpperThirdBBoxCtr(const ModelObject& object)
{
  const auto kf = object.getSkeleton()->getInterpolationInfo().getNearestFrame();
  const auto bbox = kf->bbox.toBBox();

  const auto ctrX = (bbox.minX + bbox.maxX) / 2;
  const auto ctrZ = (bbox.minZ + bbox.maxZ) / 2;
  const auto ctrY3 = (bbox.maxY - bbox.minY) / 3 + bbox.minY;

  core::RoomBoundPosition result{object.m_state.position};
  result.position += util::pitch(core::TRVec{ctrX, ctrY3, ctrZ}, object.m_state.rotation.Y);
  return result;
}

void LaraObject::unholsterGuns(const WeaponId weaponId)
{
  auto nextFrame = leftArm.frame + 1_frame;
  if(nextFrame < 5_frame || nextFrame > 23_frame)
  {
    nextFrame = 5_frame;
  }
  else if(nextFrame == 13_frame)
  {
    overrideLaraMeshesUnholsterGuns(weaponId);
    playSoundEffect(TR1SoundId::LaraUnholster);
  }
  else if(nextFrame == 23_frame)
  {
    initAimInfoPistol();
    nextFrame = 0_frame;
  }

  leftArm.frame = nextFrame;
  rightArm.frame = nextFrame;
}

void LaraObject::findTarget(const Weapon& weapon)
{
  core::RoomBoundPosition gunPosition{m_state.position};
  gunPosition.position.Y -= weapons[WeaponId::Shotgun].gunHeight;
  std::shared_ptr<ModelObject> bestEnemy = nullptr;
  core::Angle bestYAngle{std::numeric_limits<core::Angle::type>::max()};
  for(const auto& currentEnemy : getEngine().getObjects() | boost::adaptors::map_values)
  {
    if(currentEnemy->m_state.health <= 0_hp || currentEnemy.get().get() == &getEngine().getLara())
      continue;

    const auto modelEnemy = std::dynamic_pointer_cast<ModelObject>(currentEnemy.get());
    if(modelEnemy == nullptr)
    {
      BOOST_LOG_TRIVIAL(warning) << "Ignoring non-model object " << currentEnemy->getNode()->getId();
      continue;
    }

    if(!modelEnemy->getNode()->isVisible())
      continue;

    const auto d = currentEnemy->m_state.position.position - gunPosition.position;
    if(abs(d.X) > weapon.targetDist)
      continue;

    if(abs(d.Y) > weapon.targetDist)
      continue;

    if(abs(d.Z) > weapon.targetDist)
      continue;

    if(util::square(d.X) + util::square(d.Y) + util::square(d.Z) >= util::square(weapon.targetDist))
      continue;

    auto enemyPos = getUpperThirdBBoxCtr(*std::dynamic_pointer_cast<const ModelObject>(currentEnemy.get()));
    if(!CameraController::clampPosition(gunPosition, enemyPos, getEngine()))
      continue;

    auto aimAngle = getVectorAngles(enemyPos.position - gunPosition.position);
    aimAngle.X -= m_torsoRotation.X + m_state.rotation.X;
    aimAngle.Y -= m_torsoRotation.Y + m_state.rotation.Y;
    if(aimAngle.Y < weapon.lockAngles.y.min || aimAngle.Y > weapon.lockAngles.y.max
       || aimAngle.X < weapon.lockAngles.x.min || aimAngle.X > weapon.lockAngles.x.max)
      continue;

    const auto absY = abs(aimAngle.Y);
    if(absY >= bestYAngle)
      continue;

    bestYAngle = absY;
    bestEnemy = modelEnemy;
  }
  target = bestEnemy;
  updateAimingState(weapon);
}

void LaraObject::initAimInfoPistol()
{
  m_handStatus = HandStatus::Combat;
  leftArm.aimRotation.Y = 0_deg;
  leftArm.aimRotation.X = 0_deg;
  rightArm.aimRotation.Y = 0_deg;
  rightArm.aimRotation.X = 0_deg;
  rightArm.frame = 0_frame;
  leftArm.frame = 0_frame;
  rightArm.aiming = false;
  leftArm.aiming = false;
  m_torsoRotation.Y = 0_deg;
  m_torsoRotation.X = 0_deg;
  m_headRotation.Y = 0_deg;
  m_headRotation.X = 0_deg;
  target = nullptr;

  rightArm.weaponAnimData = getEngine().findAnimatedModelForType(TR1ItemId::LaraPistolsAnim)->frames;
  leftArm.weaponAnimData = rightArm.weaponAnimData;
}

void LaraObject::initAimInfoShotgun()
{
  m_handStatus = HandStatus::Combat;
  leftArm.aimRotation.Y = 0_deg;
  leftArm.aimRotation.X = 0_deg;
  rightArm.aimRotation.Y = 0_deg;
  rightArm.aimRotation.X = 0_deg;
  rightArm.frame = 0_frame;
  leftArm.frame = 0_frame;
  rightArm.aiming = false;
  leftArm.aiming = false;
  m_torsoRotation.Y = 0_deg;
  m_torsoRotation.X = 0_deg;
  m_headRotation.Y = 0_deg;
  m_headRotation.X = 0_deg;
  target = nullptr;

  rightArm.weaponAnimData = getEngine().findAnimatedModelForType(TR1ItemId::LaraShotgunAnim)->frames;
  leftArm.weaponAnimData = rightArm.weaponAnimData;
}

void LaraObject::overrideLaraMeshesUnholsterGuns(const WeaponId weaponId)
{
  TR1ItemId id;
  if(weaponId == WeaponId::AutoPistols)
  {
    id = TR1ItemId::LaraMagnumsAnim;
  }
  else if(weaponId == WeaponId::Uzi)
  {
    id = TR1ItemId::LaraUzisAnim;
  }
  else
  {
    id = TR1ItemId::LaraPistolsAnim;
  }

  const auto& src = getEngine().findAnimatedModelForType(id);
  Expects(src != nullptr);
  BOOST_ASSERT(src->bones.size() == getSkeleton()->getBoneCount());
  const auto& normalLara = *getEngine().findAnimatedModelForType(TR1ItemId::Lara);
  BOOST_ASSERT(normalLara.bones.size() == getSkeleton()->getBoneCount());
  getSkeleton()->setMeshPart(1, normalLara.bones[1].mesh);
  getSkeleton()->setMeshPart(4, normalLara.bones[4].mesh);
  getSkeleton()->setMeshPart(10, src->bones[10].mesh);
  getSkeleton()->setMeshPart(13, src->bones[13].mesh);
  getSkeleton()->rebuildMesh();
}

void LaraObject::overrideLaraMeshesUnholsterShotgun()
{
  const auto& src = *getEngine().findAnimatedModelForType(TR1ItemId::LaraShotgunAnim);
  BOOST_ASSERT(src.bones.size() == getSkeleton()->getBoneCount());
  const auto& normalLara = *getEngine().findAnimatedModelForType(TR1ItemId::Lara);
  BOOST_ASSERT(normalLara.bones.size() == getSkeleton()->getBoneCount());
  getSkeleton()->setMeshPart(7, normalLara.bones[7].mesh);
  getSkeleton()->setMeshPart(10, src.bones[10].mesh);
  getSkeleton()->setMeshPart(13, src.bones[13].mesh);
  getSkeleton()->rebuildMesh();
}

void LaraObject::unholsterShotgun()
{
  auto nextFrame = leftArm.frame + 1_frame;
  if(nextFrame < 5_frame || nextFrame > 47_frame)
  {
    nextFrame = 13_frame;
  }
  else if(nextFrame == 23_frame)
  {
    overrideLaraMeshesUnholsterShotgun();

    playSoundEffect(TR1SoundId::LaraUnholster);
  }
  else if(nextFrame == 47_frame)
  {
    initAimInfoShotgun();
    nextFrame = 0_frame;
  }

  leftArm.frame = nextFrame;
  rightArm.frame = nextFrame;
}

void LaraObject::updateAimAngles(const Weapon& weapon, AimInfo& aimInfo) const
{
  core::TRRotationXY targetRot{0_deg, 0_deg};
  if(aimInfo.aiming)
  {
    targetRot = m_weaponTargetVector;
  }

  if(aimInfo.aimRotation.X >= targetRot.X - weapon.aimSpeed && aimInfo.aimRotation.X <= targetRot.X + weapon.aimSpeed)
  {
    aimInfo.aimRotation.X = targetRot.X;
  }
  else if(aimInfo.aimRotation.X >= targetRot.X)
  {
    aimInfo.aimRotation.X -= weapon.aimSpeed;
  }
  else
  {
    aimInfo.aimRotation.X += weapon.aimSpeed;
  }

  if(aimInfo.aimRotation.Y >= targetRot.Y - weapon.aimSpeed && aimInfo.aimRotation.Y <= weapon.aimSpeed + targetRot.Y)
  {
    aimInfo.aimRotation.Y = targetRot.Y;
  }
  else if(aimInfo.aimRotation.Y >= targetRot.Y)
  {
    aimInfo.aimRotation.Y -= weapon.aimSpeed;
  }
  else
  {
    aimInfo.aimRotation.Y += weapon.aimSpeed;
  }
}

void LaraObject::updateAnimShotgun()
{
  auto aimingFrame = leftArm.frame;
  if(leftArm.aiming)
  {
    if(leftArm.frame < 0_frame || leftArm.frame >= 13_frame)
    {
      const auto nextFrame = leftArm.frame + 1_frame;
      if(leftArm.frame == 47_frame)
      {
        if(getEngine().getInputHandler().getInputState().action)
        {
          tryShootShotgun();
          rightArm.frame = nextFrame;
          leftArm.frame = nextFrame;
          return;
        }
      }
      else if(leftArm.frame <= 47_frame || leftArm.frame >= 80_frame)
      {
        if(leftArm.frame >= 114_frame && leftArm.frame < 127_frame)
        {
          aimingFrame = leftArm.frame + 1_frame;
          if(leftArm.frame == 126_frame)
          {
            rightArm.frame = 0_frame;
            leftArm.frame = 0_frame;
            return;
          }
        }
      }
      else
      {
        aimingFrame = leftArm.frame + 1_frame;
        if(nextFrame == 80_frame)
        {
          rightArm.frame = 47_frame;
          leftArm.frame = 47_frame;
          return;
        }
        if(nextFrame == 57_frame)
        {
          playSoundEffect(TR1SoundId::LaraPistolsCocking);
          rightArm.frame = aimingFrame;
          leftArm.frame = aimingFrame;
          return;
        }
      }
    }
    else
    {
      aimingFrame = leftArm.frame + 1_frame;
      if(leftArm.frame == 12_frame)
      {
        aimingFrame = 47_frame;
      }
    }

    rightArm.frame = aimingFrame;
    leftArm.frame = aimingFrame;
    return;
  }

  if(leftArm.frame == 0_frame && getEngine().getInputHandler().getInputState().action)
  {
    leftArm.frame += 1_frame;
    rightArm.frame += 1_frame;
    return;
  }

  if(leftArm.frame <= 0_frame || leftArm.frame >= 13_frame)
  {
    const auto nextFrame = leftArm.frame + 1_frame;
    if(leftArm.frame == 47_frame)
    {
      if(getEngine().getInputHandler().getInputState().action)
      {
        tryShootShotgun();
        rightArm.frame = aimingFrame + 1_frame;
        leftArm.frame = aimingFrame + 1_frame;
        return;
      }

      rightArm.frame = 114_frame;
      leftArm.frame = 114_frame;
      return;
    }
    if(leftArm.frame <= 47_frame || leftArm.frame >= 80_frame)
    {
      if(leftArm.frame >= 114_frame && leftArm.frame < 127_frame)
      {
        aimingFrame = leftArm.frame + 1_frame;
        if(leftArm.frame == 126_frame)
        {
          aimingFrame = 0_frame;
        }
      }
    }
    else
    {
      aimingFrame = leftArm.frame + 1_frame;
      if(nextFrame == 60_frame)
      {
        rightArm.frame = 0_frame;
        leftArm.frame = 0_frame;
        return;
      }
      if(nextFrame == 80_frame)
      {
        rightArm.frame = 114_frame;
        leftArm.frame = 114_frame;
        return;
      }
      if(nextFrame == 57_frame)
      {
        playSoundEffect(TR1SoundId::LaraPistolsCocking);
        rightArm.frame = aimingFrame;
        leftArm.frame = aimingFrame;
        return;
      }
    }
  }
  else
  {
    aimingFrame = leftArm.frame + 1_frame;
    if(leftArm.frame == 12_frame)
    {
      if(getEngine().getInputHandler().getInputState().action)
      {
        rightArm.frame = 47_frame;
        leftArm.frame = 47_frame;
        return;
      }

      rightArm.frame = 114_frame;
      leftArm.frame = 114_frame;
      return;
    }
  }

  rightArm.frame = aimingFrame;
  leftArm.frame = aimingFrame;
}

void LaraObject::tryShootShotgun()
{
  bool fireShotgun = false;
  for(int i = 0; i < 5; ++i)
  {
    core::TRRotationXY aimAngle;
    aimAngle.Y = util::rand15s(+20_deg) + m_state.rotation.Y + leftArm.aimRotation.Y;
    aimAngle.X = util::rand15s(+20_deg) + leftArm.aimRotation.X;
    if(fireWeapon(WeaponId::Shotgun, target, *this, aimAngle))
    {
      fireShotgun = true;
    }
  }
  if(fireShotgun)
  {
    playSoundEffect(weapons[WeaponId::Shotgun].shotSound);
  }
}

void LaraObject::holsterShotgun()
{
  auto aimFrame = leftArm.frame;
  if(leftArm.frame == 0_frame)
  {
    aimFrame = 80_frame;
  }
  else if(leftArm.frame >= 0_frame && leftArm.frame < 13_frame)
  {
    aimFrame = leftArm.frame + 1_frame;
    if(leftArm.frame == 12_frame)
    {
      aimFrame = 114_frame;
    }
  }
  else if(leftArm.frame == 47_frame)
  {
    aimFrame = 114_frame;
  }
  else if(leftArm.frame >= 47_frame && leftArm.frame < 80_frame)
  {
    aimFrame = leftArm.frame + 1_frame;
    if(leftArm.frame == 59_frame)
    {
      aimFrame = 0_frame;
    }
    else if(aimFrame == 80_frame)
    {
      aimFrame = 114_frame;
    }
  }
  else if(leftArm.frame >= 114_frame && leftArm.frame < 127_frame)
  {
    aimFrame = leftArm.frame + 1_frame;
    if(leftArm.frame == 126_frame)
    {
      aimFrame = 80_frame;
    }
  }
  else if(leftArm.frame >= 80_frame && leftArm.frame < 114_frame)
  {
    aimFrame = leftArm.frame + 1_frame;
    if(leftArm.frame == 100_frame)
    {
      const auto& src = *getEngine().findAnimatedModelForType(TR1ItemId::Lara);
      BOOST_ASSERT(src.bones.size() == getSkeleton()->getBoneCount());
      const auto& normalLara = *getEngine().findAnimatedModelForType(TR1ItemId::Lara);
      BOOST_ASSERT(normalLara.bones.size() == getSkeleton()->getBoneCount());
      getSkeleton()->setMeshPart(7, src.bones[7].mesh);
      getSkeleton()->setMeshPart(10, normalLara.bones[10].mesh);
      getSkeleton()->setMeshPart(13, normalLara.bones[13].mesh);
      getSkeleton()->rebuildMesh();

      playSoundEffect(TR1SoundId::LaraUnholster);
    }
    else if(leftArm.frame == 113_frame)
    {
      aimFrame = 0_frame;
      m_handStatus = HandStatus::None;
      target = nullptr;
      rightArm.aiming = false;
      leftArm.aiming = false;
    }
  }

  rightArm.frame = aimFrame;
  leftArm.frame = aimFrame;

  m_torsoRotation.X /= 2;
  m_torsoRotation.Y /= 2;
  m_headRotation.X = 0_deg;
  m_headRotation.Y = 0_deg;
}

void LaraObject::holsterGuns(const WeaponId weaponId)
{
  if(leftArm.frame >= 24_frame)
  {
    leftArm.frame = 4_frame;
  }
  else if(leftArm.frame > 0_frame && leftArm.frame < 5_frame)
  {
    leftArm.aimRotation.X -= leftArm.aimRotation.X / leftArm.frame.get_as<core::Angle::type>();
    leftArm.aimRotation.Y -= leftArm.aimRotation.Y / leftArm.frame.get_as<core::Angle::type>();
    leftArm.frame -= 1_frame;
  }
  else if(leftArm.frame == 0_frame)
  {
    leftArm.aimRotation.X = 0_deg;
    leftArm.aimRotation.Y = 0_deg;
    leftArm.frame = 23_frame;
  }
  else if(leftArm.frame > 5_frame && leftArm.frame < 24_frame)
  {
    leftArm.frame -= 1_frame;
    if(leftArm.frame == 12_frame)
    {
      TR1ItemId srcId = TR1ItemId::LaraPistolsAnim;
      if(weaponId == WeaponId::AutoPistols)
      {
        srcId = TR1ItemId::LaraMagnumsAnim;
      }
      else if(weaponId == WeaponId::Uzi)
      {
        srcId = TR1ItemId::LaraUzisAnim;
      }

      const auto& src = *getEngine().findAnimatedModelForType(srcId);
      BOOST_ASSERT(src.bones.size() == getSkeleton()->getBoneCount());
      const auto& normalLara = *getEngine().findAnimatedModelForType(TR1ItemId::Lara);
      BOOST_ASSERT(normalLara.bones.size() == getSkeleton()->getBoneCount());
      getSkeleton()->setMeshPart(1, src.bones[1].mesh);
      getSkeleton()->setMeshPart(13, normalLara.bones[13].mesh);
      getSkeleton()->rebuildMesh();

      playSoundEffect(TR1SoundId::LaraHolster);
    }
  }

  if(rightArm.frame >= 24_frame)
  {
    rightArm.frame = 4_frame;
  }
  else if(rightArm.frame > 0_frame && rightArm.frame < 5_frame)
  {
    rightArm.aimRotation.X -= rightArm.aimRotation.X / rightArm.frame.get_as<core::Angle::type>();
    rightArm.aimRotation.Y -= rightArm.aimRotation.Y / rightArm.frame.get_as<core::Angle::type>();
    rightArm.frame -= 1_frame;
  }
  else if(rightArm.frame == 0_frame)
  {
    rightArm.aimRotation.Y = 0_deg;
    rightArm.aimRotation.X = 0_deg;
    rightArm.frame = 23_frame;
  }
  else if(rightArm.frame > 5_frame && rightArm.frame < 24_frame)
  {
    rightArm.frame -= 1_frame;
    if(rightArm.frame == 12_frame)
    {
      TR1ItemId srcId = TR1ItemId::LaraPistolsAnim;
      if(weaponId == WeaponId::AutoPistols)
      {
        srcId = TR1ItemId::LaraMagnumsAnim;
      }
      else if(weaponId == WeaponId::Uzi)
      {
        srcId = TR1ItemId::LaraUzisAnim;
      }

      const auto& src = *getEngine().findAnimatedModelForType(srcId);
      BOOST_ASSERT(src.bones.size() == getSkeleton()->getBoneCount());
      const auto& normalLara = *getEngine().findAnimatedModelForType(TR1ItemId::Lara);
      BOOST_ASSERT(normalLara.bones.size() == getSkeleton()->getBoneCount());
      getSkeleton()->setMeshPart(4, src.bones[4].mesh);
      getSkeleton()->setMeshPart(10, normalLara.bones[10].mesh);
      getSkeleton()->rebuildMesh();

      playSoundEffect(TR1SoundId::LaraHolster);
    }
  }

  if(leftArm.frame == 5_frame && rightArm.frame == 5_frame)
  {
    m_handStatus = HandStatus::None;
    leftArm.frame = 0_frame;
    rightArm.frame = 0_frame;
    target = nullptr;
    rightArm.aiming = false;
    leftArm.aiming = false;
  }

  m_headRotation.X = (rightArm.aimRotation.X + leftArm.aimRotation.X) / 4;
  m_headRotation.Y = rightArm.aimRotation.Y / 4;
  m_torsoRotation.X = (rightArm.aimRotation.X + leftArm.aimRotation.X) / 4;
  m_torsoRotation.Y = rightArm.aimRotation.Y / 4;
}

void LaraObject::updateAnimNotShotgun(const WeaponId weaponId)
{
  const auto& weapon = weapons[weaponId];

  if(!rightArm.aiming && (!getEngine().getInputHandler().getInputState().action || target != nullptr))
  {
    if(rightArm.frame >= 24_frame)
    {
      rightArm.frame = 4_frame;
    }
    else if(rightArm.frame > 0_frame && rightArm.frame <= 4_frame)
    {
      rightArm.frame -= 1_frame;
    }
  }
  else if(rightArm.frame >= 0_frame && rightArm.frame < 4_frame)
  {
    rightArm.frame += 1_frame;
  }
  else if(getEngine().getInputHandler().getInputState().action && rightArm.frame == 4_frame)
  {
    core::TRRotationXY aimAngle;
    aimAngle.X = rightArm.aimRotation.X;
    aimAngle.Y = m_state.rotation.Y + rightArm.aimRotation.Y;
    if(fireWeapon(weaponId, target, *this, aimAngle))
    {
      rightArm.flashTimeout = weapon.flashTime;
      playSoundEffect(weapon.shotSound);
    }
    rightArm.frame = 24_frame;
  }
  else if(rightArm.frame >= 24_frame)
  {
    rightArm.frame += 1_frame;
    if(rightArm.frame == weapon.recoilFrame + 24_frame)
    {
      rightArm.frame = 4_frame;
    }
  }

  if(!leftArm.aiming && (!getEngine().getInputHandler().getInputState().action || target != nullptr))
  {
    if(leftArm.frame >= 24_frame)
    {
      leftArm.frame = 4_frame;
    }
    else if(leftArm.frame > 0_frame && leftArm.frame <= 4_frame)
    {
      leftArm.frame -= 1_frame;
    }
  }
  else if(leftArm.frame >= 0_frame && leftArm.frame < 4_frame)
  {
    leftArm.frame += 1_frame;
  }
  else if(getEngine().getInputHandler().getInputState().action && leftArm.frame == 4_frame)
  {
    core::TRRotationXY aimAngle;
    aimAngle.Y = m_state.rotation.Y + leftArm.aimRotation.Y;
    aimAngle.X = leftArm.aimRotation.X;
    if(fireWeapon(weaponId, target, *this, aimAngle))
    {
      leftArm.flashTimeout = weapon.flashTime;
      playSoundEffect(weapon.shotSound);
    }
    leftArm.frame = 24_frame;
  }
  else if(leftArm.frame >= 24_frame)
  {
    leftArm.frame += 1_frame;
    if(leftArm.frame == weapon.recoilFrame + 24_frame)
    {
      leftArm.frame = 4_frame;
    }
  }
}

bool LaraObject::fireWeapon(const WeaponId weaponId,
                            const std::shared_ptr<ModelObject>& targetObject,
                            const ModelObject& gunHolder,
                            const core::TRRotationXY& aimAngle)
{
  Expects(weaponId != WeaponId::None);

  Ammo* ammoPtr;
  if(weaponId == WeaponId::Pistols)
  {
    ammoPtr = &pistolsAmmo;
    pistolsAmmo.ammo = 1000;
  }
  else if(weaponId == WeaponId::AutoPistols)
  {
    ammoPtr = &revolverAmmo;
  }
  else if(weaponId == WeaponId::Uzi)
  {
    ammoPtr = &uziAmmo;
  }
  else if(weaponId == WeaponId::Shotgun)
  {
    ammoPtr = &shotgunAmmo;
  }
  else
  {
    BOOST_THROW_EXCEPTION(std::out_of_range("weaponId"));
  }

  if(true /* FIXME engine::allAmmoCheat */)
  {
    ammoPtr->ammo = 1000;
  }

  if(ammoPtr->ammo <= 0)
  {
    ammoPtr->ammo = 0;
    playSoundEffect(TR1SoundId::EmptyAmmo);
    requestedGunType = WeaponId::Pistols;
    return false;
  }

  --ammoPtr->ammo;
  const auto weapon = &weapons[weaponId];
  core::TRVec gunPosition = gunHolder.m_state.position.position;
  gunPosition.Y -= weapon->gunHeight;
  const core::TRRotation shootVector{
    util::rand15s(weapon->shotAccuracy / 2) + aimAngle.X, util::rand15s(weapon->shotAccuracy / 2) + aimAngle.Y, +0_deg};

  std::vector<SkeletalModelNode::Sphere> spheres;
  if(targetObject != nullptr)
  {
    spheres = targetObject->getSkeleton()->getBoneCollisionSpheres(
      targetObject->m_state, *targetObject->getSkeleton()->getInterpolationInfo().getNearestFrame(), nullptr);
  }
  bool hasHit = false;
  glm::vec3 hitPos;
  const auto bulletDir = normalize(glm::vec3(shootVector.toMatrix()[2])); // +Z is our shooting direction
  if(!spheres.empty())
  {
    core::Length minD{std::numeric_limits<core::Length::type>::max()};
    for(const auto& sphere : spheres)
    {
      hitPos = gunPosition.toRenderSystem()
               + bulletDir * dot(sphere.getPosition() - gunPosition.toRenderSystem(), bulletDir);

      const auto d = core::Length{static_cast<core::Length::type>(length(hitPos - sphere.getPosition()))};
      if(d > sphere.radius || d >= minD)
        continue;

      minD = d;
      hasHit = true;
    }
  }

  if(!hasHit)
  {
    ++ammoPtr->misses;

    static constexpr float VeryLargeDistanceProbablyClipping = 1 << 14;

    core::RoomBoundPosition aimHitPos{gunHolder.m_state.position.room,
                                      gunPosition + core::TRVec{-bulletDir * VeryLargeDistanceProbablyClipping}};

    const core::RoomBoundPosition bulletPos{gunHolder.m_state.position.room, gunPosition};
    CameraController::clampPosition(bulletPos, aimHitPos, getEngine());
    playShotMissed(aimHitPos);
  }
  else
  {
    BOOST_ASSERT(targetObject != nullptr);
    ++ammoPtr->hits;
    hitTarget(*targetObject, core::TRVec{hitPos}, weapon->damage);
  }

  return true;
}

void LaraObject::hitTarget(ModelObject& object, const core::TRVec& hitPos, const core::Health& damage)
{
  if(object.m_state.health > 0_hp && object.m_state.health <= damage)
  {
    // TODO ++g_numKills;
  }
  object.m_state.is_hit = true;
  object.m_state.health -= damage;
  auto fx = createBloodSplat(getEngine(),
                             core::RoomBoundPosition{object.m_state.position.room, hitPos},
                             object.m_state.speed,
                             object.m_state.rotation.Y);
  getEngine().getParticles().emplace_back(fx);
  if(object.m_state.health <= 0_hp)
    return;

  TR1SoundId soundId;
  switch(object.m_state.type.get_as<TR1ItemId>())
  {
  case TR1ItemId::Wolf: soundId = TR1SoundId::WolfHurt; break;
  case TR1ItemId::Bear: soundId = TR1SoundId::BearHurt; break;
  case TR1ItemId::LionMale:
  case TR1ItemId::LionFemale: soundId = TR1SoundId::LionHurt; break;
  case TR1ItemId::RatOnLand: soundId = TR1SoundId::RatHurt; break;
  case TR1ItemId::SkateboardKid: soundId = TR1SoundId::SkateboardKidHurt; break;
  case TR1ItemId::TorsoBoss: soundId = TR1SoundId::TorsoBossHurt; break;
  default: return;
  }

  playSoundEffect(soundId);
}

namespace
{
class MatrixStack
{
private:
  std::stack<glm::mat4> m_stack;

public:
  explicit MatrixStack()
  {
    m_stack.push(glm::mat4{1.0f});
  }

  void push()
  {
    m_stack.push(m_stack.top());
  }

  void pop()
  {
    m_stack.pop();
  }

  [[nodiscard]] const glm::mat4& top() const
  {
    return m_stack.top();
  }

  glm::mat4& top()
  {
    return m_stack.top();
  }

  void rotate(const glm::mat4& m)
  {
    m_stack.top() *= m;
  }

  void rotate(const core::TRRotation& r)
  {
    rotate(r.toMatrix());
  }

  void rotate(const core::TRRotationXY& r)
  {
    rotate(r.toMatrix());
  }

  void resetRotation()
  {
    top()[0] = glm::vec4{1, 0, 0, 0};
    top()[1] = glm::vec4{0, 1, 0, 0};
    top()[2] = glm::vec4{0, 0, 1, 0};
  }

  void rotate(const uint32_t packed)
  {
    m_stack.top() *= core::fromPackedAngles(packed);
  }

  void translate(const glm::vec3& c)
  {
    m_stack.top() = glm::translate(m_stack.top(), c);
  }

  void transform(const std::initializer_list<size_t>& indices,
                 const std::vector<loader::file::SkeletalModelType::Bone>& bones,
                 const gsl::span<const uint32_t>& angleData,
                 const std::shared_ptr<SkeletalModelNode>& skeleton)
  {
    for(auto idx : indices)
      transform(idx, bones, angleData, skeleton);
  }

  void transform(const size_t idx,
                 const std::vector<loader::file::SkeletalModelType::Bone>& bones,
                 const gsl::span<const uint32_t>& angleData,
                 const std::shared_ptr<SkeletalModelNode>& skeleton)
  {
    BOOST_ASSERT(idx > 0);
    translate(bones.at(idx).position);
    rotate(angleData[idx]);
    apply(skeleton, idx);
  }

  void apply(const std::shared_ptr<SkeletalModelNode>& skeleton, const size_t idx)
  {
    BOOST_ASSERT(skeleton != nullptr);
    skeleton->setMeshMatrix(idx, m_stack.top());
  }
};

class DualMatrixStack
{
private:
  MatrixStack m_stack1{};
  MatrixStack m_stack2{};
  const float m_bias;

public:
  explicit DualMatrixStack(const float bias)
      : m_bias{bias}
  {
  }

  void push()
  {
    m_stack1.push();
    m_stack2.push();
  }

  void pop()
  {
    m_stack1.pop();
    m_stack2.pop();
  }

  [[nodiscard]] glm::mat4 itop() const
  {
    return util::mix(m_stack1.top(), m_stack2.top(), m_bias);
  }

  void rotate(const glm::mat4& m)
  {
    m_stack1.top() *= m;
    m_stack2.top() *= m;
  }

  void rotate(const core::TRRotation& r)
  {
    rotate(r.toMatrix());
  }

  void rotate(const core::TRRotationXY& r)
  {
    rotate(r.toMatrix());
  }

  void rotate(const uint32_t packed1, const uint32_t packed2)
  {
    m_stack1.top() *= core::fromPackedAngles(packed1);
    m_stack2.top() *= core::fromPackedAngles(packed2);
  }

  void resetRotation()
  {
    m_stack1.resetRotation();
    m_stack2.resetRotation();
  }

  void translate(const glm::vec3& v1, const glm::vec3& v2)
  {
    m_stack1.top() = glm::translate(m_stack1.top(), v1);
    m_stack2.top() = glm::translate(m_stack2.top(), v2);
  }

  void translate(const glm::vec3& v)
  {
    translate(v, v);
  }

  void transform(const std::initializer_list<size_t>& indices,
                 const std::vector<loader::file::SkeletalModelType::Bone>& bones,
                 const gsl::span<const uint32_t>& angleData1,
                 const gsl::span<const uint32_t>& angleData2,
                 const std::shared_ptr<SkeletalModelNode>& skeleton)
  {
    for(auto idx : indices)
      transform(idx, bones, angleData1, angleData2, skeleton);
  }

  void transform(const size_t idx,
                 const std::vector<loader::file::SkeletalModelType::Bone>& bones,
                 const gsl::span<const uint32_t>& angleData1,
                 const gsl::span<const uint32_t>& angleData2,
                 const std::shared_ptr<SkeletalModelNode>& skeleton)
  {
    BOOST_ASSERT(idx > 0);
    translate(bones.at(idx).position);
    rotate(angleData1[idx], angleData2[idx]);
    apply(skeleton, idx);
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void apply(const std::shared_ptr<SkeletalModelNode>& skeleton, const size_t idx)
  {
    BOOST_ASSERT(skeleton != nullptr);
    skeleton->setMeshMatrix(idx, itop());
  }
};
} // namespace

void LaraObject::drawRoutine()
{
  const auto interpolationInfo = getSkeleton()->getInterpolationInfo();
  if(!hit_direction.has_value() && interpolationInfo.firstFrame != interpolationInfo.secondFrame)
  {
    drawRoutineInterpolated(interpolationInfo);
    return;
  }

  const auto& objInfo = *getEngine().findAnimatedModelForType(m_state.type);
  const loader::file::AnimFrame* frame;
  if(!hit_direction.has_value())
  {
    frame = interpolationInfo.firstFrame;
  }
  else
  {
    switch(*hit_direction)
    {
    case core::Axis::PosX: frame = getEngine().getAnimation(AnimationId::AH_LEFT).frames; break;
    case core::Axis::NegZ: frame = getEngine().getAnimation(AnimationId::AH_BACKWARD).frames; break;
    case core::Axis::NegX: frame = getEngine().getAnimation(AnimationId::AH_RIGHT).frames; break;
    default: frame = getEngine().getAnimation(AnimationId::AH_FORWARD).frames; break;
    }
    frame = frame->next(hit_frame.get());
  }

  updateLighting();

  MatrixStack matrixStack;

  matrixStack.push();
  matrixStack.translate(frame->pos.toGl());
  const auto angleData = frame->getAngleData();
  matrixStack.rotate(angleData[0]);
  matrixStack.apply(getSkeleton(), 0);

  matrixStack.push();
  matrixStack.transform({1, 2, 3}, objInfo.bones, angleData, getSkeleton());

  matrixStack.pop();
  matrixStack.push();
  matrixStack.transform({4, 5, 6}, objInfo.bones, angleData, getSkeleton());

  matrixStack.pop();
  matrixStack.translate(objInfo.bones[7].position);
  matrixStack.rotate(angleData[7]);
  matrixStack.rotate(m_torsoRotation);
  matrixStack.apply(getSkeleton(), 7);

  matrixStack.push();
  matrixStack.translate(objInfo.bones[14].position);
  matrixStack.rotate(angleData[14]);
  matrixStack.rotate(m_headRotation);
  matrixStack.apply(getSkeleton(), 14);

  WeaponId activeGunType = WeaponId::None;
  if(m_handStatus == HandStatus::Combat || m_handStatus == HandStatus::Unholster || m_handStatus == HandStatus::Holster)
  {
    activeGunType = gunType;
  }

  matrixStack.pop();
  gsl::span<const uint32_t> armAngleData;
  switch(activeGunType)
  {
  case WeaponId::None:
    matrixStack.push();
    matrixStack.transform({8, 9, 10}, objInfo.bones, angleData, getSkeleton());

    matrixStack.pop();
    matrixStack.push();
    matrixStack.transform({11, 12, 13}, objInfo.bones, angleData, getSkeleton());
    break;
  case WeaponId::Pistols:
  case WeaponId::AutoPistols:
  case WeaponId::Uzi:
    matrixStack.push();
    matrixStack.translate(objInfo.bones[8].position);
    matrixStack.resetRotation();
    matrixStack.rotate(rightArm.aimRotation);

    armAngleData = rightArm.weaponAnimData->next(rightArm.frame.get())->getAngleData();
    matrixStack.rotate(armAngleData[8]);
    matrixStack.apply(getSkeleton(), 8);

    matrixStack.transform(9, objInfo.bones, armAngleData, getSkeleton());
    matrixStack.transform(10, objInfo.bones, armAngleData, getSkeleton());

    renderGunFlare(activeGunType, matrixStack.top(), m_gunFlareRight, rightArm.flashTimeout != 0_frame);
    matrixStack.pop();
    matrixStack.push();
    matrixStack.translate(objInfo.bones[11].position);
    matrixStack.resetRotation();
    matrixStack.rotate(leftArm.aimRotation);
    armAngleData = leftArm.weaponAnimData->next(leftArm.frame.get())->getAngleData();
    matrixStack.rotate(armAngleData[11]);
    matrixStack.apply(getSkeleton(), 11);

    matrixStack.transform({12, 13}, objInfo.bones, armAngleData, getSkeleton());

    renderGunFlare(activeGunType, matrixStack.top(), m_gunFlareLeft, leftArm.flashTimeout != 0_frame);
    break;
  case WeaponId::Shotgun:
    matrixStack.push();
    armAngleData = rightArm.weaponAnimData->next(rightArm.frame.get())->getAngleData();
    matrixStack.transform({8, 9, 10}, objInfo.bones, armAngleData, getSkeleton());

    matrixStack.pop();
    matrixStack.push();
    armAngleData = leftArm.weaponAnimData->next(leftArm.frame.get())->getAngleData();
    matrixStack.transform({11, 12, 13}, objInfo.bones, armAngleData, getSkeleton());
    break;
  default: break;
  }
}

void LaraObject::drawRoutineInterpolated(const SkeletalModelNode::InterpolationInfo& interpolationInfo)
{
  updateLighting();

  const auto& objInfo = *getEngine().findAnimatedModelForType(m_state.type);

  DualMatrixStack matrixStack{interpolationInfo.bias};

  matrixStack.push();
  matrixStack.translate(interpolationInfo.firstFrame->pos.toGl(), interpolationInfo.secondFrame->pos.toGl());
  const auto angleDataA = interpolationInfo.firstFrame->getAngleData();
  const auto angleDataB = interpolationInfo.secondFrame->getAngleData();
  matrixStack.rotate(angleDataA[0], angleDataB[0]);
  matrixStack.apply(getSkeleton(), 0);

  matrixStack.push();
  matrixStack.transform({1, 2, 3}, objInfo.bones, angleDataA, angleDataB, getSkeleton());

  matrixStack.pop();
  matrixStack.push();
  matrixStack.transform({4, 5, 6}, objInfo.bones, angleDataA, angleDataB, getSkeleton());

  matrixStack.pop();
  matrixStack.translate(objInfo.bones[7].position);
  matrixStack.rotate(angleDataA[7], angleDataB[7]);
  matrixStack.rotate(m_torsoRotation);
  matrixStack.apply(getSkeleton(), 7);

  matrixStack.push();
  matrixStack.translate(objInfo.bones[14].position);
  matrixStack.rotate(angleDataA[14], angleDataB[14]);
  matrixStack.rotate(m_headRotation);
  matrixStack.apply(getSkeleton(), 14);

  WeaponId activeGunType = WeaponId::None;
  if(m_handStatus == HandStatus::Combat || m_handStatus == HandStatus::Unholster || m_handStatus == HandStatus::Holster)
  {
    activeGunType = gunType;
  }

  matrixStack.pop();
  gsl::span<const uint32_t> armAngleData;
  switch(activeGunType)
  {
  case WeaponId::None:
    matrixStack.push();
    matrixStack.transform({8, 9, 10}, objInfo.bones, angleDataA, angleDataB, getSkeleton());

    matrixStack.pop();
    matrixStack.push();
    matrixStack.transform({11, 12, 13}, objInfo.bones, angleDataA, angleDataB, getSkeleton());
    break;
  case WeaponId::Pistols:
  case WeaponId::AutoPistols:
  case WeaponId::Uzi:
    matrixStack.push();
    matrixStack.translate(objInfo.bones[8].position);
    matrixStack.resetRotation();
    matrixStack.rotate(rightArm.aimRotation);

    armAngleData = rightArm.weaponAnimData->next(rightArm.frame.get())->getAngleData();
    matrixStack.rotate(armAngleData[8], armAngleData[8]);
    matrixStack.apply(getSkeleton(), 8);

    matrixStack.transform(9, objInfo.bones, armAngleData, armAngleData, getSkeleton());
    matrixStack.transform(10, objInfo.bones, armAngleData, armAngleData, getSkeleton());

    renderGunFlare(activeGunType, matrixStack.itop(), m_gunFlareRight, rightArm.flashTimeout != 0_frame);
    matrixStack.pop();
    matrixStack.push();
    matrixStack.translate(objInfo.bones[11].position);
    matrixStack.resetRotation();
    matrixStack.rotate(leftArm.aimRotation);
    armAngleData = leftArm.weaponAnimData->next(leftArm.frame.get())->getAngleData();
    matrixStack.rotate(armAngleData[11], armAngleData[11]);
    matrixStack.apply(getSkeleton(), 11);

    matrixStack.transform({12, 13}, objInfo.bones, armAngleData, armAngleData, getSkeleton());

    renderGunFlare(activeGunType, matrixStack.itop(), m_gunFlareLeft, leftArm.flashTimeout != 0_frame);
    break;
  case WeaponId::Shotgun:
    matrixStack.push();
    armAngleData = rightArm.weaponAnimData->next(rightArm.frame.get())->getAngleData();
    matrixStack.transform({8, 9, 10}, objInfo.bones, armAngleData, armAngleData, getSkeleton());

    matrixStack.pop();
    matrixStack.push();
    armAngleData = leftArm.weaponAnimData->next(leftArm.frame.get())->getAngleData();
    matrixStack.transform({11, 12, 13}, objInfo.bones, armAngleData, armAngleData, getSkeleton());
    break;
  default: break;
  }
}

void LaraObject::renderGunFlare(const WeaponId weaponId,
                                glm::mat4 m,
                                const gsl::not_null<std::shared_ptr<render::scene::Node>>& flareNode,
                                const bool visible) const
{
  if(!visible)
  {
    flareNode->setVisible(false);
    return;
  }

  core::Shade shade{core::Shade::type{0}};
  core::Length dy = 0_len;
  switch(weaponId)
  {
  case WeaponId::None:
  case WeaponId::Pistols:
    shade = core::Shade{core::Shade::type{5120}};
    dy = 155_len;
    break;
  case WeaponId::AutoPistols:
    shade = core::Shade{core::Shade::type{4096}};
    dy = 155_len;
    break;
  case WeaponId::Uzi:
    shade = core::Shade{core::Shade::type{2560}};
    dy = 180_len;
    break;
  case WeaponId::Shotgun:
    shade = core::Shade{core::Shade::type{5120}};
    dy = 155_len;
    break;
  default: BOOST_THROW_EXCEPTION(std::domain_error("WeaponId"));
  }

  m = translate(m, core::TRVec{0_len, dy, 55_len}.toRenderSystem());
  m *= core::TRRotation(-90_deg, 0_deg, util::rand15s(180_deg) * 2).toMatrix();

  flareNode->setVisible(true);
  setParent(flareNode, getNode()->getParent().lock());
  flareNode->setLocalMatrix(getNode()->getLocalMatrix() * m);

  flareNode->addUniformSetter(
    "u_lightAmbient", [brightness = toBrightness(shade)](const render::scene::Node& /*node*/, gl::Uniform& uniform) {
      uniform.set(brightness.get());
    });
}

void LaraObject::burnIfAlive()
{
  if(m_state.health <= 0_hp)
    return;

  const auto sector = findRealFloorSector(m_state.position.position, m_state.position.room);
  if(HeightInfo::fromFloor(
       sector, {m_state.position.position.X, 32000_len, m_state.position.position.Z}, getEngine().getObjects())
       .y
     != m_state.floor)
    return;

  m_state.health = -1_hp;
  m_state.is_hit = true;

  for(size_t i = 0; i < 10; ++i)
  {
    auto particle = std::make_shared<FlameParticle>(m_state.position, getEngine(), true);
    setParent(particle, m_state.position.room->node);
    getEngine().getParticles().emplace_back(particle);
  }
}

void LaraObject::serialize(const serialization::Serializer& ser)
{
  ModelObject::serialize(ser);
  ser(S_NV("yRotationSpeed", m_yRotationSpeed),
      S_NV("fallSpeedOverride", m_fallSpeedOverride),
      S_NV("movementAngle", m_movementAngle),
      S_NV("air", m_air),
      S_NV("currentSlideAngle", m_currentSlideAngle),
      S_NV("handStatus", m_handStatus),
      S_NV("underwaterState", m_underwaterState),
      S_NV("swimToDiveKeypressDuration", m_swimToDiveKeypressDuration),
      S_NV("headRotation", m_headRotation),
      S_NV("torsoRotation", m_torsoRotation),
      S_NV("underwaterCurrentStrength", m_underwaterCurrentStrength),
      S_NV("underwaterRoute", m_underwaterRoute),
      S_NV("hitDirection", hit_direction),
      S_NV("hitFrame", hit_frame),
      S_NV("explosionStumblingDuration", explosionStumblingDuration),
      // FIXME S_NVP(forceSourcePosition),
      S_NV("leftArm", leftArm),
      S_NV("rightArm", rightArm),
      S_NV("gunType", gunType),
      S_NV("requestedGunType", requestedGunType),
      S_NV("pistolsAmmo", pistolsAmmo),
      S_NV("revolverAmmo", revolverAmmo),
      S_NV("uziAmmo", uziAmmo),
      S_NV("shotgunAmmo", shotgunAmmo),
      S_NV("weaponTargetVector", m_weaponTargetVector),
      S_NV("weapons", weapons));

  ser.lazy(
    [this](const serialization::Serializer& ser) { ser(S_NV("target", serialization::ObjectReference{target})); });

  if(ser.loading)
    forceSourcePosition = nullptr;
}

LaraObject::LaraObject(const gsl::not_null<Engine*>& engine,
                       const gsl::not_null<const loader::file::Room*>& room,
                       const loader::file::Item& item,
                       const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
    : ModelObject(engine, room, item, false, animatedModel)
    , m_underwaterRoute{*engine}
{
  setAnimation(AnimationId::STAY_IDLE);
  setGoalAnimState(LaraStateId::Stop);
  setMovementAngle(m_state.rotation.Y);

  m_underwaterRoute.step = core::SectorSize * 20;
  m_underwaterRoute.drop = -core::SectorSize * 20;
  m_underwaterRoute.fly = core::QuarterSectorSize;

  Weapon w{};
  weapons[WeaponId::None] = w;

  w.lockAngles.y.min = -60_deg;
  w.lockAngles.y.max = +60_deg;
  w.lockAngles.x.min = -60_deg;
  w.lockAngles.x.max = +60_deg;
  w.leftAngles.y.min = -170_deg;
  w.leftAngles.y.max = +60_deg;
  w.leftAngles.x.min = -80_deg;
  w.leftAngles.x.max = +80_deg;
  w.rightAngles.y.min = -60_deg;
  w.rightAngles.y.max = +170_deg;
  w.rightAngles.x.min = -80_deg;
  w.rightAngles.x.max = +80_deg;
  w.aimSpeed = +10_deg;
  w.shotAccuracy = +8_deg;
  w.gunHeight = 650_len;
  w.damage = 1_hp;
  w.targetDist = core::SectorSize * 8;
  w.recoilFrame = 9_frame;
  w.flashTime = 3_frame;
  w.shotSound = TR1SoundId::LaraShootPistols;
  weapons[WeaponId::Pistols] = w;

  w.lockAngles.y.min = -60_deg;
  w.lockAngles.y.max = +60_deg;
  w.lockAngles.x.min = -60_deg;
  w.lockAngles.x.max = +60_deg;
  w.leftAngles.y.min = -170_deg;
  w.leftAngles.y.max = +60_deg;
  w.leftAngles.x.min = -80_deg;
  w.leftAngles.x.max = +80_deg;
  w.rightAngles.y.min = -60_deg;
  w.rightAngles.y.max = +170_deg;
  w.rightAngles.x.min = -80_deg;
  w.rightAngles.x.max = +80_deg;
  w.aimSpeed = +10_deg;
  w.shotAccuracy = +8_deg;
  w.gunHeight = 650_len;
  w.damage = 2_hp;
  w.targetDist = core::SectorSize * 8;
  w.recoilFrame = 9_frame;
  w.flashTime = 3_frame;
  w.shotSound = TR1SoundId::CowboyShoot;
  weapons[WeaponId::AutoPistols] = w;

  w.lockAngles.y.min = -60_deg;
  w.lockAngles.y.max = +60_deg;
  w.lockAngles.x.min = -60_deg;
  w.lockAngles.x.max = +60_deg;
  w.leftAngles.y.min = -170_deg;
  w.leftAngles.y.max = +60_deg;
  w.leftAngles.x.min = -80_deg;
  w.leftAngles.x.max = +80_deg;
  w.rightAngles.y.min = -60_deg;
  w.rightAngles.y.max = +170_deg;
  w.rightAngles.x.min = -80_deg;
  w.rightAngles.x.max = +80_deg;
  w.aimSpeed = +10_deg;
  w.shotAccuracy = +8_deg;
  w.gunHeight = 650_len;
  w.damage = 1_hp;
  w.targetDist = core::SectorSize * 8;
  w.recoilFrame = 3_frame;
  w.flashTime = 2_frame;
  w.shotSound = TR1SoundId::LaraShootUzis;
  weapons[WeaponId::Uzi] = w;

  w.lockAngles.y.min = -60_deg;
  w.lockAngles.y.max = +60_deg;
  w.lockAngles.x.min = -55_deg;
  w.lockAngles.x.max = +55_deg;
  w.leftAngles.y.min = -80_deg;
  w.leftAngles.y.max = +80_deg;
  w.leftAngles.x.min = -65_deg;
  w.leftAngles.x.max = +65_deg;
  w.rightAngles.y.min = -80_deg;
  w.rightAngles.y.max = +80_deg;
  w.rightAngles.x.min = -65_deg;
  w.rightAngles.x.max = +65_deg;
  w.aimSpeed = +10_deg;
  w.shotAccuracy = 0_deg;
  w.gunHeight = 500_len;
  w.damage = 4_hp;
  w.targetDist = core::SectorSize * 8;
  w.recoilFrame = 9_frame;
  w.flashTime = 3_frame;
  w.shotSound = TR1SoundId::LaraShootShotgun;
  weapons[WeaponId::Shotgun] = w;

  m_state.health = core::LaraHealth;
  m_state.collidable = true;
  m_state.is_hit = true;
  m_state.falling = true;

  initGunflares();
}

void LaraObject::initGunflares()
{
  const auto& gunFlareModel = getEngine().findAnimatedModelForType(TR1ItemId::Gunflare);
  if(gunFlareModel == nullptr)
    return;

  loader::file::RenderMeshDataCompositor compositor;
  compositor.append(*gunFlareModel->bones[0].mesh);
  auto mdl = compositor.toMesh(*getEngine().getMaterialManager(), false, {});

  m_gunFlareLeft->setRenderable(mdl);
  m_gunFlareLeft->setVisible(false);

  m_gunFlareRight->setRenderable(mdl);
  m_gunFlareRight->setVisible(false);
}

void LaraObject::Ammo::serialize(const serialization::Serializer& ser)
{
  ser(S_NV("ammo", ammo), S_NV("hits", hits), S_NV("misses", misses));
}

void LaraObject::AimInfo::serialize(const serialization::Serializer& ser)
{
  ser(S_NV("weaponAnimData", weaponAnimData),
      S_NV("frame", frame),
      S_NV("aiming", aiming),
      S_NV("aimRotation", aimRotation),
      S_NV("flashTimeout", flashTimeout));
}

void LaraObject::Weapon::serialize(const serialization::Serializer& ser)
{
  ser(S_NV("lockAngles", lockAngles),
      S_NV("leftAngles", leftAngles),
      S_NV("rightAngles", rightAngles),
      S_NV("aimSpeed", aimSpeed),
      S_NV("shotAccuracy", shotAccuracy),
      S_NV("gunHeight", gunHeight),
      S_NV("damage", damage),
      S_NV("targetDist", targetDist),
      S_NV("recoilFrame", recoilFrame),
      S_NV("flashTime", flashTime),
      S_NV("shotSound", shotSound));
}

void LaraObject::Range::serialize(const serialization::Serializer& ser)
{
  ser(S_NV("min", min), S_NV("max", max));
}

void LaraObject::RangeXY::serialize(const serialization::Serializer& ser)
{
  ser(S_NV("x", x), S_NV("y", y));
}
} // namespace engine::objects
