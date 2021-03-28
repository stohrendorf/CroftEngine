#include "cameracontroller.h"

#include "engine.h"
#include "objects/laraobject.h"
#include "presenter.h"
#include "raycast.h"
#include "render/portaltracer.h"
#include "render/scene/camera.h"
#include "serialization/objectreference.h"
#include "serialization/optional.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "world/sector.h"

#include <utility>

namespace engine
{
namespace
{
void freeLookClamp(core::Length& goalX,
                   core::Length& goalY,
                   const core::Length& startX,
                   const core::Length& startY,
                   const core::Length& minX,
                   const core::Length& minY,
                   const core::Length& maxX,
                   const core::Length& maxY)
{
  const auto dx = goalX - startX;
  const auto dy = goalY - startY;
  if((minX < maxX) != (startX < minX))
  {
    goalY = startY + dy * (minX - startX) / dx;
    goalX = minX;
  }
  if((goalY < minY && minY < std::min(maxY, startY)) || (goalY > minY && minY > std::max(maxY, startY)))
  {
    goalX = startX + dx * (minY - startY) / dy;
    goalY = minY;
  }
}

bool isVerticallyOutsideRoom(const core::TRVec& pos,
                             const gsl::not_null<const world::Room*>& room,
                             const ObjectManager& objectManager)
{
  const auto sector = findRealFloorSector(pos, room);
  const auto floor = HeightInfo::fromFloor(sector, pos, objectManager.getObjects()).y;
  const auto ceiling = HeightInfo::fromCeiling(sector, pos, objectManager.getObjects()).y;
  return pos.Y >= floor || pos.Y <= ceiling;
}

void clampToCorners(const core::Area& targetHorizontalDistanceSq,
                    core::Length& x,
                    core::Length& y,
                    const core::Length& targetX,
                    const core::Length& targetY,
                    const core::Length& minX,
                    const core::Length& minY,
                    const core::Length& maxX,
                    const core::Length& maxY)
{
  const auto dxSqMinX = util::square(targetX - minX);
  const auto dySqMinY = util::square(targetY - minY);

  if(const auto dxySqMin = dxSqMinX + dySqMinY; dxySqMin > targetHorizontalDistanceSq)
  {
    x = minX;
    if(const auto delta = targetHorizontalDistanceSq - dxSqMinX; delta >= util::square(0_len))
    {
      auto tmp = sqrt(delta);
      if(minY < maxY)
        tmp = -tmp;
      y = tmp + targetY;
    }
    return;
  }
  else if(dxySqMin > util::square(core::QuarterSectorSize))
  {
    x = minX;
    y = minY;
    return;
  }

  if(const auto dxySqMinXMaxY = dxSqMinX + util::square(targetY - maxY); dxySqMinXMaxY > targetHorizontalDistanceSq)
  {
    x = minX;
    if(const auto delta = targetHorizontalDistanceSq - dxSqMinX; delta >= util::square(0_len))
    {
      auto tmp = sqrt(delta);
      if(minY >= maxY)
        tmp = -tmp;
      y = tmp + targetY;
    }
    return;
  }
  else if(dxySqMinXMaxY > util::square(core::QuarterSectorSize))
  {
    x = minX;
    y = maxY;
    return;
  }

  const auto dxySqMaxXMinY = util::square(targetX - maxX) + dySqMinY;
  if(targetHorizontalDistanceSq >= dxySqMaxXMinY)
  {
    x = maxX;
    y = minY;
    return;
  }
  if(const auto delta = targetHorizontalDistanceSq - dySqMinY; delta >= util::square(0_len))
  {
    y = minY;
    auto tmp = sqrt(delta);
    if(minX >= maxX)
      tmp = -tmp;
    x = tmp + targetX;
  }
}

using ClampCallback = void(core::Length& goalX,
                           core::Length& goalY,
                           const core::Length& startX,
                           const core::Length& startY,
                           const core::Length& minX,
                           const core::Length& minY,
                           const core::Length& maxX,
                           const core::Length& maxY);

core::RoomBoundPosition clampBox(const core::RoomBoundPosition& start,
                                 const core::TRVec& goal,
                                 const std::function<ClampCallback>& callback,
                                 const ObjectManager& objectManager)
{
  auto result = raycastLineOfSight(start, goal, objectManager).second;
  const gsl::not_null startSector = start.room->getSectorByAbsolutePosition(start.position);
  auto box = startSector->box;
  if(const gsl::not_null goalSector = result.room->getSectorByAbsolutePosition(result.position);
     const auto goalBox = goalSector->box)
  {
    if(box == nullptr || !box->contains(result.position.X, result.position.Z))
      box = goalBox;
  }

  Expects(box != nullptr);

  // align to the closest border of the next sector
  static const auto alignMin = [](core::Length& x) {
    x = (x / core::SectorSize) * core::SectorSize - 1_len;
    BOOST_ASSERT(x % core::SectorSize == core::SectorSize - 1_len);
  };
  static const auto alignMax = [](core::Length& x) {
    x = (x / core::SectorSize + 1) * core::SectorSize;
    BOOST_ASSERT(x % core::SectorSize == 0_len);
  };

  core::TRVec testPos = result.position;

  const auto testPosInvalid
    = [&testPos, &result, &objectManager] { return isVerticallyOutsideRoom(testPos, result.room, objectManager); };

  alignMin(testPos.Z);
  BOOST_ASSERT(abs(testPos.Z - result.position.Z) <= core::SectorSize);
  auto minZ = box->zmin;
  const bool invalidMinZ = testPosInvalid();
  if(!invalidMinZ && findRealFloorSector(testPos, result.room)->box != nullptr)
  {
    minZ = std::min(minZ, findRealFloorSector(testPos, result.room)->box->zmin);
  }
  minZ += core::QuarterSectorSize;

  testPos = result.position;
  alignMax(testPos.Z);
  BOOST_ASSERT(abs(testPos.Z - result.position.Z) <= core::SectorSize);
  auto maxZ = box->zmax;
  const bool invalidMaxZ = testPosInvalid();
  if(!invalidMaxZ && findRealFloorSector(testPos, result.room)->box != nullptr)
  {
    maxZ = std::max(maxZ, findRealFloorSector(testPos, result.room)->box->zmax);
  }
  maxZ -= core::QuarterSectorSize;

  testPos = result.position;
  alignMin(testPos.X);
  BOOST_ASSERT(abs(testPos.X - result.position.X) <= core::SectorSize);
  auto minX = box->xmin;
  const bool invalidMinX = testPosInvalid();
  if(!invalidMinX && findRealFloorSector(testPos, result.room)->box != nullptr)
  {
    minX = std::max(minX, findRealFloorSector(testPos, result.room)->box->xmin);
  }
  minX += core::QuarterSectorSize;

  testPos = result.position;
  alignMax(testPos.X);
  BOOST_ASSERT(abs(testPos.X - result.position.X) <= core::SectorSize);
  auto maxX = box->xmax;
  const bool invalidMaxX = testPosInvalid();
  if(!invalidMaxX && findRealFloorSector(testPos, result.room)->box != nullptr)
  {
    maxX = std::max(maxX, findRealFloorSector(testPos, result.room)->box->xmax);
  }
  maxX -= core::QuarterSectorSize;

  if(invalidMinZ && result.position.Z < minZ)
  {
    if(result.position.X >= start.position.X)
      std::swap(minX, maxX);
    callback(result.position.Z, result.position.X, start.position.Z, start.position.X, minZ, minX, maxZ, maxX);
    world::findRealFloorSector(result);
    return result;
  }

  if(invalidMaxZ && result.position.Z > maxZ)
  {
    if(result.position.X >= start.position.X)
      std::swap(minX, maxX);
    callback(result.position.Z, result.position.X, start.position.Z, start.position.X, maxZ, minX, minZ, maxX);
    world::findRealFloorSector(result);
    return result;
  }

  if(invalidMinX && result.position.X < minX)
  {
    if(result.position.Z >= start.position.Z)
      std::swap(minZ, maxZ);
    callback(result.position.X, result.position.Z, start.position.X, start.position.Z, minX, minZ, maxX, maxZ);
    world::findRealFloorSector(result);
    return result;
  }

  if(invalidMaxX && result.position.X > maxX)
  {
    if(result.position.Z >= start.position.Z)
      std::swap(minZ, maxZ);
    callback(result.position.X, result.position.Z, start.position.X, start.position.Z, maxX, minZ, minX, maxZ);
    world::findRealFloorSector(result);
    return result;
  }

  return result;
}
} // namespace

CameraController::CameraController(const gsl::not_null<world::World*>& world,
                                   gsl::not_null<std::shared_ptr<render::scene::Camera>> camera)
    : Listener{world->getPresenter().getSoundEngine().get()}
    , m_camera{std::move(camera)}
    , m_world{world}
    , m_position{world->getObjectManager().getLara().m_state.position}
    , m_lookAt{world->getObjectManager().getLara().m_state.position}
{
  const auto yOffset = world->getObjectManager().getLara().m_state.position.position.Y - core::SectorSize;
  m_lookAt.position.Y -= yOffset;
  m_position.position.Y -= yOffset;
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
  if(m_world->getCameraSinks().at(camId).isActive())
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
  else
    m_camOverrideTimeout = 1_frame;

  if(camParams.oneshot)
    m_world->getCameraSinks()[camId].setActive(true);

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
std::unordered_set<const world::Portal*> CameraController::tracePortals()
{
  for(const auto& room : m_world->getRooms())
    room.node->setVisible(false);

  auto result = render::PortalTracer::trace(*m_position.room, *m_world);

  for(const auto& portal : m_position.room->portals)
    portal.adjoiningRoom->node->setVisible(true);

  return result;
}

std::unordered_set<const world::Portal*> CameraController::update()
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

  if(m_lookAtObject != nullptr && !isCompletelyFixed)
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
      focusY - ((focusBBox.minY + focusBBox.maxY) / 2 + m_lookAtObject->m_state.position.position.Y), distToFocused);
    eyeRotX /= 2;

    if(abs(eyeRotY) < 50_deg && abs(eyeRotX) < 85_deg)
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
    focusY -= core::QuarterSectorSize;
    if(m_isCompletelyFixed)
    {
      m_lookAt.position.Y = focusY;
      m_smoothness = 1;
    }
    else
    {
      m_lookAt.position.Y += (focusY - m_lookAt.position.Y) / 4;
      if(m_mode == CameraMode::FreeLook)
        m_smoothness = 4;
      else
        m_smoothness = 8;
    }
    m_isCompletelyFixed = false;
    if(m_mode == CameraMode::FreeLook)
      handleFreeLook(*focusedObject);
    else
      handleEnemy(*focusedObject);
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

    if(m_isCompletelyFixed == isCompletelyFixed)
    {
      m_lookAt.position.Y += (focusY - m_lookAt.position.Y) / 4;
      m_isCompletelyFixed = false;
    }
    else
    {
      // switching between fixed cameras, so we're not doing any smoothing
      m_smoothness = 1;
      m_lookAt.position.Y = focusY;
      m_isCompletelyFixed = true;
    }

    m_lookAt.room = focusedObject->m_state.position.room;
    const auto sector = world::findRealFloorSector(m_lookAt);
    if(HeightInfo::fromFloor(sector, m_lookAt.position, m_world->getObjectManager().getObjects()).y
       < m_lookAt.position.Y)
      HeightInfo::skipSteepSlants = false;

    if(m_mode == CameraMode::Chase || m_modifier == CameraModifier::Chase)
      chaseObject(*focusedObject);
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

  const auto& camera = m_world->getCameraSinks().at(m_fixedCameraId);
  auto [success, goal] = raycastLineOfSight(m_lookAt, camera.position, m_world->getObjectManager());
  if(!success)
  {
    moveIntoBox(goal, core::QuarterSectorSize);
  }

  m_isCompletelyFixed = true;
  updatePosition(goal, m_smoothness);

  if(m_camOverrideTimeout != 0_frame)
  {
    m_camOverrideTimeout -= 1_frame;
    if(m_camOverrideTimeout == 0_frame)
      m_camOverrideTimeout = -1_frame;
  }
}

core::Length CameraController::moveIntoBox(core::RoomBoundPosition& goal, const core::Length& margin) const
{
  const auto sector = world::findRealFloorSector(goal);
  Expects(sector->box != nullptr);

  if(goal.position.Z < sector->box->zmin + margin
     && isVerticallyOutsideRoom(
       goal.position - core::TRVec(0_len, 0_len, margin), goal.room, m_world->getObjectManager()))
  {
    goal.position.Z = sector->box->zmin + margin;
  }
  else if(goal.position.Z > sector->box->zmax - margin
          && isVerticallyOutsideRoom(
            goal.position + core::TRVec(0_len, 0_len, margin), goal.room, m_world->getObjectManager()))
  {
    goal.position.Z = sector->box->zmax - margin;
  }

  if(goal.position.X < sector->box->xmin + margin
     && isVerticallyOutsideRoom(
       goal.position - core::TRVec(margin, 0_len, 0_len), goal.room, m_world->getObjectManager()))
  {
    goal.position.X = sector->box->xmin + margin;
  }
  else if(goal.position.X > sector->box->xmax - margin
          && isVerticallyOutsideRoom(
            goal.position + core::TRVec(margin, 0_len, 0_len), goal.room, m_world->getObjectManager()))
  {
    goal.position.X = sector->box->xmax - margin;
  }

  auto bottom = HeightInfo::fromFloor(sector, goal.position, m_world->getObjectManager().getObjects()).y - margin;
  auto top = HeightInfo::fromCeiling(sector, goal.position, m_world->getObjectManager().getObjects()).y + margin;
  if(bottom < top)
    top = bottom = (bottom + top) / 2;

  if(goal.position.Y > bottom)
    return bottom - goal.position.Y;
  else if(goal.position.Y < top)
    return top - goal.position.Y;
  else
    return 0_len;
}

void CameraController::updatePosition(const core::RoomBoundPosition& goal, const int smoothFactor)
{
  m_position.position += (goal.position - m_position.position) / smoothFactor;
  m_position.room = goal.room;
  HeightInfo::skipSteepSlants = false;
  auto sector = world::findRealFloorSector(m_position);
  auto floor = HeightInfo::fromFloor(sector, m_position.position, m_world->getObjectManager().getObjects()).y
               - core::QuarterSectorSize;
  if(floor <= std::min(m_position.position.Y, goal.position.Y))
  {
    m_position = raycastLineOfSight(m_lookAt, m_position.position, m_world->getObjectManager()).second;
    sector = world::findRealFloorSector(m_position);
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

  auto camPos = m_position.position;
  if(m_position.position.Y > floor)
    camPos.Y = floor;
  else if(m_position.position.Y < ceiling)
    camPos.Y = ceiling;

  // update current room
  findRealFloorSector(camPos, &m_position.room);

  if(camPos.X != m_lookAt.position.X || camPos.Z != m_lookAt.position.Z)
  {
    // only apply lookAt if we won't get NaN values because of parallel up and look axes
    const auto m = lookAt(camPos.toRenderSystem(), m_lookAt.position.toRenderSystem(), {0, 1, 0});
    m_camera->setViewMatrix(m);
  }
}

void CameraController::chaseObject(const objects::Object& object)
{
  m_rotationAroundLara.X += object.m_state.rotation.X;
  if(m_rotationAroundLara.X > 85_deg)
    m_rotationAroundLara.X = 85_deg;
  else if(m_rotationAroundLara.X < -85_deg)
    m_rotationAroundLara.X = -85_deg;

  const auto dist = util::cos(m_distance, m_rotationAroundLara.X);
  auto goal = clampBox(
    m_lookAt,
    m_lookAt.position
      - util::pitch(
        dist, m_rotationAroundLara.Y + object.m_state.rotation.Y, -util::sin(m_distance, m_rotationAroundLara.X)),
    [distSq = util::square(dist)](core::Length& a,
                                  core::Length& b,
                                  const core::Length& c,
                                  const core::Length& d,
                                  const core::Length& e,
                                  const core::Length& f,
                                  const core::Length& g,
                                  const core::Length& h) { clampToCorners(distSq, a, b, c, d, e, f, g, h); },
    m_world->getObjectManager());

  updatePosition(goal, m_isCompletelyFixed ? m_smoothness : 12);
}

void CameraController::handleFreeLook(const objects::Object& object)
{
  const auto originalLookAt = m_lookAt.position;
  m_lookAt.position.X = object.m_state.position.position.X;
  m_lookAt.position.Z = object.m_state.position.position.Z;

  m_rotationAroundLara.X = m_world->getObjectManager().getLara().m_torsoRotation.X
                           + m_world->getObjectManager().getLara().m_headRotation.X + object.m_state.rotation.X;
  m_rotationAroundLara.Y = m_world->getObjectManager().getLara().m_torsoRotation.Y
                           + m_world->getObjectManager().getLara().m_headRotation.Y + object.m_state.rotation.Y;
  m_distance = core::DefaultCameraLaraDistance;
  m_lookAt.position += util::pitch(-util::sin(core::SectorSize / 2, m_rotationAroundLara.X), object.m_state.rotation.Y);

  if(isVerticallyOutsideRoom(m_lookAt.position, m_lookAt.room, m_world->getObjectManager()))
  {
    m_lookAt.position.X = object.m_state.position.position.X;
    m_lookAt.position.Z = object.m_state.position.position.Z;
  }
  m_lookAt.position.Y += moveIntoBox(m_lookAt, core::CameraWallDistance);

  auto goal = clampBox(
    m_lookAt,
    m_lookAt.position - util::pitch(m_distance, m_rotationAroundLara.Y, -util::sin(m_distance, m_rotationAroundLara.X)),
    &freeLookClamp,
    m_world->getObjectManager());

  m_lookAt.position.X = originalLookAt.X + (m_lookAt.position.X - originalLookAt.X) / m_smoothness;
  m_lookAt.position.Z = originalLookAt.Z + (m_lookAt.position.Z - originalLookAt.Z) / m_smoothness;

  updatePosition(goal, m_smoothness);
}

void CameraController::handleEnemy(objects::Object& object)
{
  m_lookAt.position.X = object.m_state.position.position.X;
  m_lookAt.position.Z = object.m_state.position.position.Z;

  if(m_enemy != nullptr)
  {
    m_rotationAroundLara.X
      = getWorld()->getObjectManager().getLara().m_weaponTargetVector.X + object.m_state.rotation.X;
    m_rotationAroundLara.Y
      = getWorld()->getObjectManager().getLara().m_weaponTargetVector.Y + object.m_state.rotation.Y;
  }
  else
  {
    m_rotationAroundLara.X = m_world->getObjectManager().getLara().m_torsoRotation.X
                             + m_world->getObjectManager().getLara().m_headRotation.X + object.m_state.rotation.X;
    m_rotationAroundLara.Y = m_world->getObjectManager().getLara().m_torsoRotation.Y
                             + m_world->getObjectManager().getLara().m_headRotation.Y + object.m_state.rotation.Y;
  }

  m_distance = core::CombatCameraLaraDistance;
  const auto dist = util::cos(m_distance, m_rotationAroundLara.X);
  auto eye = clampBox(
    m_lookAt,
    m_lookAt.position - util::pitch(dist, m_rotationAroundLara.Y, -util::sin(m_distance, m_rotationAroundLara.X)),
    [distSq = util::square(dist)](core::Length& a,
                                  core::Length& b,
                                  const core::Length& c,
                                  const core::Length& d,
                                  const core::Length& e,
                                  const core::Length& f,
                                  const core::Length& g,
                                  const core::Length& h) { clampToCorners(distSq, a, b, c, d, e, f, g, h); },
    m_world->getObjectManager());
  updatePosition(eye, m_smoothness);
}

std::unordered_set<const world::Portal*> CameraController::updateCinematic(const world::CinematicFrame& frame,
                                                                           const bool ingame)
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
  m = rotate(m, frame.rotZ, -glm::vec3{m[2]});
  m_camera->setViewMatrix(m);
  m_camera->setFieldOfView(frame.fov);

  // portal tracing doesn't work here because we always render each room.
  // assuming "sane" room layout here without overlapping rooms.
  std::unordered_set<const world::Portal*> result;
  for(const auto& room : getWorld()->getRooms())
  {
    if(room.isWaterRoom)
      continue;

    for(const auto& portal : room.portals)
    {
      if(portal.adjoiningRoom->isWaterRoom)
        result.emplace(&portal);
    }
  }
  return result;
}

CameraController::CameraController(const gsl::not_null<world::World*>& world,
                                   gsl::not_null<std::shared_ptr<render::scene::Camera>> camera,
                                   bool /*noLaraTag*/)
    : Listener{world->getPresenter().getSoundEngine().get()}
    , m_camera{std::move(camera)}
    , m_world{world}
    , m_position{&world->getRooms()[0]}
    , m_lookAt{&world->getRooms()[0]}
{
}

void CameraController::serialize(const serialization::Serializer<world::World>& ser)
{
  ser(S_NV("position", m_position),
      S_NV("lookAt", m_lookAt),
      S_NV("mode", m_mode),
      S_NV("modifier", m_modifier),
      S_NV("completelyFixed", m_isCompletelyFixed),
      S_NV("lookAtObject", serialization::ObjectReference{m_lookAtObject}),
      S_NV("previousLookAtObject", serialization::ObjectReference{m_previousLookAtObject}),
      S_NV("enemy", serialization::ObjectReference{m_enemy}),
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

glm::vec3 CameraController::getPosition() const
{
  return m_camera->getPosition();
}

glm::vec3 CameraController::getFrontVector() const
{
  return m_camera->getFrontVector();
}

glm::vec3 CameraController::getUpVector() const
{
  return m_camera->getUpVector();
}
} // namespace engine
