#include "cameracontroller.h"

#include "engine.h"
#include "objects/laraobject.h"
#include "presenter.h"
#include "render/portaltracer.h"
#include "serialization/objectreference.h"
#include "serialization/optional.h"
#include "serialization/quantity.h"

#include <utility>

namespace engine
{
CameraController::CameraController(const gsl::not_null<World*>& world,
                                   gsl::not_null<std::shared_ptr<render::scene::Camera>> camera)
    : Listener{world->getPresenter().getSoundEngine().get()}
    , m_camera{std::move(camera)}
    , m_world{world}
    , m_position{world->getObjectManager().getLara().m_state.position.room}
    , m_lookAt{core::RoomBoundPosition{world->getObjectManager().getLara().m_state.position.room,
                                       world->getObjectManager().getLara().m_state.position.position}}
    , m_positionYOffset{world->getObjectManager().getLara().m_state.position.position.Y - core::SectorSize}
{
  m_lookAt.position.Y -= m_positionYOffset;
  m_position = m_lookAt;
  m_position.position.Z -= 100_len;
}

void CameraController::setRotationAroundLara(const core::Angle& x, const core::Angle& y)
{
  setRotationAroundLaraX(x);
  setRotationAroundLaraY(y);
}

void CameraController::setRotationAroundLaraX(const core::Angle& x)
{
  m_rotationAroundLara.X = x;
}

void CameraController::setRotationAroundLaraY(const core::Angle& y)
{
  m_rotationAroundLara.Y = y;
}

void CameraController::setCamOverride(const floordata::CameraParameters& camParams,
                                      const uint16_t camId,
                                      const floordata::SequenceCondition condition,
                                      const bool fromHeavy,
                                      const core::Frame& timeout,
                                      const bool switchIsOn)
{
  if(m_world->getCameras().at(camId).isActive())
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
    m_world->getCameras()[camId].setActive(true);

  m_smoothness = 1 + (camParams.smoothness * 4);
  if(fromHeavy)
    m_mode = CameraMode::HeavyFixedPosition;
  else
    m_mode = CameraMode::FixedPosition;
}

void CameraController::handleCommandSequence(const floordata::FloorDataValue* cmdSequence)
{
  if(m_mode == CameraMode::HeavyFixedPosition)
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
      m_lookAtObject = m_world->getObjectManager().getObject(command.parameter);
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
          m_mode = CameraMode::FixedPosition;
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

  if(type == Type::Invalid)
    m_lookAtObject = nullptr;

  if(m_lookAtObject == nullptr)
    return;

  if(type == Type::NoChange && m_lookAtObject->m_state.already_looked_at && m_lookAtObject != m_previousLookAtObject)
    m_lookAtObject = nullptr;
}

// ReSharper disable once CppMemberFunctionMayBeConst
std::unordered_set<const loader::file::Portal*> CameraController::tracePortals()
{
  for(const auto& room : m_world->getRooms())
    room.node->setVisible(false);

  return render::PortalTracer::trace(*m_position.room, *m_world);
}

bool CameraController::clampY(const core::TRVec& start,
                              core::TRVec& end,
                              const gsl::not_null<const loader::file::Sector*>& sector,
                              const ObjectManager& objectManager)
{
  const HeightInfo floor = HeightInfo::fromFloor(sector, end, objectManager.getObjects());
  const HeightInfo ceiling = HeightInfo::fromCeiling(sector, end, objectManager.getObjects());

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
                                                          const ObjectManager& objectManager)
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
    if(testPos.Y > HeightInfo::fromFloor(sector, testPos, objectManager.getObjects()).y
       || testPos.Y < HeightInfo::fromCeiling(sector, testPos, objectManager.getObjects()).y)
    {
      end.position = testPos;
      return ClampType::Ceiling;
    }

    core::TRVec heightPos = testPos;
    heightPos.X += sign * 1_len;
    auto tmp = end.room;
    sector = findRealFloorSector(heightPos, &tmp);
    if(testPos.Y > HeightInfo::fromFloor(sector, heightPos, objectManager.getObjects()).y
       || testPos.Y < HeightInfo::fromCeiling(sector, heightPos, objectManager.getObjects()).y)
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
                                                          const ObjectManager& objectManager)
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
    if(testPos.Y > HeightInfo::fromFloor(sector, testPos, objectManager.getObjects()).y
       || testPos.Y < HeightInfo::fromCeiling(sector, testPos, objectManager.getObjects()).y)
    {
      end.position = testPos;
      return ClampType::Ceiling;
    }

    core::TRVec heightPos = testPos;
    heightPos.Z += sign * 1_len;
    auto tmp = end.room;
    sector = findRealFloorSector(heightPos, &tmp);
    if(testPos.Y > HeightInfo::fromFloor(sector, heightPos, objectManager.getObjects()).y
       || testPos.Y < HeightInfo::fromCeiling(sector, heightPos, objectManager.getObjects()).y)
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
                                     const ObjectManager& objectManager)
{
  bool firstUnclamped;
  ClampType secondClamp;
  if(abs(end.position.Z - start.position.Z) <= abs(end.position.X - start.position.X))
  {
    firstUnclamped = clampAlongZ(start, end, objectManager) == ClampType::None;
    secondClamp = clampAlongX(start, end, objectManager);
  }
  else
  {
    firstUnclamped = clampAlongX(start, end, objectManager) == ClampType::None;
    secondClamp = clampAlongZ(start, end, objectManager);
  }

  if(secondClamp == ClampType::Wall)
  {
    return false;
  }

  const auto sector = loader::file::findRealFloorSector(end);
  return clampY(start.position, end.position, sector, objectManager) && firstUnclamped
         && secondClamp == ClampType::None;
}

std::unordered_set<const loader::file::Portal*> CameraController::update()
{
  m_rotationAroundLara.X = std::clamp(m_rotationAroundLara.X, -85_deg, +85_deg);

  if(m_mode == CameraMode::Cinematic)
  {
    if(++m_cinematicFrame >= m_world->getCinematicFrames().size())
    {
      m_cinematicFrame = m_world->getCinematicFrames().size() - 1;
    }

    updateCinematic(m_world->getCinematicFrames()[m_cinematicFrame], true);
    return tracePortals();
  }

  if(m_modifier != CameraModifier::AllowSteepSlants)
    HeightInfo::skipSteepSlants = true;

  const bool isCompletelyFixed
    = m_lookAtObject != nullptr && (m_mode == CameraMode::FixedPosition || m_mode == CameraMode::HeavyFixedPosition);

  // if we have a fixed position, we also have an object we're looking at
  const gsl::not_null focusedObject = isCompletelyFixed ? m_lookAtObject.get() : &m_world->getObjectManager().getLara();
  auto focusBBox = focusedObject->getBoundingBox();
  auto focusY = focusedObject->m_state.position.position.Y;
  if(isCompletelyFixed)
    focusY += (focusBBox.minY + focusBBox.maxY) / 2;
  else
    focusY += (focusBBox.minY - focusBBox.maxY) * 3 / 4 + focusBBox.maxY;

  if(m_lookAtObject != nullptr && !(m_mode == CameraMode::FixedPosition || m_mode == CameraMode::HeavyFixedPosition))
  {
    // lara moves around and looks at some object, some sort of involuntary free look;
    // in this case, we have an object to look at, but the camera is _not_ fixed

    BOOST_ASSERT(m_lookAtObject.get() != focusedObject);
    const auto distToFocused
      = m_lookAtObject->m_state.position.position.distanceTo(focusedObject->m_state.position.position);
    auto eyeRotY
      = angleFromAtan(m_lookAtObject->m_state.position.position.X - focusedObject->m_state.position.position.X,
                      m_lookAtObject->m_state.position.position.Z - focusedObject->m_state.position.position.Z)
        - focusedObject->m_state.rotation.Y;
    eyeRotY /= 2;
    focusBBox = m_lookAtObject->getBoundingBox();
    auto eyeRotX = angleFromAtan(
      distToFocused, focusY - (focusBBox.minY + focusBBox.maxY) / 2 + m_lookAtObject->m_state.position.position.Y);
    eyeRotX /= 2;

    if(eyeRotY < 50_deg && eyeRotY > -50_deg && eyeRotX < 85_deg && eyeRotX > -85_deg)
    {
      eyeRotY -= m_world->getObjectManager().getLara().m_headRotation.Y;
      m_world->getObjectManager().getLara().m_headRotation.Y += std::clamp(eyeRotY, -4_deg, +4_deg);
      m_world->getObjectManager().getLara().m_torsoRotation.Y = m_world->getObjectManager().getLara().m_headRotation.Y;

      eyeRotX -= m_world->getObjectManager().getLara().m_headRotation.X;
      m_world->getObjectManager().getLara().m_headRotation.X += std::clamp(eyeRotX, -4_deg, +4_deg);
      m_world->getObjectManager().getLara().m_torsoRotation.X = m_world->getObjectManager().getLara().m_headRotation.X;

      m_mode = CameraMode::FreeLook;
      m_lookAtObject->m_state.already_looked_at = true;
    }
  }

  m_lookAt.room = focusedObject->m_state.position.room;

  if(m_mode == CameraMode::FreeLook || m_mode == CameraMode::Combat)
  {
    if(m_isCompletelyFixed)
    {
      m_lookAt.position.Y = focusY - core::QuarterSectorSize;
      m_smoothness = 1;
    }
    else
    {
      m_lookAt.position.Y += (focusY - core::QuarterSectorSize - m_lookAt.position.Y) / 4;
      if(m_mode == CameraMode::FreeLook)
        m_smoothness = 4;
      else
        m_smoothness = 8;
    }
    m_isCompletelyFixed = false;
    if(m_mode == CameraMode::FreeLook)
      handleFreeLook(*focusedObject);
    else
      handleEnemy();
  }
  else
  {
    m_lookAt.position.X = focusedObject->m_state.position.position.X;
    m_lookAt.position.Z = focusedObject->m_state.position.position.Z;

    if(m_modifier == CameraModifier::FollowCenter)
    {
      const auto midZ = (focusBBox.minZ + focusBBox.maxZ) / 2;
      m_lookAt.position += util::pitch(midZ, focusedObject->m_state.rotation.Y);
    }

    bool fixedMove = false;
    if(m_isCompletelyFixed == isCompletelyFixed)
    {
      m_lookAt.position.Y += (focusY - m_lookAt.position.Y) / 4;
    }
    else
    {
      // switching between fixed cameras, so we're not doing any smoothing
      fixedMove = true;
      m_smoothness = 1;
      m_lookAt.position.Y = focusY;
    }

    const auto sector = loader::file::findRealFloorSector(m_lookAt);
    if(HeightInfo::fromFloor(sector, m_lookAt.position, m_world->getObjectManager().getObjects()).y
       < m_lookAt.position.Y)
      HeightInfo::skipSteepSlants = false;

    if(m_mode == CameraMode::Chase || m_modifier == CameraModifier::Chase)
      chaseObject(*focusedObject, fixedMove);
    else
      handleFixedCamera();
  }

  m_isCompletelyFixed = isCompletelyFixed;
  m_currentFixedCameraId = m_fixedCameraId;
  if(m_mode != CameraMode::HeavyFixedPosition || m_camOverrideTimeout < 0_frame)
  {
    m_modifier = CameraModifier::None;
    m_mode = CameraMode::Chase;
    m_previousLookAtObject = std::exchange(m_lookAtObject, nullptr);
    m_rotationAroundLara.X = m_rotationAroundLara.Y = 0_deg;
    m_distance = core::DefaultCameraLaraDistance;
    m_fixedCameraId = -1;
  }
  HeightInfo::skipSteepSlants = false;

  return tracePortals();
}

void CameraController::handleFixedCamera()
{
  Expects(m_fixedCameraId >= 0);

  const loader::file::Camera& camera = m_world->getCameras().at(m_fixedCameraId);
  core::RoomBoundPosition pos{&m_world->getRooms().at(camera.room), camera.position};

  if(!clampPosition(m_lookAt, pos, m_world->getObjectManager()))
  {
    // ReSharper disable once CppExpressionWithoutSideEffects
    moveIntoGeometry(pos, core::QuarterSectorSize);
  }

  m_isCompletelyFixed = true;
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
  Expects(sector->box != nullptr);

  if(sector->box->zmin + margin > pos.position.Z
     && isVerticallyOutsideRoom(pos.position - core::TRVec(0_len, 0_len, margin), pos.room))
    pos.position.Z = sector->box->zmin + margin;
  else if(sector->box->zmax - margin > pos.position.Z
          && isVerticallyOutsideRoom(pos.position + core::TRVec(0_len, 0_len, margin), pos.room))
    pos.position.Z = sector->box->zmax - margin;

  if(sector->box->xmin + margin > pos.position.X
     && isVerticallyOutsideRoom(pos.position - core::TRVec(margin, 0_len, 0_len), pos.room))
    pos.position.X = sector->box->xmin + margin;
  else if(sector->box->xmax - margin > pos.position.X
          && isVerticallyOutsideRoom(pos.position + core::TRVec(margin, 0_len, 0_len), pos.room))
    pos.position.X = sector->box->xmax - margin;

  auto bottom = HeightInfo::fromFloor(sector, pos.position, m_world->getObjectManager().getObjects()).y - margin;
  auto top = HeightInfo::fromCeiling(sector, pos.position, m_world->getObjectManager().getObjects()).y + margin;
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
  const auto floor = HeightInfo::fromFloor(sector, pos, m_world->getObjectManager().getObjects()).y;
  const auto ceiling = HeightInfo::fromCeiling(sector, pos, m_world->getObjectManager().getObjects()).y;
  return pos.Y >= floor || pos.Y <= ceiling;
}

void CameraController::updatePosition(const core::RoomBoundPosition& positionGoal, const int smoothFactor)
{
  m_position.position += (positionGoal.position - m_position.position) / smoothFactor;
  m_position.room = positionGoal.room;
  HeightInfo::skipSteepSlants = false;
  auto sector = loader::file::findRealFloorSector(m_position);
  auto floor = HeightInfo::fromFloor(sector, m_position.position, m_world->getObjectManager().getObjects()).y
               - core::QuarterSectorSize;
  if(floor <= m_position.position.Y && floor <= positionGoal.position.Y)
  {
    clampPosition(m_lookAt, m_position, m_world->getObjectManager());
    sector = loader::file::findRealFloorSector(m_position);
    floor = HeightInfo::fromFloor(sector, m_position.position, m_world->getObjectManager().getObjects()).y
            - core::QuarterSectorSize;
  }

  auto ceiling = HeightInfo::fromCeiling(sector, m_position.position, m_world->getObjectManager().getObjects()).y
                 + core::QuarterSectorSize;
  if(floor < ceiling)
  {
    floor = ceiling = (floor + ceiling) / 2;
  }

  if(m_bounce < 0_len)
  {
    const core::TRVec tmp{util::rand15s(m_bounce), util::rand15s(m_bounce), util::rand15s(m_bounce)};
    m_position.position += tmp;
    m_lookAt.position += tmp;
    m_bounce += 5_len;
  }
  else if(m_bounce > 0_len)
  {
    m_position.position.Y += m_bounce;
    m_lookAt.position.Y += m_bounce;
    m_bounce = 0_len;
  }

  if(m_position.position.Y > floor)
    m_positionYOffset = floor - m_position.position.Y;
  else if(m_position.position.Y < ceiling)
    m_positionYOffset = ceiling - m_position.position.Y;
  else
    m_positionYOffset = 0_len;

  auto camPos = m_position.position;
  camPos.Y += m_positionYOffset;

  // update current room
  findRealFloorSector(camPos, &m_position.room);

  if(camPos.X != m_lookAt.position.X || camPos.Z != m_lookAt.position.Z)
  {
    // only apply lookAt if we won't get NaN values because of parallel up and look axes
    const auto m = lookAt(camPos.toRenderSystem(), m_lookAt.position.toRenderSystem(), {0, 1, 0});
    m_camera->setViewMatrix(m);
  }
}

void CameraController::chaseObject(const objects::Object& object, bool fixed)
{
  m_rotationAroundLara.X += object.m_state.rotation.X;
  if(m_rotationAroundLara.X > 85_deg)
    m_rotationAroundLara.X = 85_deg;
  else if(m_rotationAroundLara.X < -85_deg)
    m_rotationAroundLara.X = -85_deg;

  const auto dist = util::cos(m_distance, m_rotationAroundLara.X);
  core::RoomBoundPosition eye{m_position.room,
                              m_lookAt.position
                                - util::pitch(dist,
                                              m_rotationAroundLara.Y + object.m_state.rotation.Y,
                                              -util::sin(m_distance, m_rotationAroundLara.X))};
  clampBox(eye,
           [distSq = util::square(dist)](core::Length& a,
                                         core::Length& b,
                                         const core::Length& c,
                                         const core::Length& d,
                                         const core::Length& e,
                                         const core::Length& f,
                                         const core::Length& g,
                                         const core::Length& h) { clampToCorners(distSq, a, b, c, d, e, f, g, h); });

  updatePosition(eye, fixed ? m_smoothness : 12);
}

void CameraController::handleFreeLook(const objects::Object& object)
{
  const auto originalCenter = m_lookAt.position;
  m_lookAt.position.X = object.m_state.position.position.X;
  m_lookAt.position.Z = object.m_state.position.position.Z;
  m_rotationAroundLara.X = m_world->getObjectManager().getLara().m_torsoRotation.X
                           + m_world->getObjectManager().getLara().m_headRotation.X + object.m_state.rotation.X;
  m_rotationAroundLara.Y = m_world->getObjectManager().getLara().m_torsoRotation.Y
                           + m_world->getObjectManager().getLara().m_headRotation.Y + object.m_state.rotation.Y;
  m_distance = core::DefaultCameraLaraDistance;
  m_positionYOffset = -util::sin(core::SectorSize / 2, m_rotationAroundLara.Y);
  m_lookAt.position += util::pitch(m_positionYOffset, object.m_state.rotation.Y);

  if(isVerticallyOutsideRoom(m_lookAt.position, m_position.room))
  {
    m_lookAt.position.X = object.m_state.position.position.X;
    m_lookAt.position.Z = object.m_state.position.position.Z;
  }

  m_lookAt.position.Y += moveIntoGeometry(m_lookAt, core::CameraWallDistance);

  auto center = m_lookAt;
  center.position -= util::pitch(m_distance, m_rotationAroundLara.Y);
  center.position.Y += util::sin(m_distance, m_rotationAroundLara.X);
  center.room = m_position.room;

  clampBox(center, &freeLookClamp);

  m_lookAt.position.X = originalCenter.X + (m_lookAt.position.X - originalCenter.X) / m_smoothness;
  m_lookAt.position.Z = originalCenter.Z + (m_lookAt.position.Z - originalCenter.Z) / m_smoothness;

  updatePosition(center, m_smoothness);
}

void CameraController::handleEnemy()
{
  m_lookAt.position.X = m_world->getObjectManager().getLara().m_state.position.position.X;
  m_lookAt.position.Z = m_world->getObjectManager().getLara().m_state.position.position.Z;

  if(m_enemy != nullptr)
  {
    m_rotationAroundLara.X = m_eyeRotation.X + m_world->getObjectManager().getLara().m_state.rotation.X;
    m_rotationAroundLara.Y = m_eyeRotation.Y + m_world->getObjectManager().getLara().m_state.rotation.Y;
  }
  else
  {
    m_rotationAroundLara.X = m_world->getObjectManager().getLara().m_torsoRotation.X
                             + m_world->getObjectManager().getLara().m_headRotation.X
                             + m_world->getObjectManager().getLara().m_state.rotation.X;
    m_rotationAroundLara.Y = m_world->getObjectManager().getLara().m_torsoRotation.Y
                             + m_world->getObjectManager().getLara().m_headRotation.Y
                             + m_world->getObjectManager().getLara().m_state.rotation.Y;
  }

  m_distance = core::CombatCameraLaraDistance;
  const auto dist = util::cos(m_distance, m_rotationAroundLara.X);
  core::RoomBoundPosition eye{
    m_position.room,
    m_lookAt.position - util::pitch(dist, m_rotationAroundLara.Y, -util::sin(m_distance, m_rotationAroundLara.X))};

  clampBox(eye,
           [distSq = util::square(dist)](core::Length& a,
                                         core::Length& b,
                                         const core::Length& c,
                                         const core::Length& d,
                                         const core::Length& e,
                                         const core::Length& f,
                                         const core::Length& g,
                                         const core::Length& h) { clampToCorners(distSq, a, b, c, d, e, f, g, h); });
  updatePosition(eye, m_smoothness);
}

void CameraController::clampBox(core::RoomBoundPosition& eyePositionGoal,
                                const std::function<ClampCallback>& callback) const
{
  clampPosition(m_lookAt, eyePositionGoal, m_world->getObjectManager());
  const auto lookAtSector = m_lookAt.room->getSectorByAbsolutePosition(m_lookAt.position);
  auto clampBox = lookAtSector == nullptr ? nullptr : lookAtSector->box;
  if(const gsl::not_null eyeSector = eyePositionGoal.room->getSectorByAbsolutePosition(eyePositionGoal.position);
     const auto idealBox = eyeSector->box)
  {
    if(lookAtSector == nullptr || !clampBox->contains(eyePositionGoal.position.X, eyePositionGoal.position.Z))
      clampBox = idealBox;
  }

  Expects(clampBox != nullptr);

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
    if(eyePositionGoal.position.X >= m_lookAt.position.X)
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
             m_lookAt.position.Z,
             m_lookAt.position.X,
             clampZMin,
             right,
             clampZMax,
             left);
  }
  else if(posZVerticallyOutside && eyePositionGoal.position.Z > clampZMax)
  {
    skipRoomPatch = false;
    core::Length left = 0_len, right = 0_len;
    if(eyePositionGoal.position.X >= m_lookAt.position.X)
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
             m_lookAt.position.Z,
             m_lookAt.position.X,
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
    if(eyePositionGoal.position.Z >= m_lookAt.position.Z)
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
             m_lookAt.position.X,
             m_lookAt.position.Z,
             clampXMin,
             right,
             clampXMax,
             left);
  }
  else if(posXVerticallyOutside && eyePositionGoal.position.X > clampXMax)
  {
    skipRoomPatch = false;
    core::Length left = 0_len, right = 0_len;
    if(eyePositionGoal.position.Z >= m_lookAt.position.Z)
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
             m_lookAt.position.X,
             m_lookAt.position.Z,
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

void CameraController::freeLookClamp(core::Length& x,
                                     core::Length& y,
                                     const core::Length& targetX,
                                     const core::Length& targetY,
                                     const core::Length& minX,
                                     const core::Length& minY,
                                     const core::Length& maxX,
                                     const core::Length& maxY)
{
  if(minX < maxX != targetX < minX)
  {
    y = targetY + (y - targetY) * (minX - targetX) / (x - targetX);
    x = minX;
  }
  if(minY < maxY && minY < targetY && y < minY || maxY < minY && targetY < minY && minY < y)
  {
    x = (x - targetX) * (minY - targetY) / (y - targetY) + targetX;
    y = minY;
  }
}

void CameraController::clampToCorners(const core::Area& targetHorizontalDistanceSq,
                                      core::Length& x,
                                      core::Length& y,
                                      const core::Length& targetX,
                                      const core::Length& targetY,
                                      const core::Length& minX,
                                      const core::Length& minY,
                                      const core::Length& maxX,
                                      const core::Length& maxY)
{
  const auto dxSqLeft = util::square(targetX - minX);
  const auto dySqTop = util::square(targetY - minY);

  const auto dxySqTopLeft = dxSqLeft + dySqTop;
  if(dxySqTopLeft > targetHorizontalDistanceSq)
  {
    x = minX;
    if(const auto delta = targetHorizontalDistanceSq - dxSqLeft; delta >= util::square(0_len))
    {
      auto tmp = sqrt(delta);
      if(minY < maxY)
        tmp = -tmp;
      y = tmp + targetY;
    }
    return;
  }

  if(dxySqTopLeft > util::square(core::QuarterSectorSize))
  {
    x = minX;
    y = minY;
    return;
  }

  const auto dxySqBottomLeft = dxSqLeft + util::square(targetY - maxY);
  if(dxySqBottomLeft > targetHorizontalDistanceSq)
  {
    x = minX;
    if(const auto delta = targetHorizontalDistanceSq - dxSqLeft; delta >= util::square(0_len))
    {
      auto tmp = sqrt(delta);
      if(minY >= maxY)
        tmp = -tmp;
      y = tmp + targetY;
    }
    return;
  }

  if(dxySqBottomLeft > util::square(core::QuarterSectorSize))
  {
    x = minX;
    y = maxY;
    return;
  }

  const auto dxSqRight = util::square(targetX - maxX);
  const auto dxySqTopRight = dxSqRight + dySqTop;

  if(targetHorizontalDistanceSq >= dxySqTopRight)
  {
    x = maxX;
    y = minY;
    return;
  }

  if(const auto delta = targetHorizontalDistanceSq - dySqTop; delta >= util::square(0_len))
  {
    y = minY;
    auto tmp = sqrt(delta);
    if(minX >= maxX)
      tmp = -tmp;
    x = tmp + targetX;
  }
}

std::unordered_set<const loader::file::Portal*>
  CameraController::updateCinematic(const loader::file::CinematicFrame& frame, const bool ingame)
{
  const core::TRVec basePos = ingame ? m_cinematicPos : m_position.position;
  const core::TRVec newLookAt = basePos + util::pitch(frame.lookAt, m_eyeRotation.Y);
  const core::TRVec newPos = basePos + util::pitch(frame.position, m_eyeRotation.Y);

  if(ingame)
  {
    m_lookAt.position = newLookAt;
    m_position.position = newPos;
    findRealFloorSector(m_position.position, &m_position.room);
  }

  auto m = lookAt(newPos.toRenderSystem(), newLookAt.toRenderSystem(), {0, 1, 0});
  m = rotate(m, toRad(frame.rotZ), -glm::vec3{m[2]});
  m_camera->setViewMatrix(m);
  m_camera->setFieldOfView(toRad(frame.fov));

  // portal tracing doesn't work here because we always render each room.
  // assuming "sane" room layout here without overlapping rooms.
  std::unordered_set<const loader::file::Portal*> result;
  for(const auto& room : getWorld()->getRooms())
  {
    if(room.isWaterRoom())
      continue;

    for(const auto& portal : room.portals)
    {
      if(getWorld()->getRooms().at(portal.adjoining_room.get()).isWaterRoom())
        result.emplace(&portal);
    }
  }
  return result;
}

CameraController::CameraController(const gsl::not_null<World*>& world,
                                   gsl::not_null<std::shared_ptr<render::scene::Camera>> camera,
                                   bool /*noLaraTag*/)
    : Listener{world->getPresenter().getSoundEngine().get()}
    , m_camera{std::move(camera)}
    , m_world{world}
    , m_position{&world->getRooms()[0]}
    , m_lookAt{&world->getRooms()[0]}
{
}

void CameraController::serialize(const serialization::Serializer& ser)
{
  ser(S_NV("position", m_position),
      S_NV("lookAt", m_lookAt),
      S_NV("mode", m_mode),
      S_NV("modifier", m_modifier),
      S_NV("completelyFixed", m_isCompletelyFixed),
      S_NV("lookAtObject", serialization::ObjectReference{m_lookAtObject}),
      S_NV("previousLookAtObject", serialization::ObjectReference{m_previousLookAtObject}),
      S_NV("enemy", serialization::ObjectReference{m_enemy}),
      S_NV("positionYOffset", m_positionYOffset),
      S_NV("bounce", m_bounce),
      S_NV("distance", m_distance),
      S_NV("eyeRotation", m_eyeRotation),
      S_NV("rotationAroundLara", m_rotationAroundLara),
      S_NV("smoothness", m_smoothness),
      S_NV("fixedCameraId", m_fixedCameraId),
      S_NV("currentFixedCameraId", m_currentFixedCameraId),
      S_NV("camOverrideTimeout", m_camOverrideTimeout),
      S_NV("cinematicFrame", m_cinematicFrame),
      S_NV("cinematicPos", m_cinematicPos),
      S_NV("cinematicRot", m_cinematicRot));
}
} // namespace engine
