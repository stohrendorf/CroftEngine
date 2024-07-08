#include "abstractstatehandler.h"

#include "core/angle.h"
#include "core/boundingbox.h"
#include "core/genericvec.h"
#include "core/interval.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/audioengine.h"
#include "engine/collisioninfo.h"
#include "engine/heightinfo.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/objects/laraobject.h"
#include "engine/objects/objectstate.h"
#include "engine/presenter.h"
#include "engine/soundeffects_tr1.h"
#include "engine/world/animation.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "loader/file/larastateid.h"
#include "qs/qs.h"
#include "statehandler_0.h"
#include "statehandler_1.h"
#include "statehandler_10.h"
#include "statehandler_11.h"
#include "statehandler_12.h"
#include "statehandler_13.h"
#include "statehandler_14.h"
#include "statehandler_15.h"
#include "statehandler_16.h"
#include "statehandler_17.h"
#include "statehandler_18.h"
#include "statehandler_19.h"
#include "statehandler_2.h"
#include "statehandler_20.h"
#include "statehandler_21.h"
#include "statehandler_22.h"
#include "statehandler_23.h"
#include "statehandler_24.h"
#include "statehandler_25.h"
#include "statehandler_26.h"
#include "statehandler_27.h"
#include "statehandler_28.h"
#include "statehandler_29.h"
#include "statehandler_3.h"
#include "statehandler_30.h"
#include "statehandler_31.h"
#include "statehandler_32.h"
#include "statehandler_33.h"
#include "statehandler_34.h"
#include "statehandler_35.h"
#include "statehandler_36.h"
#include "statehandler_37.h"
#include "statehandler_38.h"
#include "statehandler_39.h"
#include "statehandler_4.h"
#include "statehandler_40.h"
#include "statehandler_41.h"
#include "statehandler_42.h"
#include "statehandler_43.h"
#include "statehandler_44.h"
#include "statehandler_45.h"
#include "statehandler_46.h"
#include "statehandler_47.h"
#include "statehandler_48.h"
#include "statehandler_49.h"
#include "statehandler_5.h"
#include "statehandler_50.h"
#include "statehandler_51.h"
#include "statehandler_52.h"
#include "statehandler_53.h"
#include "statehandler_54.h"
#include "statehandler_55.h"
#include "statehandler_6.h"
#include "statehandler_7.h"
#include "statehandler_8.h"
#include "statehandler_9.h"
#include "util/helpers.h"

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cstdlib>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <optional>
#include <stdexcept>

namespace engine::lara
{
std::unique_ptr<AbstractStateHandler> AbstractStateHandler::create(const LaraStateId id,
                                                                   const gsl::not_null<objects::LaraObject*>& lara)
{
  switch(id)
  {
  case LaraStateId::WalkForward:
    return std::make_unique<StateHandler_0>(lara);
  case LaraStateId::RunForward:
    return std::make_unique<StateHandler_1>(lara);
  case LaraStateId::Stop:
    return std::make_unique<StateHandler_2>(lara);
  case LaraStateId::JumpForward:
    return std::make_unique<StateHandler_3>(lara);
  case LaraStateId::Pose:
    return std::make_unique<StateHandler_4>(lara);
  case LaraStateId::RunBack:
    return std::make_unique<StateHandler_5>(lara);
  case LaraStateId::TurnRightSlow:
    return std::make_unique<StateHandler_6>(lara);
  case LaraStateId::TurnLeftSlow:
    return std::make_unique<StateHandler_7>(lara);
  case LaraStateId::Death:
    return std::make_unique<StateHandler_8>(lara);
  case LaraStateId::FreeFall:
    return std::make_unique<StateHandler_9>(lara);
  case LaraStateId::Hang:
    return std::make_unique<StateHandler_10>(lara);
  case LaraStateId::Reach:
    return std::make_unique<StateHandler_11>(lara);
  case LaraStateId::Unknown12:
    return std::make_unique<StateHandler_12>(lara);
  case LaraStateId::UnderwaterStop:
    return std::make_unique<StateHandler_13>(lara);
  case LaraStateId::GrabToFall:
    return std::make_unique<StateHandler_14>(lara);
  case LaraStateId::JumpPrepare:
    return std::make_unique<StateHandler_15>(lara);
  case LaraStateId::WalkBackward:
    return std::make_unique<StateHandler_16>(lara);
  case LaraStateId::UnderwaterForward:
    return std::make_unique<StateHandler_17>(lara);
  case LaraStateId::UnderwaterInertia:
    return std::make_unique<StateHandler_18>(lara);
  case LaraStateId::Climbing:
    return std::make_unique<StateHandler_19>(lara);
  case LaraStateId::TurnFast:
    return std::make_unique<StateHandler_20>(lara);
  case LaraStateId::StepRight:
    return std::make_unique<StateHandler_21>(lara);
  case LaraStateId::StepLeft:
    return std::make_unique<StateHandler_22>(lara);
  case LaraStateId::RollBackward:
    return std::make_unique<StateHandler_23>(lara);
  case LaraStateId::SlideForward:
    return std::make_unique<StateHandler_24>(lara);
  case LaraStateId::JumpBack:
    return std::make_unique<StateHandler_25>(lara);
  case LaraStateId::JumpLeft:
    return std::make_unique<StateHandler_26>(lara);
  case LaraStateId::JumpRight:
    return std::make_unique<StateHandler_27>(lara);
  case LaraStateId::JumpUp:
    return std::make_unique<StateHandler_28>(lara);
  case LaraStateId::FallBackward:
    return std::make_unique<StateHandler_29>(lara);
  case LaraStateId::ShimmyLeft:
    return std::make_unique<StateHandler_30>(lara);
  case LaraStateId::ShimmyRight:
    return std::make_unique<StateHandler_31>(lara);
  case LaraStateId::SlideBackward:
    return std::make_unique<StateHandler_32>(lara);
  case LaraStateId::OnWaterStop:
    return std::make_unique<StateHandler_33>(lara);
  case LaraStateId::OnWaterForward:
    return std::make_unique<StateHandler_34>(lara);
  case LaraStateId::UnderwaterDiving:
    return std::make_unique<StateHandler_35>(lara);
  case LaraStateId::PushablePush:
    return std::make_unique<StateHandler_36>(lara);
  case LaraStateId::PushablePull:
    return std::make_unique<StateHandler_37>(lara);
  case LaraStateId::PushableGrab:
    return std::make_unique<StateHandler_38>(lara);
  case LaraStateId::PickUp:
    return std::make_unique<StateHandler_39>(lara);
  case LaraStateId::SwitchDown:
    return std::make_unique<StateHandler_40>(lara);
  case LaraStateId::SwitchUp:
    return std::make_unique<StateHandler_41>(lara);
  case LaraStateId::InsertKey:
    return std::make_unique<StateHandler_42>(lara);
  case LaraStateId::InsertPuzzle:
    return std::make_unique<StateHandler_43>(lara);
  case LaraStateId::WaterDeath:
    return std::make_unique<StateHandler_44>(lara);
  case LaraStateId::RollForward:
    return std::make_unique<StateHandler_45>(lara);
  case LaraStateId::BoulderDeath:
    return std::make_unique<StateHandler_46>(lara);
  case LaraStateId::OnWaterBackward:
    return std::make_unique<StateHandler_47>(lara);
  case LaraStateId::OnWaterLeft:
    return std::make_unique<StateHandler_48>(lara);
  case LaraStateId::OnWaterRight:
    return std::make_unique<StateHandler_49>(lara);
  case LaraStateId::UseMidas:
    return std::make_unique<StateHandler_50>(lara);
  case LaraStateId::MidasDeath:
    return std::make_unique<StateHandler_51>(lara);
  case LaraStateId::SwandiveBegin:
    return std::make_unique<StateHandler_52>(lara);
  case LaraStateId::SwandiveEnd:
    return std::make_unique<StateHandler_53>(lara);
  case LaraStateId::Handstand:
    return std::make_unique<StateHandler_54>(lara);
  case LaraStateId::OnWaterExit:
    return std::make_unique<StateHandler_55>(lara);
  default:
    BOOST_LOG_TRIVIAL(error) << "No state handler for state " << toString(id);
    BOOST_THROW_EXCEPTION(std::runtime_error("Unhandled state"));
  }
}

void AbstractStateHandler::setAir(const core::Frame& a) noexcept
{
  m_lara->setAir(a);
}

void AbstractStateHandler::setMovementAngle(const core::Angle& angle) noexcept
{
  m_lara->setMovementAngle(angle);
}

core::Angle AbstractStateHandler::getMovementAngle() const noexcept
{
  return m_lara->getMovementAngle();
}

objects::HandStatus AbstractStateHandler::getHandStatus() const noexcept
{
  return m_lara->getHandStatus();
}

void AbstractStateHandler::setHandStatus(const objects::HandStatus status) noexcept
{
  m_lara->setHandStatus(status);
}

LaraStateId AbstractStateHandler::getCurrentAnimState() const
{
  return m_lara->getCurrentAnimState();
}

void AbstractStateHandler::setAnimation(const AnimationId anim, const std::optional<core::Frame>& firstFrame)
{
  m_lara->setAnimation(anim, firstFrame);
  m_lara->drawRoutine();
}

const world::World& AbstractStateHandler::getWorld() const
{
  return m_lara->getWorld();
}

world::World& AbstractStateHandler::getWorld()
{
  return m_lara->getWorld();
}

void AbstractStateHandler::placeOnFloor(const CollisionInfo& collisionInfo)
{
  m_lara->placeOnFloor(collisionInfo);
}

void AbstractStateHandler::setYRotationSpeed(const core::RotationSpeed& spd)
{
  m_lara->setYRotationSpeed(spd);
}

core::RotationSpeed AbstractStateHandler::getYRotationSpeed() const
{
  return m_lara->getYRotationSpeed();
}

void AbstractStateHandler::subYRotationSpeed(const core::RotationAcceleration& val, const core::RotationSpeed& limit)
{
  m_lara->subYRotationSpeed(val, limit);
}

void AbstractStateHandler::addYRotationSpeed(const core::RotationAcceleration& val, const core::RotationSpeed& limit)
{
  m_lara->addYRotationSpeed(val, limit);
}

void AbstractStateHandler::setFallSpeedOverride(const core::Speed& v)
{
  m_lara->setFallSpeedOverride(v);
}

void AbstractStateHandler::dampenHorizontalSpeed(const float f)
{
  m_lara->dampenHorizontalSpeed(f);
}

core::Angle AbstractStateHandler::getCurrentSlideAngle() const noexcept
{
  return m_lara->getCurrentSlideAngle();
}

void AbstractStateHandler::setCurrentSlideAngle(const core::Angle& a) noexcept
{
  m_lara->setCurrentSlideAngle(a);
}

void AbstractStateHandler::setGoalAnimState(const LaraStateId state)
{
  m_lara->setGoalAnimState(state);
}

void AbstractStateHandler::setCurrentAnimState(const LaraStateId state)
{
  m_lara->setCurrentAnimState(state);
}

LaraStateId AbstractStateHandler::getGoalAnimState() const
{
  return m_lara->getGoalAnimState();
}

bool AbstractStateHandler::stopIfCeilingBlocked(const CollisionInfo& collisionInfo)
{
  if(collisionInfo.collisionType != CollisionInfo::AxisColl::Top
     && collisionInfo.collisionType != CollisionInfo::AxisColl::Jammed)
  {
    return false;
  }

  m_lara->m_state.location.position = collisionInfo.initialPosition;

  setAnimation(AnimationId::STAY_SOLID);
  setGoalAnimState(LaraStateId::Stop);
  setCurrentAnimState(LaraStateId::Stop);
  m_lara->m_state.speed = 0_spd;
  m_lara->m_state.fallspeed = 0_spd;
  m_lara->m_state.falling = false;
  return true;
}

bool AbstractStateHandler::tryClimb(const CollisionInfo& collisionInfo)
{
  if(collisionInfo.collisionType != CollisionInfo::AxisColl::Front
     || !getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action)
     || getHandStatus() != objects::HandStatus::None)
  {
    return false;
  }

  const auto floorGradient = abs(collisionInfo.frontLeft.floor.dy - collisionInfo.frontRight.floor.dy);
  if(floorGradient >= core::MaxGrabbableGradient)
  {
    return false;
  }

  auto alignedRotation = snapRotation(m_lara->m_state.rotation.Y, 30_deg);
  if(!alignedRotation)
  {
    return false;
  }

  const auto climbHeight = collisionInfo.front.floor.dy;
  if(climbHeight >= -core::ClimbLimit2ClickMax && climbHeight <= -core::ClimbLimit2ClickMin)
  {
    if(climbHeight < collisionInfo.front.ceiling.dy
       || collisionInfo.frontLeft.floor.dy < collisionInfo.frontLeft.ceiling.dy
       || collisionInfo.frontRight.floor.dy < collisionInfo.frontRight.ceiling.dy)
    {
      return false;
    }

    setAnimation(AnimationId::CLIMB_2CLICK);
    setGoalAnimState(LaraStateId::Stop);
    setCurrentAnimState(LaraStateId::Climbing);
    m_lara->m_state.location.position.Y += 2 * core::QuarterSectorSize + climbHeight;
    setHandStatus(objects::HandStatus::Grabbing);
  }
  else if(climbHeight >= -core::ClimbLimit3ClickMax && climbHeight <= -core::ClimbLimit2ClickMax)
  {
    if(collisionInfo.front.floor.dy < collisionInfo.front.ceiling.dy
       || collisionInfo.frontLeft.floor.dy < collisionInfo.frontLeft.ceiling.dy
       || collisionInfo.frontRight.floor.dy < collisionInfo.frontRight.ceiling.dy)
    {
      return false;
    }

    setGoalAnimState(LaraStateId::Stop);
    setAnimation(AnimationId::CLIMB_3CLICK);
    setCurrentAnimState(LaraStateId::Climbing);
    m_lara->m_state.location.position.Y += 3 * core::QuarterSectorSize + climbHeight;
    setHandStatus(objects::HandStatus::Grabbing);
  }
  else if(climbHeight >= -core::JumpReachableHeight && climbHeight <= -core::ClimbLimit3ClickMax)
  {
    setAnimation(AnimationId::STAY_SOLID);
    setGoalAnimState(LaraStateId::JumpUp);
    setCurrentAnimState(LaraStateId::Stop);
    setFallSpeedOverride(-(sqrt(-2 * core::Gravity * (climbHeight + 800_len)) + 3_spd));
    m_lara->advanceFrame();
  }
  else
  {
    return false;
  }

  m_lara->m_state.rotation.Y = *alignedRotation;
  applyShift(collisionInfo);

  return true;
}

void AbstractStateHandler::applyShift(const CollisionInfo& collisionInfo)
{
  m_lara->applyShift(collisionInfo);
}

bool AbstractStateHandler::checkWallCollision(const CollisionInfo& collisionInfo)
{
  switch(collisionInfo.collisionType)
  {
  case CollisionInfo::AxisColl::FrontLeft:
    applyShift(collisionInfo);
    m_lara->m_state.rotation.Y += 5_deg;
    return false;
  case CollisionInfo::AxisColl::FrontRight:
    applyShift(collisionInfo);
    m_lara->m_state.rotation.Y -= 5_deg;
    return false;
  case CollisionInfo::AxisColl::Front:
    [[fallthrough]];
  case CollisionInfo::AxisColl::FrontTop:
    applyShift(collisionInfo);
    setGoalAnimState(LaraStateId::Stop);
    m_lara->m_state.falling = false;
    m_lara->m_state.speed = 0_spd;
    return true;
  default:
    return false;
  }
}

bool AbstractStateHandler::tryStartSlide(const CollisionInfo& collisionInfo)
{
  const auto slantX = std::labs(collisionInfo.floorSlantX);
  const auto slantZ = std::labs(collisionInfo.floorSlantZ);
  if(slantX <= 2 && slantZ <= 2)
  {
    return false;
  }

  core::Angle targetAngle{0_deg};
  if(collisionInfo.floorSlantX < -2)
  {
    targetAngle = 90_deg;
  }
  else if(collisionInfo.floorSlantX > 2)
  {
    targetAngle = -90_deg;
  }

  if(collisionInfo.floorSlantZ > std::max(2L, slantX))
  {
    targetAngle = 180_deg;
  }
  else if(collisionInfo.floorSlantZ < std::min(-2L, -slantX))
  {
    targetAngle = 0_deg;
  }

  const core::Angle dy = abs(targetAngle - m_lara->m_state.rotation.Y);
  applyShift(collisionInfo);
  if(dy > 90_deg)
  {
    if(getCurrentAnimState() != LaraStateId::SlideBackward || targetAngle != getCurrentSlideAngle())
    {
      setAnimation(AnimationId::START_SLIDE_BACKWARD);
      setGoalAnimState(LaraStateId::SlideBackward);
      setCurrentAnimState(LaraStateId::SlideBackward);
      setMovementAngle(targetAngle);
      setCurrentSlideAngle(targetAngle);
      m_lara->m_state.rotation.Y = targetAngle - 180_deg;
    }
  }
  else
  {
    if(getCurrentAnimState() != LaraStateId::SlideForward || targetAngle != getCurrentSlideAngle())
    {
      setAnimation(AnimationId::SLIDE_FORWARD);
      setGoalAnimState(LaraStateId::SlideForward);
      setCurrentAnimState(LaraStateId::SlideForward);
      setMovementAngle(targetAngle);
      setCurrentSlideAngle(targetAngle);
      m_lara->m_state.rotation.Y = targetAngle;
    }
  }
  return true;
}

void AbstractStateHandler::commonJumpHandling(CollisionInfo& collisionInfo)
{
  collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::HeightLimit};
  collisionInfo.validCeilingHeightMin = 192_len;
  collisionInfo.facingAngle = getMovementAngle();
  collisionInfo.initHeightInfo(m_lara->m_state.location.position, getWorld(), core::LaraWalkHeight);
  checkJumpWallSmash(collisionInfo);
  if(m_lara->m_state.fallspeed <= 0_spd || collisionInfo.mid.floor.dy > 0_len)
  {
    return;
  }

  if(applyLandingDamage())
  {
    setGoalAnimState(LaraStateId::Death);
  }
  else
  {
    setGoalAnimState(LaraStateId::Stop);
  }
  m_lara->m_state.fallspeed = 0_spd;
  m_lara->m_state.falling = false;
  placeOnFloor(collisionInfo);
}

void AbstractStateHandler::commonSlideHandling(CollisionInfo& collisionInfo)
{
  collisionInfo.validFloorHeight = {-core::QuarterSectorSize * 2, core::HeightLimit};
  collisionInfo.validCeilingHeightMin = 0_len;
  collisionInfo.facingAngle = getMovementAngle();
  collisionInfo.initHeightInfo(m_lara->m_state.location.position, getWorld(), core::LaraWalkHeight);

  if(stopIfCeilingBlocked(collisionInfo))
  {
    getWorld().getAudioEngine().stopSoundEffect(TR1SoundEffect::LaraSliding, &getLara().m_state);
    return;
  }

  checkWallCollision(collisionInfo);
  if(collisionInfo.mid.floor.dy <= 200_len)
  {
    tryStartSlide(collisionInfo);
    placeOnFloor(collisionInfo);
    if(std::labs(collisionInfo.floorSlantX) <= 2 && std::labs(collisionInfo.floorSlantZ) <= 2)
    {
      getWorld().getAudioEngine().stopSoundEffect(TR1SoundEffect::LaraSliding, &getLara().m_state);
      setGoalAnimState(LaraStateId::Stop);
    }
    return;
  }

  getWorld().getAudioEngine().stopSoundEffect(TR1SoundEffect::LaraSliding, &getLara().m_state);
  if(getCurrentAnimState() == LaraStateId::SlideForward)
  {
    setAnimation(AnimationId::FREE_FALL_FORWARD);
    setGoalAnimState(LaraStateId::JumpForward);
    setCurrentAnimState(LaraStateId::JumpForward);
  }
  else
  {
    setAnimation(AnimationId::FREE_FALL_BACK);
    setGoalAnimState(LaraStateId::FallBackward);
    setCurrentAnimState(LaraStateId::FallBackward);
  }

  m_lara->m_state.fallspeed = 0_spd;
  m_lara->m_state.falling = true;
}

void AbstractStateHandler::commonEdgeHangHandling(CollisionInfo& collisionInfo)
{
  collisionInfo.validFloorHeight = {-core::HeightLimit, core::HeightLimit};
  collisionInfo.validCeilingHeightMin = 0_len;
  collisionInfo.facingAngle = getMovementAngle();
  collisionInfo.initHeightInfo(m_lara->m_state.location.position, getWorld(), core::LaraWalkHeight);
  const bool tooCloseToFloor = collisionInfo.front.floor.dy < 200_len;
  m_lara->m_state.fallspeed = 0_spd;
  m_lara->m_state.falling = false;
  setMovementAngle(m_lara->m_state.rotation.Y);
  const auto axis = axisFromAngle(getMovementAngle());
  switch(axis)
  {
  case core::Axis::Deg0:
    m_lara->m_state.location.position.Z += 2_len;
    break;
  case core::Axis::Right90:
    m_lara->m_state.location.position.X += 2_len;
    break;
  case core::Axis::Deg180:
    m_lara->m_state.location.position.Z -= 2_len;
    break;
  case core::Axis::Left90:
    m_lara->m_state.location.position.X -= 2_len;
    break;
  }

  collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::HeightLimit};
  collisionInfo.validCeilingHeightMin = 0_len;
  collisionInfo.facingAngle = getMovementAngle();
  collisionInfo.initHeightInfo(m_lara->m_state.location.position, getWorld(), core::LaraWalkHeight);
  if(!getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action) || m_lara->m_state.health <= 0_hp)
  {
    setAnimation(AnimationId::TRY_HANG_VERTICAL,
                 getWorld().getWorldGeometry().getAnimation(AnimationId::TRY_HANG_VERTICAL).firstFrame + 9_frame);
    setGoalAnimState(LaraStateId::JumpUp);
    setCurrentAnimState(LaraStateId::JumpUp);
    setHandStatus(objects::HandStatus::None);
    const auto hangDistance = collisionInfo.front.floor.dy - getLara().getBoundingBox().y.min + 2_len;
    const core::TRVec pos
      = m_lara->m_state.location.position + core::TRVec(collisionInfo.shift.X, hangDistance, collisionInfo.shift.Z);
    m_lara->m_state.location.position = pos;
    m_lara->m_state.speed = 2_spd;
    m_lara->m_state.fallspeed = 1_spd;
    m_lara->m_state.falling = true;
    return;
  }

  const auto gradient = abs(collisionInfo.frontLeft.floor.dy - collisionInfo.frontRight.floor.dy);
  if(gradient >= core::MaxGrabbableGradient || collisionInfo.mid.ceiling.dy >= 0_len
     || collisionInfo.collisionType != CollisionInfo::AxisColl::Front || tooCloseToFloor)
  {
    m_lara->m_state.location.position = collisionInfo.initialPosition;
    if(getCurrentAnimState() != LaraStateId::ShimmyLeft && getCurrentAnimState() != LaraStateId::ShimmyRight)
    {
      return;
    }

    setAnimation(AnimationId::HANG_IDLE,
                 getWorld().getWorldGeometry().getAnimation(AnimationId::HANG_IDLE).firstFrame + 21_frame);
    setGoalAnimState(LaraStateId::Hang);
    setCurrentAnimState(LaraStateId::Hang);
    return;
  }

  switch(axis)
  {
  case core::Axis::PosZ:
  case core::Axis::NegZ:
    m_lara->m_state.location.position.Z += collisionInfo.shift.Z;
    break;
  case core::Axis::PosX:
  case core::Axis::NegX:
    m_lara->m_state.location.position.X += collisionInfo.shift.X;
    break;
  }

  const auto spaceToReach = collisionInfo.front.floor.dy - getLara().getBoundingBox().y.min;
  if(spaceToReach >= -core::QuarterSectorSize && spaceToReach <= core::QuarterSectorSize)
  {
    m_lara->m_state.location.position.Y += spaceToReach;
  }
}

bool AbstractStateHandler::applyLandingDamage()
{
  const auto sector = m_lara->m_state.location.moved({}).updateRoom();
  const HeightInfo h
    = HeightInfo::fromFloor(sector,
                            m_lara->m_state.location.position - core::TRVec{0_len, core::LaraWalkHeight, 0_len},
                            getWorld().getObjectManager().getObjects());
  m_lara->m_state.floor = h.y;
  getWorld().handleCommandSequence(h.lastCommandSequenceOrDeath, false);
  const auto damageSpeed = m_lara->m_state.fallspeed - core::DamageFallSpeedThreshold;
  if(damageSpeed <= 0_spd)
  {
    return false;
  }

  static constexpr auto DeathSpeedLimit = core::DeadlyFallSpeedThreshold - core::DamageFallSpeedThreshold;
  static constexpr auto DeathSpeedLimitSq = util::square(DeathSpeedLimit);

  if(damageSpeed <= DeathSpeedLimit)
  {
    const auto dsSq = util::square(damageSpeed);
    m_lara->m_state.health -= core::LaraHealth * dsSq / DeathSpeedLimitSq;
  }
  else
  {
    m_lara->m_state.health = core::DeadHealth;
  }
  return m_lara->isDead();
}

void AbstractStateHandler::addSwimToDiveKeypressDuration(const core::Frame& n) noexcept
{
  m_lara->addSwimToDiveKeypressDuration(n);
}

void AbstractStateHandler::setSwimToDiveKeypressDuration(const core::Frame& n) noexcept
{
  m_lara->setSwimToDiveKeypressDuration(n);
}

core::Frame AbstractStateHandler::getSwimToDiveKeypressDuration() const
{
  return m_lara->getSwimToDiveKeypressDuration();
}

void AbstractStateHandler::setUnderwaterState(const objects::UnderwaterState u) noexcept
{
  m_lara->setUnderwaterState(u);
}

void AbstractStateHandler::jumpAgainstWall(CollisionInfo& collisionInfo)
{
  applyShift(collisionInfo);
  switch(collisionInfo.collisionType)
  {
  case CollisionInfo::AxisColl::None:
  case CollisionInfo::AxisColl::Front:
  case CollisionInfo::AxisColl::FrontTop:
    break;
  case CollisionInfo::AxisColl::FrontLeft:
    m_lara->m_state.rotation.Y += 5_deg;
    break;
  case CollisionInfo::AxisColl::FrontRight:
    m_lara->m_state.rotation.Y -= 5_deg;
    break;
  case CollisionInfo::AxisColl::Top:
    if(m_lara->m_state.fallspeed <= 0_spd)
    {
      m_lara->m_state.fallspeed = 1_spd;
    }
    break;
  case CollisionInfo::AxisColl::Jammed:
    m_lara->m_state.location.move(util::pitch(core::DefaultCollisionRadius, m_lara->m_state.rotation.Y));
    m_lara->m_state.speed = 0_spd;
    collisionInfo.mid.floor.dy = 0_len;
    if(m_lara->m_state.fallspeed < 0_spd)
    {
      m_lara->m_state.fallspeed = 16_spd;
    }
    break;
  }
}

void AbstractStateHandler::checkJumpWallSmash(CollisionInfo& collisionInfo)
{
  applyShift(collisionInfo);

  switch(collisionInfo.collisionType)
  {
  case CollisionInfo::AxisColl::None:
    break;
  case CollisionInfo::AxisColl::Front:
    [[fallthrough]];
  case CollisionInfo::AxisColl::FrontTop:
    m_lara->m_state.speed /= 4;
    setMovementAngle(getMovementAngle() - 180_deg);
    setAnimation(AnimationId::SMASH_JUMP,
                 getWorld().getWorldGeometry().getAnimation(AnimationId::SMASH_JUMP).firstFrame + 1_frame);
    setGoalAnimState(LaraStateId::FreeFall);
    setCurrentAnimState(LaraStateId::FreeFall);
    if(m_lara->m_state.fallspeed <= 0_spd)
    {
      m_lara->m_state.fallspeed = 1_spd;
    }
    break;
  case CollisionInfo::AxisColl::FrontLeft:
    m_lara->m_state.rotation.Y += 5_deg;
    break;
  case CollisionInfo::AxisColl::FrontRight:
    m_lara->m_state.rotation.Y -= 5_deg;
    break;
  case CollisionInfo::AxisColl::Jammed:
    m_lara->m_state.location.move(util::pitch(core::DefaultCollisionRadius, collisionInfo.facingAngle));
    m_lara->m_state.speed = 0_spd;
    collisionInfo.mid.floor.dy = 0_len;
    if(m_lara->m_state.fallspeed <= 0_spd)
    {
      m_lara->m_state.fallspeed = 16_spd;
    }
    break;
  case CollisionInfo::AxisColl::Top:
    if(m_lara->m_state.fallspeed <= 0_spd)
    {
      m_lara->m_state.fallspeed = 1_spd;
    }
    break;
  }
}

void AbstractStateHandler::laraAdvanceFrame()
{
  m_lara->advanceFrame();
}
} // namespace engine::lara
