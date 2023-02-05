#include "cameracontroller.h"

#include "core/boundingbox.h"
#include "core/genericvec.h"
#include "core/interval.h"
#include "engine/floordata/floordata.h"
#include "engine/floordata/types.h"
#include "engine/heightinfo.h"
#include "engine/location.h"
#include "engine/objects/object.h"
#include "engine/objects/objectstate.h"
#include "engine/world/camerasink.h"
#include "engine/world/cinematicframe.h"
#include "objectmanager.h"
#include "objects/laraobject.h"
#include "presenter.h"
#include "qs/qs.h"
#include "raycast.h"
#include "render/portaltracer.h"
#include "render/scene/camera.h"
#include "render/scene/node.h"
#include "serialization/objectreference.h" // IWYU pragma: keep
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "util/helpers.h"
#include "world/box.h"
#include "world/room.h"
#include "world/sector.h"
#include "world/world.h"

#include <algorithm>
#include <boost/assert.hpp>
#include <cstddef>
#include <exception>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <map>
#include <type_traits>
#include <utility>
#include <vector>

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

bool isVerticallyOutsideRoom(Location location, const ObjectManager& objectManager)
{
  const auto sector = location.updateRoom();
  const auto floor = HeightInfo::fromFloor(sector, location.position, objectManager.getObjects()).y;
  const auto ceiling = HeightInfo::fromCeiling(sector, location.position, objectManager.getObjects()).y;
  return location.position.Y >= floor || location.position.Y <= ceiling;
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

Location clampBox(const Location& start,
                  const core::TRVec& goal,
                  const std::function<ClampCallback>& callback,
                  const ObjectManager& objectManager)
{
  auto result = raycastLineOfSight(start, goal, objectManager).second;
  const auto startSector = gsl::not_null{start.room->getSectorByAbsolutePosition(start.position)};
  auto box = startSector->box;
  if(const auto goalSector = gsl::not_null{result.room->getSectorByAbsolutePosition(result.position)};
     const auto goalBox = goalSector->box)
  {
    if(box == nullptr || !box->xInterval.contains(result.position.X) || !box->zInterval.contains(result.position.Z))
      box = goalBox;
  }

  if(box == nullptr)
    return result;

  // align to the closest border of the next sector
  static const auto alignNeg = [](core::Length& x)
  {
    x = snappedSector(x) - 1_len;
    BOOST_ASSERT(toSectorLocal(x) == 1_sectors - 1_len);
  };
  static const auto alignPos = [](core::Length& x)
  {
    x = snappedSector(x) + 1_sectors;
    BOOST_ASSERT(toSectorLocal(x) == 0_len);
  };

  core::TRVec testPos = result.position;

  const auto testPosInvalid = [&testPos, &result, &objectManager]
  {
    return isVerticallyOutsideRoom(Location{result.room, testPos}, objectManager);
  };

  alignNeg(testPos.Z);
  BOOST_ASSERT(abs(testPos.Z - result.position.Z) <= 1_sectors);
  auto minZ = box->zInterval.min;
  const bool invalidMinZ = testPosInvalid();
  if(!invalidMinZ)
  {
    if(const auto testBox = Location{result.room, testPos}.updateRoom()->box)
      minZ = std::min(minZ, testBox->zInterval.min);
  }
  minZ += core::QuarterSectorSize;

  testPos = result.position;
  alignPos(testPos.Z);
  BOOST_ASSERT(abs(testPos.Z - result.position.Z) <= 1_sectors);
  auto maxZ = box->zInterval.max;
  const bool invalidMaxZ = testPosInvalid();
  if(!invalidMaxZ)
  {
    if(const auto testBox = Location{result.room, testPos}.updateRoom()->box)
      maxZ = std::max(maxZ, testBox->zInterval.max);
  }
  maxZ -= core::QuarterSectorSize;

  testPos = result.position;
  alignNeg(testPos.X);
  BOOST_ASSERT(abs(testPos.X - result.position.X) <= 1_sectors);
  auto minX = box->xInterval.min;
  const bool invalidMinX = testPosInvalid();
  if(!invalidMinX)
  {
    if(const auto testBox = Location{result.room, testPos}.updateRoom()->box)
      minX = std::max(minX, testBox->xInterval.min);
  }
  minX += core::QuarterSectorSize;

  testPos = result.position;
  alignPos(testPos.X);
  BOOST_ASSERT(abs(testPos.X - result.position.X) <= 1_sectors);
  auto maxX = box->xInterval.max;
  const bool invalidMaxX = testPosInvalid();
  if(!invalidMaxX)
  {
    if(const auto testBox = Location{result.room, testPos}.updateRoom()->box)
      maxX = std::max(maxX, testBox->xInterval.max);
  }
  maxX -= core::QuarterSectorSize;

  if(invalidMinZ && result.position.Z < minZ)
  {
    if(result.position.X >= start.position.X)
      std::swap(minX, maxX);
    callback(result.position.Z, result.position.X, start.position.Z, start.position.X, minZ, minX, maxZ, maxX);
    result.updateRoom();
    return result;
  }

  if(invalidMaxZ && result.position.Z > maxZ)
  {
    if(result.position.X >= start.position.X)
      std::swap(minX, maxX);
    callback(result.position.Z, result.position.X, start.position.Z, start.position.X, maxZ, minX, minZ, maxX);
    result.updateRoom();
    return result;
  }

  if(invalidMinX && result.position.X < minX)
  {
    if(result.position.Z >= start.position.Z)
      std::swap(minZ, maxZ);
    callback(result.position.X, result.position.Z, start.position.X, start.position.Z, minX, minZ, maxX, maxZ);
    result.updateRoom();
    return result;
  }

  if(invalidMaxX && result.position.X > maxX)
  {
    if(result.position.Z >= start.position.Z)
      std::swap(minZ, maxZ);
    callback(result.position.X, result.position.Z, start.position.X, start.position.Z, maxX, minZ, minX, maxZ);
    result.updateRoom();
    return result;
  }

  return result;
}
} // namespace

CameraController::CameraController(const gsl::not_null<world::World*>& world,
                                   gslu::nn_shared<render::scene::Camera> camera)
    : Listener{gsl::not_null{world->getPresenter().getSoundEngine().get()}}
    , m_camera{std::move(camera)}
    , m_world{world}
    , m_location{world->getObjectManager().getLara().m_state.location}
    , m_lookAt{world->getObjectManager().getLara().m_state.location}
{
  const auto yOffset = world->getObjectManager().getLara().m_state.location.position.Y - 1_sectors;
  m_lookAt.position.Y -= yOffset;
  m_location.position.Y -= yOffset;
  m_location.position.Z -= 100_len;
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

  m_smoothness = 1_frame + gsl::narrow_cast<core::Frame::type>(camParams.smoothness) * 4_frame;
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

std::unordered_set<const world::Portal*> CameraController::tracePortals()
{
  for(const auto& room : m_world->getRooms())
  {
    room.node->setVisible(false);
    room.node->clearScissors();
  }

  return render::PortalTracer::trace(*m_location.room, *m_world);
}

std::unordered_set<const world::Portal*> CameraController::update()
{
  m_rotationAroundLara.X = std::clamp(m_rotationAroundLara.X, -85_deg, +85_deg);

  if(m_mode == CameraMode::Cinematic)
  {
    m_cinematicFrame
      = std::min(m_cinematicFrame + 1_frame,
                 core::Frame{gsl::narrow_cast<core::Frame::type>(m_world->getCinematicFrames().size() - 1)});

    updateCinematic(m_world->getCinematicFrames()[gsl::narrow_cast<size_t>(m_cinematicFrame.get())], true);
    return tracePortals();
  }

  if(m_modifier != CameraModifier::AllowSteepSlants)
    HeightInfo::skipSteepSlants = true;

  const bool isCompletelyFixed
    = m_lookAtObject != nullptr && (m_mode == CameraMode::FixedPosition || m_mode == CameraMode::HeavyFixedPosition);

  // if we have a fixed position, we also have an object we're looking at
  const auto focusedObject
    = gsl::not_null{isCompletelyFixed ? m_lookAtObject.get() : &m_world->getObjectManager().getLara()};
  auto focusBBox = focusedObject->getBoundingBox();
  auto focusY = focusedObject->m_state.location.position.Y;
  if(isCompletelyFixed)
    focusY += focusBBox.y.mid();
  else
    focusY += focusBBox.y.max - focusBBox.y.size() * 3 / 4;

  if(m_lookAtObject != nullptr && !isCompletelyFixed)
  {
    // lara moves around and looks at some object, some sort of involuntary free look;
    // in this case, we have an object to look at, but the camera is _not_ fixed

    BOOST_ASSERT(m_lookAtObject.get() != focusedObject);
    const auto& focusedPosition = focusedObject->m_state.location.position;
    const auto& lookAtPosition = m_lookAtObject->m_state.location.position;
    const auto distToFocused = distanceTo(lookAtPosition, focusedPosition);
    auto eyeRotY = angleFromAtan(lookAtPosition.X - focusedPosition.X, lookAtPosition.Z - focusedPosition.Z)
                   - focusedObject->m_state.rotation.Y;
    eyeRotY /= 2;
    focusBBox = m_lookAtObject->getBoundingBox();
    auto eyeRotX
      = angleFromAtan(focusY - (focusBBox.y.mid() + m_lookAtObject->m_state.location.position.Y), distToFocused);
    eyeRotX /= 2;

    if(abs(eyeRotY) < 50_deg && abs(eyeRotX) < 85_deg)
    {
      auto& lara = m_world->getObjectManager().getLara();
      eyeRotY -= lara.m_headRotation.Y;
      lara.m_headRotation.Y += std::clamp(eyeRotY, -4_deg, +4_deg);
      lara.m_torsoRotation.Y = lara.m_headRotation.Y;

      eyeRotX -= lara.m_headRotation.X;
      lara.m_headRotation.X += std::clamp(eyeRotX, -4_deg, +4_deg);
      lara.m_torsoRotation.X = lara.m_headRotation.X;

      m_mode = CameraMode::FreeLook;
      m_lookAtObject->m_state.already_looked_at = true;
    }
  }

  m_lookAt.room = focusedObject->m_state.location.room;
  if(m_mode == CameraMode::FreeLook || m_mode == CameraMode::Combat)
  {
    focusY -= core::QuarterSectorSize;
    if(m_isCompletelyFixed)
    {
      m_lookAt.position.Y = focusY;
      m_smoothness = 1_frame;
    }
    else
    {
      m_lookAt.position.Y += (focusY - m_lookAt.position.Y) / 4;
      if(m_mode == CameraMode::FreeLook)
        m_smoothness = 4_frame;
      else
        m_smoothness = 8_frame;
    }
    m_isCompletelyFixed = false;
    if(m_mode == CameraMode::FreeLook)
      handleFreeLook();
    else
      handleEnemy();
  }
  else
  {
    m_lookAt.position.X = focusedObject->m_state.location.position.X;
    m_lookAt.position.Z = focusedObject->m_state.location.position.Z;

    if(m_modifier == CameraModifier::FollowCenter)
    {
      const auto midZ = focusBBox.z.mid();
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
      m_smoothness = 1_frame;
      m_lookAt.position.Y = focusY;
      m_isCompletelyFixed = true;
    }

    m_lookAt.room = focusedObject->m_state.location.room;
    const auto sector = m_lookAt.updateRoom();
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

core::Length CameraController::moveIntoBox(Location& goal, const core::Length& margin) const
{
  const auto sector = goal.updateRoom();
  if(sector->box == nullptr)
  {
    BOOST_LOG_TRIVIAL(warning) << "Invalid camera goal: " << goal;
    return 0_len;
  }

  {
    const auto narrowed = sector->box->zInterval.narrowed(margin);
    if(goal.position.Z < narrowed.min
       && isVerticallyOutsideRoom(goal.moved(0_len, 0_len, -margin), m_world->getObjectManager()))
    {
      goal.position.Z = narrowed.min;
    }
    else if(goal.position.Z > narrowed.max
            && isVerticallyOutsideRoom(goal.moved(0_len, 0_len, margin), m_world->getObjectManager()))
    {
      goal.position.Z = narrowed.max;
    }
  }

  {
    const auto narrowed = sector->box->xInterval.narrowed(margin);
    if(goal.position.X < narrowed.min
       && isVerticallyOutsideRoom(goal.moved(-margin, 0_len, 0_len), m_world->getObjectManager()))
    {
      goal.position.X = narrowed.min;
    }
    else if(goal.position.X > narrowed.max
            && isVerticallyOutsideRoom(goal.moved(margin, 0_len, 0_len), m_world->getObjectManager()))
    {
      goal.position.X = narrowed.max;
    }
  }

  auto floor = HeightInfo::fromFloor(sector, goal.position, m_world->getObjectManager().getObjects()).y - margin;
  auto ceiling = HeightInfo::fromCeiling(sector, goal.position, m_world->getObjectManager().getObjects()).y + margin;
  if(floor < ceiling)
    ceiling = floor = (floor + ceiling) / 2;

  if(goal.position.Y > floor)
    return floor - goal.position.Y;
  else if(goal.position.Y < ceiling)
    return ceiling - goal.position.Y;
  else
    return 0_len;
}

void CameraController::updatePosition(const Location& goal, const core::Frame& smoothFactor)
{
  m_location.position += (goal.position - m_location.position) / smoothFactor * 1_frame;
  m_location.room = goal.room;
  HeightInfo::skipSteepSlants = false;
  auto sector = m_location.updateRoom();
  auto floor = HeightInfo::fromFloor(sector, m_location.position, m_world->getObjectManager().getObjects()).y
               - core::QuarterSectorSize;
  if(floor <= std::min(m_location.position.Y, goal.position.Y))
  {
    m_location = raycastLineOfSight(m_lookAt, m_location.position, m_world->getObjectManager()).second;
    sector = m_location.updateRoom();
    floor = HeightInfo::fromFloor(sector, m_location.position, m_world->getObjectManager().getObjects()).y
            - core::QuarterSectorSize;
  }

  auto ceiling = HeightInfo::fromCeiling(sector, m_location.position, m_world->getObjectManager().getObjects()).y
                 + core::QuarterSectorSize;
  if(floor < ceiling)
  {
    floor = ceiling = (floor + ceiling) / 2;
  }

  if(m_bounce < 0_len)
  {
    const core::TRVec tmp{util::rand15s(m_bounce), util::rand15s(m_bounce), util::rand15s(m_bounce)};
    m_location.position += tmp;
    m_lookAt.position += tmp;
    m_bounce += 5_len;
  }
  else if(m_bounce > 0_len)
  {
    m_location.position.Y += m_bounce;
    m_lookAt.position.Y += m_bounce;
    m_bounce = 0_len;
  }

  if(m_location.position.Y > floor)
    m_location.position.Y = floor;
  else if(m_location.position.Y < ceiling)
    m_location.position.Y = ceiling;

  m_location.updateRoom();

  if(m_location.position.X != m_lookAt.position.X || m_location.position.Z != m_lookAt.position.Z)
  {
    // only apply lookAt if we won't get NaN values because of parallel up and look axes
    const auto m = lookAt(m_location.position.toRenderSystem(), m_lookAt.position.toRenderSystem(), {0, 1, 0});
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
                                  const core::Length& h)
    {
      clampToCorners(distSq, a, b, c, d, e, f, g, h);
    },
    m_world->getObjectManager());

  updatePosition(goal, m_isCompletelyFixed ? m_smoothness : 12_frame);
}

void CameraController::handleFreeLook()
{
  const auto originalLookAt = m_lookAt.position;
  const auto& lara = m_world->getObjectManager().getLara();

  m_lookAt.position.X = lara.m_state.location.position.X;
  m_lookAt.position.Z = lara.m_state.location.position.Z;

  m_rotationAroundLara.X = lara.m_torsoRotation.X + lara.m_headRotation.X + lara.m_state.rotation.X;
  m_rotationAroundLara.Y = lara.m_torsoRotation.Y + lara.m_headRotation.Y + lara.m_state.rotation.Y;
  m_distance = core::DefaultCameraLaraDistance;
  m_lookAt.position += util::pitch(util::sin(-1_sectors / 2, m_rotationAroundLara.X), lara.m_state.rotation.Y);

  if(isVerticallyOutsideRoom(m_lookAt, m_world->getObjectManager()))
  {
    m_lookAt.position.X = lara.m_state.location.position.X;
    m_lookAt.position.Z = lara.m_state.location.position.Z;
  }
  m_lookAt.position.Y += moveIntoBox(m_lookAt, core::CameraWallDistance);

  const auto goal = clampBox(m_lookAt,
                             m_lookAt.position
                               - util::pitch(util::cos(m_distance, m_rotationAroundLara.X),
                                             m_rotationAroundLara.Y,
                                             -util::sin(m_distance, m_rotationAroundLara.X)),
                             &freeLookClamp,
                             m_world->getObjectManager());

  m_lookAt.position.X = originalLookAt.X + (m_lookAt.position.X - originalLookAt.X) / m_smoothness * 1_frame;
  m_lookAt.position.Z = originalLookAt.Z + (m_lookAt.position.Z - originalLookAt.Z) / m_smoothness * 1_frame;

  updatePosition(goal, m_smoothness);
}

void CameraController::handleEnemy()
{
  const auto& lara = m_world->getObjectManager().getLara();
  m_lookAt.position.X = lara.m_state.location.position.X;
  m_lookAt.position.Z = lara.m_state.location.position.Z;

  if(m_lookAtObject != nullptr)
  {
    m_rotationAroundLara.X = lara.m_weaponTargetVector.X + lara.m_state.rotation.X;
    m_rotationAroundLara.Y = lara.m_weaponTargetVector.Y + lara.m_state.rotation.Y;
  }
  else
  {
    m_rotationAroundLara.X = lara.m_torsoRotation.X + lara.m_headRotation.X + lara.m_state.rotation.X;
    m_rotationAroundLara.Y = lara.m_torsoRotation.Y + lara.m_headRotation.Y + lara.m_state.rotation.Y;
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
                                  const core::Length& h)
    {
      clampToCorners(distSq, a, b, c, d, e, f, g, h);
    },
    m_world->getObjectManager());
  updatePosition(eye, m_smoothness);
}

std::unordered_set<const world::Portal*> CameraController::updateCinematic(const world::CinematicFrame& frame,
                                                                           const bool ingame)
{
  const core::TRVec basePos = ingame ? m_cinematicPos : m_location.position;
  const auto yRotOffset = ingame ? m_cinematicRot.Y : 0_deg;
  const core::TRVec newLookAt = basePos + util::pitch(frame.lookAt, m_eyeRotation.Y + yRotOffset);
  const core::TRVec newPos = basePos + util::pitch(frame.position, m_eyeRotation.Y + yRotOffset);

  if(ingame)
  {
    m_lookAt.position = newLookAt;
    m_location.position = newPos;
    m_location.updateRoom();
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
                                   gslu::nn_shared<render::scene::Camera> camera,
                                   bool /*noLaraTag*/)
    : Listener{gsl::not_null{world->getPresenter().getSoundEngine().get()}}
    , m_camera{std::move(camera)}
    , m_world{world}
    , m_location{gsl::not_null{&world->getRooms()[0]}}
    , m_lookAt{gsl::not_null{&world->getRooms()[0]}}
{
}

void CameraController::serialize(const serialization::Serializer<world::World>& ser) const
{
  ser(S_NV("location", m_location),
      S_NV("lookAt", m_lookAt),
      S_NV("mode", m_mode),
      S_NV("modifier", m_modifier),
      S_NV("completelyFixed", m_isCompletelyFixed),
      S_NV("lookAtObject", serialization::ObjectReference{std::cref(m_lookAtObject)}),
      S_NV("previousLookAtObject", serialization::ObjectReference{std::cref(m_previousLookAtObject)}),
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

void CameraController::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ser(S_NV("location", m_location),
      S_NV("lookAt", m_lookAt),
      S_NV("mode", m_mode),
      S_NV("modifier", m_modifier),
      S_NV("completelyFixed", m_isCompletelyFixed),
      S_NV("lookAtObject", serialization::ObjectReference{std::ref(m_lookAtObject)}),
      S_NV("previousLookAtObject", serialization::ObjectReference{std::ref(m_previousLookAtObject)}),
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
