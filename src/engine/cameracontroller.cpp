#include "cameracontroller.h"

#include "engine.h"
#include "objects/laraobject.h"
#include "render/portaltracer.h"
#include "serialization/objectreference.h"
#include "serialization/optional.h"

#include <utility>

namespace engine
{
CameraController::CameraController(const gsl::not_null<Engine*>& engine,
                                   gsl::not_null<std::shared_ptr<render::scene::Camera>> camera)
    : Listener{&engine->getSoundEngine()}
    , m_camera{std::move(camera)}
    , m_engine{engine}
    , m_eye{engine->getLara().m_state.position.room}
    , m_center{core::RoomBoundPosition{engine->getLara().m_state.position.room,
                                       engine->getLara().m_state.position.position}}
    , m_eyeYOffset{engine->getLara().m_state.position.position.Y - core::SectorSize}
{
  m_center->position.Y -= m_eyeYOffset;
  m_eye = m_center;
  m_eye->position.Z -= 100_len;
}

void CameraController::setRotationAroundCenter(const core::Angle& x, const core::Angle& y)
{
  setRotationAroundCenterX(x);
  setRotationAroundCenterY(y);
}

void CameraController::setRotationAroundCenterX(const core::Angle& x)
{
  m_rotationAroundCenter.X = x;
}

void CameraController::setRotationAroundCenterY(const core::Angle& y)
{
  m_rotationAroundCenter.Y = y;
}

void CameraController::setCamOverride(const floordata::CameraParameters& camParams,
                                      const uint16_t camId,
                                      const floordata::SequenceCondition condition,
                                      const bool fromHeavy,
                                      const core::Frame& timeout,
                                      const bool switchIsOn)
{
  if(m_engine->getCameras().at(camId).isActive())
    return;

  m_fixedCameraId = camId;
  if(m_mode == CameraMode::FreeLook || m_mode == CameraMode::Combat
     || condition == floordata::SequenceCondition::LaraInCombatMode)
    return;

  if(condition == floordata::SequenceCondition::ItemActivated && timeout != 0_frame && switchIsOn)
    return;

  if(condition != floordata::SequenceCondition::ItemActivated && m_fixedCameraId == m_currentFixedCameraId)
    return;

  if(camParams.timeout != 1_sec)
    m_camOverrideTimeout = camParams.timeout * core::FrameRate;

  if(camParams.oneshot)
    m_engine->getCameras()[camId].setActive(true);

  m_smoothness = 1 + (camParams.smoothness * 4);
  if(fromHeavy)
    m_mode = CameraMode::Heavy;
  else
    m_mode = CameraMode::Fixed;
}

void CameraController::handleCommandSequence(const floordata::FloorDataValue* cmdSequence)
{
  if(m_mode == CameraMode::Heavy)
    return;

  enum class Type
  {
    Invalid,
    FixedCamChange,
    NoChange
  };

  Type type = Type::NoChange;
  while(true)
  {
    const floordata::Command command{*cmdSequence++};

    if(command.opcode == floordata::CommandOpcode::LookAt && m_mode != CameraMode::FreeLook
       && m_mode != CameraMode::Combat)
    {
      m_targetObject = m_engine->getObject(command.parameter);
    }
    else if(command.opcode == floordata::CommandOpcode::SwitchCamera)
    {
      ++cmdSequence; // skip camera parameters

      if(command.parameter != m_currentFixedCameraId)
      {
        type = Type::Invalid; // new override
      }
      else
      {
        m_fixedCameraId = m_currentFixedCameraId;
        if(m_camOverrideTimeout >= 0_frame && m_mode != CameraMode::FreeLook && m_mode != CameraMode::Combat)
        {
          type = Type::FixedCamChange;
          m_mode = CameraMode::Fixed;
        }
        else
        {
          type = Type::Invalid;
          m_camOverrideTimeout = -1_frame;
        }
      }
    }

    if(command.isLast)
      break;
  }

  if(m_targetObject == nullptr)
    return;

  if(type == Type::NoChange && m_targetObject->m_state.already_looked_at && m_targetObject != m_previousObject)
    m_targetObject = nullptr;
}

// ReSharper disable once CppMemberFunctionMayBeConst
std::unordered_set<const loader::file::Portal*> CameraController::tracePortals()
{
  for(const auto& room : m_engine->getRooms())
    room.node->setVisible(false);

  return render::PortalTracer::trace(*m_eye->room, *m_engine);
}

bool CameraController::clampY(const core::TRVec& start,
                              core::TRVec& end,
                              const gsl::not_null<const loader::file::Sector*>& sector,
                              const Engine& engine)
{
  const HeightInfo floor = HeightInfo::fromFloor(sector, end, engine.getObjects());
  const HeightInfo ceiling = HeightInfo::fromCeiling(sector, end, engine.getObjects());

  const auto d = end - start;
  if(floor.y < end.Y && floor.y > start.Y)
  {
    end.Y = floor.y;
    end.X = d.X * (floor.y - start.Y) / d.Y + start.X;
    end.Z = d.Z * (floor.y - start.Y) / d.Y + start.Z;
    return false;
  }

  if(ceiling.y > end.Y && ceiling.y < start.Y)
  {
    end.Y = ceiling.y;
    end.X = d.X * (ceiling.y - start.Y) / d.Y + start.X;
    end.Z = d.Z * (ceiling.y - start.Y) / d.Y + start.Z;
    return false;
  }

  return true;
}

CameraController::ClampType CameraController::clampAlongX(const core::RoomBoundPosition& start,
                                                          core::RoomBoundPosition& end,
                                                          const Engine& engine)
{
  if(end.position.X == start.position.X)
  {
    return ClampType::None;
  }

  const auto d = end.position - start.position;

  const auto sign = d.X < 0_len ? -1 : 1;

  core::TRVec testPos;
  testPos.X = (start.position.X / core::SectorSize) * core::SectorSize;
  if(sign > 0)
    testPos.X += core::SectorSize - 1_len;

  testPos.Y = start.position.Y + (testPos.X - start.position.X) * d.Y / d.X;
  testPos.Z = start.position.Z + (testPos.X - start.position.X) * d.Z / d.X;

  core::TRVec step;
  step.X = sign * core::SectorSize;
  step.Y = step.X * d.Y / d.X;
  step.Z = step.X * d.Z / d.X;

  end.room = start.room;

  while(true)
  {
    if(sign > 0 && testPos.X >= end.position.X)
    {
      return ClampType::None;
    }
    if(sign < 0 && testPos.X <= end.position.X)
    {
      return ClampType::None;
    }

    auto sector = findRealFloorSector(testPos, &end.room);
    if(testPos.Y > HeightInfo::fromFloor(sector, testPos, engine.getObjects()).y
       || testPos.Y < HeightInfo::fromCeiling(sector, testPos, engine.getObjects()).y)
    {
      end.position = testPos;
      return ClampType::Ceiling;
    }

    core::TRVec heightPos = testPos;
    heightPos.X += sign * 1_len;
    auto tmp = end.room;
    sector = findRealFloorSector(heightPos, &tmp);
    if(testPos.Y > HeightInfo::fromFloor(sector, heightPos, engine.getObjects()).y
       || testPos.Y < HeightInfo::fromCeiling(sector, heightPos, engine.getObjects()).y)
    {
      end.position = testPos;
      end.room = tmp;
      return ClampType::Wall;
    }

    testPos += step;
  }
}

CameraController::ClampType CameraController::clampAlongZ(const core::RoomBoundPosition& start,
                                                          core::RoomBoundPosition& end,
                                                          const Engine& engine)
{
  if(end.position.Z == start.position.Z)
  {
    return ClampType::None;
  }

  const auto d = end.position - start.position;

  const auto sign = d.Z < 0_len ? -1 : 1;

  core::TRVec testPos;
  testPos.Z = (start.position.Z / core::SectorSize) * core::SectorSize;
  if(sign > 0)
    testPos.Z += core::SectorSize - 1_len;

  testPos.X = start.position.X + (testPos.Z - start.position.Z) * d.X / d.Z;
  testPos.Y = start.position.Y + (testPos.Z - start.position.Z) * d.Y / d.Z;

  core::TRVec step;
  step.Z = sign * core::SectorSize;
  step.X = step.Z * d.X / d.Z;
  step.Y = step.Z * d.Y / d.Z;

  end.room = start.room;

  while(true)
  {
    if(sign > 0 && testPos.Z >= end.position.Z)
    {
      return ClampType::None;
    }
    if(sign < 0 && testPos.Z <= end.position.Z)
    {
      return ClampType::None;
    }

    auto sector = findRealFloorSector(testPos, &end.room);
    if(testPos.Y > HeightInfo::fromFloor(sector, testPos, engine.getObjects()).y
       || testPos.Y < HeightInfo::fromCeiling(sector, testPos, engine.getObjects()).y)
    {
      end.position = testPos;
      return ClampType::Ceiling;
    }

    core::TRVec heightPos = testPos;
    heightPos.Z += sign * 1_len;
    auto tmp = end.room;
    sector = findRealFloorSector(heightPos, &tmp);
    if(testPos.Y > HeightInfo::fromFloor(sector, heightPos, engine.getObjects()).y
       || testPos.Y < HeightInfo::fromCeiling(sector, heightPos, engine.getObjects()).y)
    {
      end.position = testPos;
      end.room = tmp;
      return ClampType::Wall;
    }

    testPos += step;
  }
}

bool CameraController::clampPosition(const core::RoomBoundPosition& start,
                                     core::RoomBoundPosition& end,
                                     const Engine& engine)
{
  bool firstUnclamped;
  ClampType secondClamp;
  if(abs(end.position.Z - start.position.Z) <= abs(end.position.X - start.position.X))
  {
    firstUnclamped = clampAlongZ(start, end, engine) == ClampType::None;
    secondClamp = clampAlongX(start, end, engine);
  }
  else
  {
    firstUnclamped = clampAlongX(start, end, engine) == ClampType::None;
    secondClamp = clampAlongZ(start, end, engine);
  }

  if(secondClamp == ClampType::Wall)
  {
    return false;
  }

  const auto sector = loader::file::findRealFloorSector(end);
  return clampY(start.position, end.position, sector, engine) && firstUnclamped && secondClamp == ClampType::None;
}

std::unordered_set<const loader::file::Portal*> CameraController::update()
{
  m_rotationAroundCenter.X = util::clamp(m_rotationAroundCenter.X, -85_deg, +85_deg);

  if(m_mode == CameraMode::Cinematic)
  {
    if(++m_cinematicFrame >= m_engine->getCinematicFrames().size())
    {
      m_cinematicFrame = m_engine->getCinematicFrames().size() - 1;
    }

    updateCinematic(m_engine->getCinematicFrames()[m_cinematicFrame], true);
    return tracePortals();
  }

  if(m_modifier != CameraModifier::AllowSteepSlants)
    HeightInfo::skipSteepSlants = true;

  const bool fixed = m_targetObject != nullptr && (m_mode == CameraMode::Fixed || m_mode == CameraMode::Heavy);

  // if we have a fixed position, we also have an object we're looking at
  objects::Object* const focusedObject = fixed ? m_targetObject.get() : &m_engine->getLara();
  BOOST_ASSERT(focusedObject != nullptr);
  auto focusBBox = focusedObject->getBoundingBox();
  auto focusY = focusedObject->m_state.position.position.Y;
  if(fixed)
    focusY += (focusBBox.minY + focusBBox.maxY) / 2;
  else
    focusY += (focusBBox.minY - focusBBox.maxY) * 3 / 4 + focusBBox.maxY;

  if(m_targetObject != nullptr && !fixed)
  {
    // lara moves around and looks at some object, some sort of involuntary free look;
    // in this case, we have an object to look at, but the camera is _not_ fixed

    BOOST_ASSERT(m_targetObject.get() != focusedObject);
    const auto distToFocused
      = m_targetObject->m_state.position.position.distanceTo(focusedObject->m_state.position.position);
    auto eyeRotY
      = angleFromAtan(m_targetObject->m_state.position.position.X - focusedObject->m_state.position.position.X,
                      m_targetObject->m_state.position.position.Z - focusedObject->m_state.position.position.Z)
        - focusedObject->m_state.rotation.Y;
    eyeRotY /= 2;
    focusBBox = m_targetObject->getBoundingBox();
    auto eyeRotX = angleFromAtan(
      distToFocused, focusY - (focusBBox.minY + focusBBox.maxY) / 2 + m_targetObject->m_state.position.position.Y);
    eyeRotX /= 2;

    if(eyeRotY < 50_deg && eyeRotY > -50_deg && eyeRotX < 85_deg && eyeRotX > -85_deg)
    {
      eyeRotY -= m_engine->getLara().m_headRotation.Y;
      m_engine->getLara().m_headRotation.Y += util::clamp(eyeRotY, -4_deg, +4_deg);
      m_engine->getLara().m_torsoRotation.Y = m_engine->getLara().m_headRotation.Y;

      eyeRotX -= m_engine->getLara().m_headRotation.X;
      m_engine->getLara().m_headRotation.X += util::clamp(eyeRotX, -4_deg, +4_deg);
      m_engine->getLara().m_torsoRotation.X = m_engine->getLara().m_headRotation.X;

      m_mode = CameraMode::FreeLook;
      m_targetObject->m_state.already_looked_at = true;
    }
  }

  m_center->room = focusedObject->m_state.position.room;

  if(m_mode == CameraMode::FreeLook || m_mode == CameraMode::Combat)
  {
    if(m_fixed)
    {
      m_center->position.Y = focusY - core::QuarterSectorSize;
      m_smoothness = 1;
    }
    else
    {
      m_center->position.Y += (focusY - core::QuarterSectorSize - m_center->position.Y) / 4;
      if(m_mode == CameraMode::FreeLook)
        m_smoothness = 4;
      else
        m_smoothness = 8;
    }
    m_fixed = false;
    if(m_mode == CameraMode::FreeLook)
      handleFreeLook(*focusedObject);
    else
      handleEnemy(*focusedObject);
  }
  else
  {
    m_center->position.X = focusedObject->m_state.position.position.X;
    m_center->position.Z = focusedObject->m_state.position.position.Z;

    if(m_modifier == CameraModifier::FollowCenter)
    {
      const auto midZ = (focusBBox.minZ + focusBBox.maxZ) / 2;
      m_center->position += util::pitch(midZ, focusedObject->m_state.rotation.Y);
    }

    if(m_fixed == fixed)
    {
      m_fixed = false;
      m_center->position.Y += (focusY - m_center->position.Y) / 4;
    }
    else
    {
      // switching between fixed cameras, so we're not doing any smoothing
      m_fixed = true;
      m_center->position.Y = focusY;
      m_smoothness = 1;
    }

    const auto sector = loader::file::findRealFloorSector(*m_center);
    if(HeightInfo::fromFloor(sector, m_center->position, m_engine->getObjects()).y < m_center->position.Y)
      HeightInfo::skipSteepSlants = false;

    if(m_mode == CameraMode::Chase || m_modifier == CameraModifier::Chase)
      chaseObject(*focusedObject);
    else
      handleFixedCamera();
  }

  m_fixed = fixed;
  m_currentFixedCameraId = m_fixedCameraId;
  if(m_mode != CameraMode::Heavy || m_camOverrideTimeout < 0_frame)
  {
    m_modifier = CameraModifier::None;
    m_mode = CameraMode::Chase;
    m_previousObject = std::exchange(m_targetObject, nullptr);
    m_rotationAroundCenter.X = m_rotationAroundCenter.Y = 0_deg;
    m_eyeCenterDistance = core::DefaultCameraLaraDistance;
    m_fixedCameraId = -1;
  }
  HeightInfo::skipSteepSlants = false;

  return tracePortals();
}

void CameraController::handleFixedCamera()
{
  Expects(m_fixedCameraId >= 0);

  const loader::file::Camera& camera = m_engine->getCameras().at(m_fixedCameraId);
  core::RoomBoundPosition pos(&m_engine->getRooms().at(camera.room));
  pos.position = camera.position;

  if(!clampPosition(*m_center, pos, *m_engine))
  {
    // ReSharper disable once CppExpressionWithoutSideEffects
    moveIntoGeometry(pos, core::QuarterSectorSize);
  }

  m_fixed = true;
  updatePosition(pos, m_smoothness);

  if(m_camOverrideTimeout != 0_frame)
  {
    m_camOverrideTimeout -= 1_frame;
    if(m_camOverrideTimeout == 0_frame)
      m_camOverrideTimeout = -1_frame;
  }
}

core::Length CameraController::moveIntoGeometry(core::RoomBoundPosition& pos, const core::Length& margin) const
{
  const auto sector = loader::file::findRealFloorSector(pos);
  BOOST_ASSERT(sector->box != nullptr);

  const auto room = pos.room;

  if(sector->box->zmin + margin > pos.position.Z
     && isVerticallyOutsideRoom(pos.position - core::TRVec(0_len, 0_len, margin), room))
    pos.position.Z = sector->box->zmin + margin;
  else if(sector->box->zmax - margin > pos.position.Z
          && isVerticallyOutsideRoom(pos.position + core::TRVec(0_len, 0_len, margin), room))
    pos.position.Z = sector->box->zmax - margin;

  if(sector->box->xmin + margin > pos.position.X
     && isVerticallyOutsideRoom(pos.position - core::TRVec(margin, 0_len, 0_len), room))
    pos.position.X = sector->box->xmin + margin;
  else if(sector->box->xmax - margin > pos.position.X
          && isVerticallyOutsideRoom(pos.position + core::TRVec(margin, 0_len, 0_len), room))
    pos.position.X = sector->box->xmax - margin;

  auto bottom = HeightInfo::fromFloor(sector, pos.position, m_engine->getObjects()).y - margin;
  auto top = HeightInfo::fromCeiling(sector, pos.position, m_engine->getObjects()).y + margin;
  if(bottom < top)
    top = bottom = (bottom + top) / 2;

  if(pos.position.Y > bottom)
    return bottom - pos.position.Y;
  if(top > pos.position.Y)
    return top - pos.position.Y;
  return 0_len;
}

bool CameraController::isVerticallyOutsideRoom(const core::TRVec& pos,
                                               const gsl::not_null<const loader::file::Room*>& room) const
{
  const auto sector = findRealFloorSector(pos, room);
  const auto floor = HeightInfo::fromFloor(sector, pos, m_engine->getObjects()).y;
  const auto ceiling = HeightInfo::fromCeiling(sector, pos, m_engine->getObjects()).y;
  return pos.Y > floor || pos.Y <= ceiling;
}

void CameraController::updatePosition(const core::RoomBoundPosition& eyePositionGoal, const int smoothFactor)
{
  m_eye->position += (eyePositionGoal.position - m_eye->position) / smoothFactor;
  HeightInfo::skipSteepSlants = false;
  m_eye->room = eyePositionGoal.room;
  auto sector = loader::file::findRealFloorSector(*m_eye);
  auto floor = HeightInfo::fromFloor(sector, m_eye->position, m_engine->getObjects()).y - core::QuarterSectorSize;
  if(floor <= m_eye->position.Y && floor <= eyePositionGoal.position.Y)
  {
    clampPosition(*m_center, *m_eye, *m_engine);
    sector = loader::file::findRealFloorSector(*m_eye);
    floor = HeightInfo::fromFloor(sector, m_eye->position, m_engine->getObjects()).y - core::QuarterSectorSize;
  }

  auto ceiling = HeightInfo::fromCeiling(sector, m_eye->position, m_engine->getObjects()).y + core::QuarterSectorSize;
  if(floor < ceiling)
  {
    floor = ceiling = (floor + ceiling) / 2;
  }

  if(m_bounce < 0_len)
  {
    const core::TRVec tmp{util::rand15s(m_bounce), util::rand15s(m_bounce), util::rand15s(m_bounce)};
    m_eye->position += tmp;
    m_center->position += tmp;
    m_bounce += 5_len;
  }
  else if(m_bounce > 0_len)
  {
    m_eye->position.Y += m_bounce;
    m_center->position.Y += m_bounce;
    m_bounce = 0_len;
  }

  if(m_eye->position.Y > floor)
    m_eyeYOffset = floor - m_eye->position.Y;
  else if(m_eye->position.Y < ceiling)
    m_eyeYOffset = ceiling - m_eye->position.Y;
  else
    m_eyeYOffset = 0_len;

  auto camPos = m_eye->position;
  camPos.Y += m_eyeYOffset;

  // update current room
  findRealFloorSector(camPos, &m_eye->room);

  if(camPos.X != m_center->position.X || camPos.Z != m_center->position.Z)
  {
    // only apply lookAt if we won't get NaN values because of parallel up and look axes
    const auto m = lookAt(camPos.toRenderSystem(), m_center->position.toRenderSystem(), {0, 1, 0});
    m_camera->setViewMatrix(m);
  }
}

void CameraController::chaseObject(const objects::Object& object)
{
  m_rotationAroundCenter.X += object.m_state.rotation.X;
  if(m_rotationAroundCenter.X > 85_deg)
    m_rotationAroundCenter.X = 85_deg;
  else if(m_rotationAroundCenter.X < -85_deg)
    m_rotationAroundCenter.X = -85_deg;

  const auto dist = util::cos(m_eyeCenterDistance, m_rotationAroundCenter.X);
  m_eyeCenterHorizontalDistanceSq = util::square(dist);

  core::RoomBoundPosition eye(m_eye->room);
  eye.position = {0_len, util::sin(m_eyeCenterDistance, m_rotationAroundCenter.X), 0_len};

  const core::Angle y = m_rotationAroundCenter.Y + object.m_state.rotation.Y;
  eye.position += m_center->position - util::pitch(dist, y);
  clampBox(eye,
           [this](core::Length& a,
                  core::Length& b,
                  const core::Length& c,
                  const core::Length& d,
                  const core::Length& e,
                  const core::Length& f,
                  const core::Length& g,
                  const core::Length& h) { clampToCorners(m_eyeCenterHorizontalDistanceSq, a, b, c, d, e, f, g, h); });

  updatePosition(eye, m_fixed ? m_smoothness : 12);
}

void CameraController::handleFreeLook(const objects::Object& object)
{
  const auto originalCenter = m_center->position;
  m_center->position.X = object.m_state.position.position.X;
  m_center->position.Z = object.m_state.position.position.Z;
  m_rotationAroundCenter.X
    = m_engine->getLara().m_torsoRotation.X + m_engine->getLara().m_headRotation.X + object.m_state.rotation.X;
  m_rotationAroundCenter.Y
    = m_engine->getLara().m_torsoRotation.Y + m_engine->getLara().m_headRotation.Y + object.m_state.rotation.Y;
  m_eyeCenterDistance = core::DefaultCameraLaraDistance;
  m_eyeYOffset = -util::sin(core::SectorSize / 2, m_rotationAroundCenter.Y);
  m_center->position += util::pitch(m_eyeYOffset, object.m_state.rotation.Y);

  if(isVerticallyOutsideRoom(m_center->position, m_eye->room))
  {
    m_center->position.X = object.m_state.position.position.X;
    m_center->position.Z = object.m_state.position.position.Z;
  }

  m_center->position.Y += moveIntoGeometry(*m_center, core::CameraWallDistance);

  auto center = *m_center;
  center.position -= util::pitch(m_eyeCenterDistance, m_rotationAroundCenter.Y);
  center.position.Y += util::sin(m_eyeCenterDistance, m_rotationAroundCenter.X);
  center.room = m_eye->room;

  clampBox(center, &freeLookClamp);

  m_center->position.X = originalCenter.X + (m_center->position.X - originalCenter.X) / m_smoothness;
  m_center->position.Z = originalCenter.Z + (m_center->position.Z - originalCenter.Z) / m_smoothness;

  updatePosition(center, m_smoothness);
}

void CameraController::handleEnemy(const objects::Object& object)
{
  m_center->position.X = object.m_state.position.position.X;
  m_center->position.Z = object.m_state.position.position.Z;

  if(m_enemy != nullptr)
  {
    m_rotationAroundCenter.X = m_eyeRotation.X + object.m_state.rotation.X;
    m_rotationAroundCenter.Y = m_eyeRotation.Y + object.m_state.rotation.Y;
  }
  else
  {
    m_rotationAroundCenter.X
      = m_engine->getLara().m_torsoRotation.X + m_engine->getLara().m_headRotation.X + object.m_state.rotation.X;
    m_rotationAroundCenter.Y
      = m_engine->getLara().m_torsoRotation.Y + m_engine->getLara().m_headRotation.Y + object.m_state.rotation.Y;
  }

  m_eyeCenterDistance = core::CombatCameraLaraDistance;
  auto center = *m_center;
  const auto d = util::cos(m_eyeCenterDistance, m_rotationAroundCenter.X);
  center.position -= util::pitch(d.retype_as<core::Length>(), m_rotationAroundCenter.Y);
  center.position.Y += util::sin(m_eyeCenterDistance, m_rotationAroundCenter.X);
  center.room = m_eye->room;

  clampBox(center,
           [this](core::Length& a,
                  core::Length& b,
                  const core::Length& c,
                  const core::Length& d,
                  const core::Length& e,
                  const core::Length& f,
                  const core::Length& g,
                  const core::Length& h) { clampToCorners(m_eyeCenterHorizontalDistanceSq, a, b, c, d, e, f, g, h); });
  updatePosition(center, m_smoothness);
}

void CameraController::clampBox(core::RoomBoundPosition& eyePositionGoal,
                                const std::function<ClampCallback>& callback) const
{
  clampPosition(*m_center, eyePositionGoal, *m_engine);
  BOOST_ASSERT(m_center->room->getSectorByAbsolutePosition(m_center->position) != nullptr);
  auto clampBox = m_center->room->getSectorByAbsolutePosition(m_center->position)->box;
  BOOST_ASSERT(clampBox != nullptr);
  BOOST_ASSERT(eyePositionGoal.room->getSectorByAbsolutePosition(eyePositionGoal.position) != nullptr);
  if(const auto idealBox = eyePositionGoal.room->getSectorByAbsolutePosition(eyePositionGoal.position)->box)
  {
    if(!clampBox->contains(eyePositionGoal.position.X, eyePositionGoal.position.Z))
      clampBox = idealBox;
  }

  core::TRVec testPos = eyePositionGoal.position;
  testPos.Z = (testPos.Z / core::SectorSize) * core::SectorSize - 1_len;
  BOOST_ASSERT(testPos.Z % core::SectorSize == core::SectorSize - 1_len
               && abs(testPos.Z - eyePositionGoal.position.Z) <= core::SectorSize);

  auto clampZMin = clampBox->zmin;
  const bool negZVerticallyOutside = isVerticallyOutsideRoom(testPos, eyePositionGoal.room);
  if(!negZVerticallyOutside && findRealFloorSector(testPos, eyePositionGoal.room)->box != nullptr)
  {
    const auto testBox = findRealFloorSector(testPos, eyePositionGoal.room)->box;
    if(testBox->zmin < clampZMin)
      clampZMin = testBox->zmin;
  }
  clampZMin += core::QuarterSectorSize;

  testPos = eyePositionGoal.position;
  testPos.Z = (testPos.Z / core::SectorSize + 1) * core::SectorSize;
  BOOST_ASSERT(testPos.Z % core::SectorSize == 0_len
               && abs(testPos.Z - eyePositionGoal.position.Z) <= core::SectorSize);

  auto clampZMax = clampBox->zmax;
  const bool posZVerticallyOutside = isVerticallyOutsideRoom(testPos, eyePositionGoal.room);
  if(!posZVerticallyOutside && findRealFloorSector(testPos, eyePositionGoal.room)->box != nullptr)
  {
    const auto testBox = findRealFloorSector(testPos, eyePositionGoal.room)->box;
    if(testBox->zmax > clampZMax)
      clampZMax = testBox->zmax;
  }
  clampZMax -= core::QuarterSectorSize;

  testPos = eyePositionGoal.position;
  testPos.X = (testPos.X / core::SectorSize) * core::SectorSize - 1_len;
  BOOST_ASSERT(testPos.X % core::SectorSize == core::SectorSize - 1_len
               && abs(testPos.X - eyePositionGoal.position.X) <= core::SectorSize);

  auto clampXMin = clampBox->xmin;
  const bool negXVerticallyOutside = isVerticallyOutsideRoom(testPos, eyePositionGoal.room);
  if(!negXVerticallyOutside && findRealFloorSector(testPos, eyePositionGoal.room)->box != nullptr)
  {
    const auto testBox = findRealFloorSector(testPos, eyePositionGoal.room)->box;
    if(testBox->xmin < clampXMin)
      clampXMin = testBox->xmin;
  }
  clampXMin += core::QuarterSectorSize;

  testPos = eyePositionGoal.position;
  testPos.X = (testPos.X / core::SectorSize + 1) * core::SectorSize;
  BOOST_ASSERT(testPos.X % core::SectorSize == 0_len
               && abs(testPos.X - eyePositionGoal.position.X) <= core::SectorSize);

  auto clampXMax = clampBox->xmax;
  const bool posXVerticallyOutside = isVerticallyOutsideRoom(testPos, eyePositionGoal.room);
  if(!posXVerticallyOutside && findRealFloorSector(testPos, eyePositionGoal.room)->box != nullptr)
  {
    const auto testBox = findRealFloorSector(testPos, eyePositionGoal.room)->box;
    if(testBox->xmax > clampXMax)
      clampXMax = testBox->xmax;
  }
  clampXMax -= core::QuarterSectorSize;

  bool skipRoomPatch = true;
  if(negZVerticallyOutside && eyePositionGoal.position.Z < clampZMin)
  {
    skipRoomPatch = false;
    core::Length left = 0_len, right = 0_len;
    if(eyePositionGoal.position.X >= m_center->position.X)
    {
      left = clampXMin;
      right = clampXMax;
    }
    else
    {
      left = clampXMax;
      right = clampXMin;
    }
    callback(eyePositionGoal.position.Z,
             eyePositionGoal.position.X,
             m_center->position.Z,
             m_center->position.X,
             clampZMin,
             right,
             clampZMax,
             left);
  }
  else if(posZVerticallyOutside && eyePositionGoal.position.Z > clampZMax)
  {
    skipRoomPatch = false;
    core::Length left = 0_len, right = 0_len;
    if(eyePositionGoal.position.X >= m_center->position.X)
    {
      left = clampXMin;
      right = clampXMax;
    }
    else
    {
      left = clampXMax;
      right = clampXMin;
    }
    callback(eyePositionGoal.position.Z,
             eyePositionGoal.position.X,
             m_center->position.Z,
             m_center->position.X,
             clampZMax,
             right,
             clampZMin,
             left);
  }

  if(!skipRoomPatch)
  {
    // ReSharper disable once CppExpressionWithoutSideEffects
    loader::file::findRealFloorSector(eyePositionGoal);
    return;
  }

  if(negXVerticallyOutside && eyePositionGoal.position.X < clampXMin)
  {
    skipRoomPatch = false;
    core::Length left = 0_len, right = 0_len;
    if(eyePositionGoal.position.Z >= m_center->position.Z)
    {
      left = clampZMin;
      right = clampZMax;
    }
    else
    {
      left = clampZMax;
      right = clampZMin;
    }
    callback(eyePositionGoal.position.X,
             eyePositionGoal.position.Z,
             m_center->position.X,
             m_center->position.Z,
             clampXMin,
             right,
             clampXMax,
             left);
  }
  else if(posXVerticallyOutside && eyePositionGoal.position.X > clampXMax)
  {
    skipRoomPatch = false;
    core::Length left = 0_len, right = 0_len;
    if(eyePositionGoal.position.Z >= m_center->position.Z)
    {
      left = clampZMin;
      right = clampZMax;
    }
    else
    {
      left = clampZMax;
      right = clampZMin;
    }
    callback(eyePositionGoal.position.X,
             eyePositionGoal.position.Z,
             m_center->position.X,
             m_center->position.Z,
             clampXMax,
             right,
             clampXMin,
             left);
  }

  if(!skipRoomPatch)
  {
    // ReSharper disable once CppExpressionWithoutSideEffects
    loader::file::findRealFloorSector(eyePositionGoal);
  }
}

void CameraController::freeLookClamp(core::Length& currentFrontBack,
                                     core::Length& currentLeftRight,
                                     const core::Length& targetFrontBack,
                                     const core::Length& targetLeftRight,
                                     const core::Length& back,
                                     const core::Length& right,
                                     const core::Length& front,
                                     const core::Length& left)
{
  if((front > back) != (targetFrontBack < back))
  {
    currentFrontBack = back;
    currentLeftRight
      = targetLeftRight
        + (currentLeftRight - targetLeftRight) * (back - targetFrontBack) / (currentFrontBack - targetFrontBack);
  }
  else if((right < left && targetLeftRight > right && right > currentLeftRight)
          || (right > left && targetLeftRight < right && right < currentLeftRight))
  {
    currentFrontBack
      = targetFrontBack
        + (currentFrontBack - targetFrontBack) * (right - targetLeftRight) / (currentLeftRight - targetLeftRight);
    currentLeftRight = right;
  }
}

void CameraController::clampToCorners(const core::Area& targetHorizontalDistanceSq,
                                      core::Length& currentFrontBack,
                                      core::Length& currentLeftRight,
                                      const core::Length& targetFrontBack,
                                      const core::Length& targetLeftRight,
                                      const core::Length& back,
                                      const core::Length& right,
                                      const core::Length& front,
                                      const core::Length& left)
{
  const auto targetRightDistSq = util::square(targetLeftRight - right);
  const auto targetBackDistSq = util::square(targetFrontBack - back);

  // back right
  const auto backRightDistSq = targetBackDistSq + targetRightDistSq;
  if(backRightDistSq > targetHorizontalDistanceSq)
  {
    currentFrontBack = back;
    if(targetHorizontalDistanceSq >= targetBackDistSq)
    {
      auto tmp = sqrt(targetHorizontalDistanceSq - targetBackDistSq);
      if(right < left)
        tmp = -tmp;
      currentLeftRight = tmp + targetLeftRight;
    }
    return;
  }

  if(backRightDistSq > util::square(core::QuarterSectorSize))
  {
    currentFrontBack = back;
    currentLeftRight = right;
    return;
  }

  // back left
  const auto targetLeftDistSq = util::square(targetLeftRight - left);
  const auto targetBackLeftDistSq = targetBackDistSq + targetLeftDistSq;
  if(targetBackLeftDistSq > targetHorizontalDistanceSq)
  {
    currentFrontBack = back;
    if(targetHorizontalDistanceSq >= targetBackDistSq)
    {
      auto tmp = sqrt(targetHorizontalDistanceSq - targetBackDistSq);
      if(right >= left)
        tmp = -tmp;
      currentLeftRight = tmp + targetLeftRight;
    }
    return;
  }

  if(targetBackLeftDistSq > util::square(core::QuarterSectorSize))
  {
    currentFrontBack = back;
    currentLeftRight = left;
    return;
  }

  // front right
  const auto targetFrontDistSq = util::square(targetFrontBack - front);
  const auto targetFrontRightDistSq = targetFrontDistSq + targetRightDistSq;

  if(targetFrontRightDistSq > targetHorizontalDistanceSq)
  {
    if(targetHorizontalDistanceSq >= targetRightDistSq)
    {
      currentLeftRight = right;
      auto tmp = sqrt(targetHorizontalDistanceSq - targetRightDistSq);
      if(back >= front)
        tmp = -tmp;
      currentFrontBack = tmp + targetFrontBack;
    }
    return;
  }

  currentFrontBack = front;
  currentLeftRight = right;
}

std::unordered_set<const loader::file::Portal*>
  CameraController::updateCinematic(const loader::file::CinematicFrame& frame, const bool ingame)
{
  if(ingame)
  {
    m_center->position += util::pitch(frame.center, m_cinematicRot.Y);
    m_cinematicPos += util::pitch(frame.eye, m_cinematicRot.Y);

    auto m = lookAt(m_cinematicPos.toRenderSystem(), m_center->position.toRenderSystem(), {0, 1, 0});
    m = rotate(m, toRad(frame.rotZ), -glm::vec3{m[2]});
    m_camera->setViewMatrix(m);
    m_camera->setFieldOfView(toRad(frame.fov));
    findRealFloorSector(m_eye->position, &m_eye->room);
  }
  else
  {
    const core::TRVec center = m_eye->position + util::pitch(frame.center, m_eyeRotation.Y);
    const core::TRVec eye = m_eye->position + util::pitch(frame.eye, m_eyeRotation.Y);

    auto m = lookAt(eye.toRenderSystem(), center.toRenderSystem(), {0, 1, 0});
    m = rotate(m, toRad(frame.rotZ), -glm::vec3{m[2]});
    m_camera->setViewMatrix(m);
    m_camera->setFieldOfView(toRad(frame.fov));
  }

  // portal tracing doesn't work here because we always render each room.
  // assuming "sane" room layout here without overlapping rooms.
  std::unordered_set<const loader::file::Portal*> result;
  for(const auto& room : getEngine()->getRooms())
  {
    if(room.isWaterRoom())
      continue;

    for(const auto& portal : room.portals)
    {
      if(getEngine()->getRooms().at(portal.adjoining_room.get()).isWaterRoom())
        result.emplace(&portal);
    }
  }
  return result;
}

CameraController::CameraController(const gsl::not_null<Engine*>& engine,
                                   gsl::not_null<std::shared_ptr<render::scene::Camera>> camera,
                                   bool /*noLaraTag*/)
    : Listener{&engine->getSoundEngine()}
    , m_camera{std::move(camera)}
    , m_engine{engine}
{
  if(engine->hasLevel())
  {
    m_eye = core::RoomBoundPosition{&engine->getRooms()[0]};
    m_center = core::RoomBoundPosition{&engine->getRooms()[0]};
  }
}

void CameraController::serialize(const serialization::Serializer& ser)
{
  ser(S_NV("eye", m_eye),
      S_NV("center", m_center),
      S_NV("mode", m_mode),
      S_NV("modifier", m_modifier),
      S_NV("fixed", m_fixed),
      S_NV("targetObject", serialization::ObjectReference{m_targetObject}),
      S_NV("previousObject", serialization::ObjectReference{m_previousObject}),
      S_NV("enemy", serialization::ObjectReference{m_enemy}),
      S_NV("eyeYOffset", m_eyeYOffset),
      S_NV("bounce", m_bounce),
      S_NV("eyeCenterDistance", m_eyeCenterDistance),
      S_NV("eyeCenterHorizontalDistanceSq", m_eyeCenterHorizontalDistanceSq),
      S_NV("eyeRotation", m_eyeRotation),
      S_NV("rotationAroundCenter", m_rotationAroundCenter),
      S_NV("smoothness", m_smoothness),
      S_NV("fixedCameraId", m_fixedCameraId),
      S_NV("currentFixedCameraId", m_currentFixedCameraId),
      S_NV("camOverrideTimeout", m_camOverrideTimeout),
      S_NV("cinematicFrame", m_cinematicFrame),
      S_NV("cinematicPos", m_cinematicPos),
      S_NV("cinematicRot", m_cinematicRot));
}
} // namespace engine
