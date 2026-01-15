#include "cameracontroller.h"

#include "audio/listener.h"
#include "audio/soundengine.h"
#include "core/angle.h"
#include "core/boundingbox.h"
#include "core/genericvec.h"
#include "core/interval.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine.h"
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
#include <boost/log/trivial.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <gsl-lite/gsl-lite.hpp>
#include <gslu.h>
#include <unordered_set>
#include <utility>
#include <vector>

namespace engine
{
namespace
{
void freeLookClamp(core::Length& targetA,
                   core::Length& targetB,
                   const core::Length& pivotA,
                   const core::Length& pivotB,
                   const core::Length& minA,
                   const core::Length& minB,
                   const core::Length& maxA,
                   const core::Length& maxB) noexcept
{
  const auto da = targetA - pivotA;
  const auto db = targetB - pivotB;
  // ReSharper disable once CppRedundantParentheses
  if((minA < maxA) != (pivotA < minA))
  {
    targetB = pivotB + db * (minA - pivotA) / da;
    targetA = minA;
  }
  if((targetB < minB && minB < std::min(maxB, pivotB)) || (targetB > minB && minB > std::max(maxB, pivotB)))
  {
    targetA = pivotA + da * (minB - pivotB) / db;
    targetB = minB;
  }
}

bool isCollidingWithFloorOrCeiling(Location location, const ObjectManager& objectManager)
{
  const auto sector = location.updateRoom();
  const auto floor = HeightInfo::fromFloor(sector, location.position, objectManager.getObjects()).y;
  const auto ceiling = HeightInfo::fromCeiling(sector, location.position, objectManager.getObjects()).y;
  return location.position.Y >= floor || location.position.Y <= ceiling;
}

void clampToCorners(const core::Area& targetHorizontalDistanceSq,
                    core::Length& posA,
                    core::Length& posB,
                    const core::Length& pivotA,
                    const core::Length& pivotB,
                    const core::Length& minA,
                    const core::Length& minB,
                    const core::Length& maxA,
                    const core::Length& maxB)
{
  const auto daSqMinA = util::square(pivotA - minA);
  const auto dbSqMinB = util::square(pivotB - minB);

  if(const auto dabSqMin = daSqMinA + dbSqMinB; dabSqMin > targetHorizontalDistanceSq)
  {
    posA = minA;
    if(const auto delta = targetHorizontalDistanceSq - daSqMinA; delta >= util::square(0_len))
    {
      auto tmp = sqrt(delta);
      if(minB < maxB)
        tmp = -tmp;
      posB = tmp + pivotB;
    }
    return;
  }
  else if(dabSqMin > util::square(core::QuarterSectorSize))
  {
    posA = minA;
    posB = minB;
    return;
  }

  if(const auto dabSqMinAMaxB = daSqMinA + util::square(pivotB - maxB); dabSqMinAMaxB > targetHorizontalDistanceSq)
  {
    posA = minA;
    if(const auto delta = targetHorizontalDistanceSq - daSqMinA; delta >= util::square(0_len))
    {
      auto tmp = sqrt(delta);
      if(minB >= maxB)
        tmp = -tmp;
      posB = tmp + pivotB;
    }
    return;
  }
  else if(dabSqMinAMaxB > util::square(core::QuarterSectorSize))
  {
    posA = minA;
    posB = maxB;
    return;
  }

  if(const auto dabSqMaxAMinB = util::square(pivotA - maxA) + dbSqMinB; targetHorizontalDistanceSq >= dabSqMaxAMinB)
  {
    posA = maxA;
    posB = minB;
    return;
  }
  if(const auto delta = targetHorizontalDistanceSq - dbSqMinB; delta >= util::square(0_len))
  {
    posB = minB;
    auto tmp = sqrt(delta);
    if(minA >= maxA)
      tmp = -tmp;
    posA = tmp + pivotA;
  }
}

using ClampCallback = void(core::Length& posA,
                           core::Length& posB,
                           const core::Length& pivotA,
                           const core::Length& pivotB,
                           const core::Length& minA,
                           const core::Length& minB,
                           const core::Length& maxA,
                           const core::Length& maxB);

Location clampBox(const Location& start,
                  const core::TRVec& goal,
                  const std::function<ClampCallback>& callback,
                  const ObjectManager& objectManager)
{
  auto result = raycastLineOfSight(start, goal, objectManager).second;
  const auto startSector = gsl_lite::not_null{start.room->getSectorByAbsolutePosition(start.position)};
  auto box = startSector->box;
  if(const auto goalSector = gsl_lite::not_null{result.room->getSectorByAbsolutePosition(result.position)};
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
    return isCollidingWithFloorOrCeiling(Location{result.room, testPos}, objectManager);
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

CameraController::CameraController(const gsl_lite::not_null<world::World*>& world,
                                   gslu::nn_shared<render::scene::Camera> camera)
    : Listener{gsl_lite::not_null{world->getEngine().getPresenter().getSoundEngine().get().get()}}
    , m_camera{std::move(camera)}
    , m_world{world}
    , m_location{world->getObjectManager().getLara().m_state.location}
    , m_previousLocation{world->getObjectManager().getLara().m_state.location}
    , m_lookAt{world->getObjectManager().getLara().m_state.location}
    , m_previousLookAt{world->getObjectManager().getLara().m_state.location}
{
  const auto yOffset = world->getObjectManager().getLara().m_state.location.position.Y - 1_sectors;
  m_lookAt.position.Y -= yOffset;
  m_location.position.Y -= yOffset;
  m_location.position.Z -= 100_len;
}

// NOLINTNEXTLINE(*-easily-swappable-parameters)
void CameraController::setRotationAroundLara(const core::Angle& x, const core::Angle& y) noexcept
{
  setRotationAroundLaraX(x);
  setRotationAroundLaraY(y);
}

void CameraController::setRotationAroundLaraX(const core::Angle& x) noexcept
{
  m_rotationAroundLara.X = x;
}

void CameraController::setRotationAroundLaraY(const core::Angle& y) noexcept
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
    m_overrideTimeout = camParams.timeout * core::FrameRate;
  else
    m_overrideTimeout = 1_frame;

  if(camParams.oneshot)
    m_world->getCameraSinks()[camId].setActive(true);

  m_smoothingFactor = 1_frame + gsl_lite::narrow_cast<core::Frame::type>(camParams.smoothness) * 4_frame;
  if(fromHeavy)
    m_mode = CameraMode::HeavyFixedPosition;
  else
    m_mode = CameraMode::FixedPosition;
}

void CameraController::handleCommandSequence(const floordata::FloorDataValue* cmdSequence)
{
  if(m_mode == CameraMode::HeavyFixedPosition)
    return;

  enum class Type : uint8_t
  {
    Invalid,
    FixedCamChange,
    NoChange
  };

  auto type = Type::NoChange;
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
        if(m_overrideTimeout >= 0_frame && m_mode != CameraMode::FreeLook && m_mode != CameraMode::Combat)
        {
          type = Type::FixedCamChange;
          m_mode = CameraMode::FixedPosition;
        }
        else
        {
          type = Type::Invalid;
          m_overrideTimeout = -1_frame;
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

std::unordered_set<const world::Portal*> CameraController::traceWaterSurfacePortals() const
{
  for(const auto& room : m_world->getRooms())
  {
    room.node->setVisible(false);
    room.node->clearScissors();
  }

  return render::PortalTracer::trace(*m_location.room, *m_world);
}

void CameraController::updateGameLogic(const bool inGameCamera)
{
  m_previousLocation = m_location;
  m_previousLookAt = m_lookAt;

  m_rotationAroundLara.X = std::clamp(m_rotationAroundLara.X, -85_deg, +85_deg);

  if(m_mode == CameraMode::Cinematic)
  {
    tickCinematic(m_world->getCinematicFrames(), inGameCamera);
    return;
  }

  if(m_modifier != CameraModifier::AllowSteepSlants)
    HeightInfo::skipSteepSlants = true;

  const bool isInFixedMode
    = m_lookAtObject != nullptr && (m_mode == CameraMode::FixedPosition || m_mode == CameraMode::HeavyFixedPosition);

  // if we have a fixed position, we also have an object we're looking at
  const auto focusedObject
    = gsl_lite::not_null{isInFixedMode ? m_lookAtObject.get() : &m_world->getObjectManager().getLara()};
  auto focusBBox = focusedObject->getBoundingBox();
  auto lookAtHeight = focusedObject->m_state.location.position.Y;
  if(isInFixedMode)
    lookAtHeight += focusBBox.y.mid();
  else
    lookAtHeight += focusBBox.y.max - focusBBox.y.size() * 3 / 4;

  if(m_lookAtObject != nullptr && !isInFixedMode)
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
      = angleFromAtan(lookAtHeight - (focusBBox.y.mid() + m_lookAtObject->m_state.location.position.Y), distToFocused);
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
    lookAtHeight -= core::QuarterSectorSize;
    if(m_wasFixedMode)
    {
      m_lookAt.position.Y = lookAtHeight;
      m_smoothingFactor = 1_frame;
    }
    else
    {
      m_lookAt.position.Y += (lookAtHeight - m_lookAt.position.Y) / 4;
      if(m_mode == CameraMode::FreeLook)
        m_smoothingFactor = 4_frame;
      else
        m_smoothingFactor = 8_frame;
    }
    m_wasFixedMode = false;
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

    if(m_wasFixedMode == isInFixedMode)
    {
      m_lookAt.position.Y += (lookAtHeight - m_lookAt.position.Y) / 4;
      m_wasFixedMode = false;
    }
    else
    {
      // switching between fixed cameras, so we're not doing any smoothing
      m_smoothingFactor = 1_frame;
      m_lookAt.position.Y = lookAtHeight;
      m_wasFixedMode = true;
    }

    m_lookAt.room = focusedObject->m_state.location.room;
    if(const auto sector = m_lookAt.updateRoom();
       HeightInfo::fromFloor(sector, m_lookAt.position, m_world->getObjectManager().getObjects()).y
       < m_lookAt.position.Y)
      HeightInfo::skipSteepSlants = false;

    if(m_mode == CameraMode::Chase || m_modifier == CameraModifier::Chase)
      chaseObject(*focusedObject);
    else
      handleFixedCamera();
  }

  m_wasFixedMode = isInFixedMode;
  m_currentFixedCameraId = m_fixedCameraId;
  if(m_mode != CameraMode::HeavyFixedPosition || m_overrideTimeout < 0_frame)
  {
    m_modifier = CameraModifier::None;
    m_mode = CameraMode::Chase;
    m_previousLookAtObject = std::exchange(m_lookAtObject, nullptr);
    m_rotationAroundLara.X = m_rotationAroundLara.Y = 0_deg;
    m_distance = core::DefaultCameraLaraDistance;
    m_fixedCameraId = -1;
  }
  HeightInfo::skipSteepSlants = false;
}

void CameraController::handleFixedCamera()
{
  gsl_Expects(m_fixedCameraId >= 0);

  const auto& camera = m_world->getCameraSinks().at(m_fixedCameraId);
  auto [success, goal] = raycastLineOfSight(m_lookAt, camera.position, m_world->getObjectManager());
  if(!success)
  {
    clampToVerticalBounds(goal, core::QuarterSectorSize);
  }

  m_wasFixedMode = true;
  updatePosition(goal, m_smoothingFactor);

  if(m_overrideTimeout != 0_frame)
  {
    m_overrideTimeout -= 1_frame;
    if(m_overrideTimeout == 0_frame)
      m_overrideTimeout = -1_frame;
  }
}

core::Length CameraController::clampToVerticalBounds(Location& goal, const core::Length& margin) const
{
  const auto sector = goal.updateRoom();
  if(sector->box == nullptr)
  {
    BOOST_LOG_TRIVIAL(warning) << "Invalid camera goal: " << goal;
    return 0_len;
  }

  {
    if(const auto narrowed = sector->box->zInterval.narrowed(margin);
       goal.position.Z < narrowed.min
       && isCollidingWithFloorOrCeiling(goal.moved(0_len, 0_len, -margin), m_world->getObjectManager()))
    {
      goal.position.Z = narrowed.min;
    }
    else if(goal.position.Z > narrowed.max
            && isCollidingWithFloorOrCeiling(goal.moved(0_len, 0_len, margin), m_world->getObjectManager()))
    {
      goal.position.Z = narrowed.max;
    }
  }

  {
    if(const auto narrowed = sector->box->xInterval.narrowed(margin);
       goal.position.X < narrowed.min
       && isCollidingWithFloorOrCeiling(goal.moved(-margin, 0_len, 0_len), m_world->getObjectManager()))
    {
      goal.position.X = narrowed.min;
    }
    else if(goal.position.X > narrowed.max
            && isCollidingWithFloorOrCeiling(goal.moved(margin, 0_len, 0_len), m_world->getObjectManager()))
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

void CameraController::updatePosition(const Location& goal, const core::Frame& smoothingFactor)
{
  m_location.position += (goal.position - m_location.position) / smoothingFactor * 1_frame;
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

  if(m_shakeAmplitude < 0_len)
  {
    const core::TRVec tmp{
      util::rand15s(m_shakeAmplitude), util::rand15s(m_shakeAmplitude), util::rand15s(m_shakeAmplitude)};
    m_location.position += tmp;
    m_lookAt.position += tmp;
    m_shakeAmplitude += 5_len;
  }
  else if(m_shakeAmplitude > 0_len)
  {
    m_location.position.Y += m_shakeAmplitude;
    m_lookAt.position.Y += m_shakeAmplitude;
    m_shakeAmplitude = 0_len;
  }

  if(m_location.position.Y > floor)
    m_location.position.Y = floor;
  else if(m_location.position.Y < ceiling)
    m_location.position.Y = ceiling;

  m_location.updateRoom();
}

void CameraController::chaseObject(const objects::Object& object)
{
  m_rotationAroundLara.X += object.m_state.rotation.X;
  if(m_rotationAroundLara.X > 85_deg)
    m_rotationAroundLara.X = 85_deg;
  else if(m_rotationAroundLara.X < -85_deg)
    m_rotationAroundLara.X = -85_deg;

  const auto dist = util::cos(m_distance, m_rotationAroundLara.X);
  const auto goal = clampBox(
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

  updatePosition(goal, m_wasFixedMode ? m_smoothingFactor : 12_frame);
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

  if(isCollidingWithFloorOrCeiling(m_lookAt, m_world->getObjectManager()))
  {
    m_lookAt.position.X = lara.m_state.location.position.X;
    m_lookAt.position.Z = lara.m_state.location.position.Z;
  }
  m_lookAt.position.Y += clampToVerticalBounds(m_lookAt, core::CameraWallDistance);

  const auto goal = clampBox(m_lookAt,
                             m_lookAt.position
                               - util::pitch(util::cos(m_distance, m_rotationAroundLara.X),
                                             m_rotationAroundLara.Y,
                                             -util::sin(m_distance, m_rotationAroundLara.X)),
                             &freeLookClamp,
                             m_world->getObjectManager());

  m_lookAt.position.X = originalLookAt.X + (m_lookAt.position.X - originalLookAt.X) / m_smoothingFactor * 1_frame;
  m_lookAt.position.Z = originalLookAt.Z + (m_lookAt.position.Z - originalLookAt.Z) / m_smoothingFactor * 1_frame;

  updatePosition(goal, m_smoothingFactor);
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
  const auto eye = clampBox(
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
  updatePosition(eye, m_smoothingFactor);
}

bool CameraController::tickCinematic(const std::vector<world::CinematicFrame>& frames, const bool inGameCamera)
{
  if(m_cinematicFrame.get<size_t>() >= frames.size() - 1)
    return false;

  m_cinematicFrame += 1_frame;

  const auto& frame = frames[m_cinematicFrame.get<size_t>()];

  m_previousLookAt = m_lookAt;
  if(inGameCamera)
    m_lookAt.position = m_cinematicPos + util::pitch(frame.lookAt, m_cinematicBaseRotation.Y + m_cinematicRot.Y);
  else
    m_lookAt.position = m_cinematicPos + util::pitch(frame.lookAt, m_cinematicBaseRotation.Y);
  m_lookAt.updateRoom();

  m_previousLocation = m_location;
  if(inGameCamera)
    m_location.position = m_cinematicPos + util::pitch(frame.position, m_cinematicBaseRotation.Y + m_cinematicRot.Y);
  else
    m_location.position = m_cinematicPos + util::pitch(frame.position, m_cinematicBaseRotation.Y);
  m_location.updateRoom();

  m_previousCinematicRoll = std::exchange(m_cinematicRoll, frame.rotZ);
  m_previousCinematicFov = std::exchange(m_cinematicFov, frame.fov);

  if(m_cinematicFrame == 1_frame)
  {
    m_previousLookAt = m_lookAt;
    m_previousLocation = m_location;
    m_previousCinematicRoll = m_cinematicRoll;
    m_previousCinematicFov = m_cinematicFov;
  }

  return true;
}

std::unordered_set<const world::Portal*> CameraController::getWaterEntryPortals() const
{
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

CameraController::CameraController(const gsl_lite::not_null<world::World*>& world,
                                   gslu::nn_shared<render::scene::Camera> camera,
                                   bool /*noLaraTag*/)
    : Listener{gsl_lite::not_null{world->getEngine().getPresenter().getSoundEngine().get().get()}}
    , m_camera{std::move(camera)}
    , m_world{world}
    , m_location{gsl_lite::not_null{world->getRooms().data()}}
    , m_previousLocation{gsl_lite::not_null{world->getRooms().data()}}
    , m_lookAt{gsl_lite::not_null{world->getRooms().data()}}
    , m_previousLookAt{gsl_lite::not_null{world->getRooms().data()}}
{
}

void CameraController::serialize(const serialization::Serializer<world::World>& ser) const
{
  ser(S_NV("location", m_location),
      S_NV("lookAt", m_lookAt),
      S_NV("mode", m_mode),
      S_NV("modifier", m_modifier),
      S_NV("wasFixedMode", m_wasFixedMode),
      S_NV("lookAtObject", serialization::ObjectReference{std::cref(m_lookAtObject)}),
      S_NV("previousLookAtObject", serialization::ObjectReference{std::cref(m_previousLookAtObject)}),
      S_NV("shakeAmplitude", m_shakeAmplitude),
      S_NV("distance", m_distance),
      S_NV("cinematicBaseRotation", m_cinematicBaseRotation),
      S_NV("rotationAroundLara", m_rotationAroundLara),
      S_NV("smoothingFactor", m_smoothingFactor),
      S_NV("fixedCameraId", m_fixedCameraId),
      S_NV("currentFixedCameraId", m_currentFixedCameraId),
      S_NV("overrideTimeout", m_overrideTimeout),
      S_NV("cinematicFrame", m_cinematicFrame),
      S_NV("cinematicPos", m_cinematicPos),
      S_NV("cinematicRot", m_cinematicRot));
}

void CameraController::deserialize(const serialization::Deserializer<world::World>& ser)
{
  // TODO CE-378 remove member name migration
  ser(S_NV("location", m_location),
      S_NV("lookAt", m_lookAt),
      S_NV("mode", m_mode),
      S_NV("modifier", m_modifier),
      S_ANV("wasFixedMode", "completelyFixed", m_wasFixedMode),
      S_NV("lookAtObject", serialization::ObjectReference{std::ref(m_lookAtObject)}),
      S_NV("previousLookAtObject", serialization::ObjectReference{std::ref(m_previousLookAtObject)}),
      S_ANV("shakeAmplitude", "bounce", m_shakeAmplitude),
      S_NV("distance", m_distance),
      S_ANV("cinematicBaseRotation", "eyeRotation", m_cinematicBaseRotation),
      S_NV("rotationAroundLara", m_rotationAroundLara),
      S_ANV("smoothingFactor", "smoothness", m_smoothingFactor),
      S_NV("fixedCameraId", m_fixedCameraId),
      S_NV("currentFixedCameraId", m_currentFixedCameraId),
      S_ANV("overrideTimeout", "camOverrideTimeout", m_overrideTimeout),
      S_NV("cinematicFrame", m_cinematicFrame),
      S_NV("cinematicPos", m_cinematicPos),
      S_NV("cinematicRot", m_cinematicRot));
}

void CameraController::startCinematic(const core::TRVec& pos, const core::TRRotation& rot)
{
  m_mode = CameraMode::Cinematic;
  m_cinematicFrame = 0_frame;
  m_cinematicPos = pos;
  m_cinematicRot = rot;
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

void CameraController::interpolateCameraTransform(const float interTickFactor)
{
  std::optional<core::Radians> roll{};
  core::Radians fov{};

  if(m_mode == CameraMode::Cinematic)
  {
    roll = lerp(m_previousCinematicRoll, m_cinematicRoll, interTickFactor);
    fov = lerp(m_previousCinematicFov, m_cinematicFov, interTickFactor);
  }
  else
  {
    fov = m_camera->getFieldOfViewY();
  }

  fixCameraJumpInterpolation();

  auto m = lookAt(
    glm::mix(m_previousLocation.position.toRenderSystem(), m_location.position.toRenderSystem(), interTickFactor),
    glm::mix(m_previousLookAt.position.toRenderSystem(), m_lookAt.position.toRenderSystem(), interTickFactor),
    core::RenderAxisUp);
  if(roll.has_value())
  {
    m = rotate(m, roll.value().get<float>(), -glm::vec3{m[2]});
  }
  m_camera->setViewMatrix(m);
  m_camera->setFieldOfView(fov);
}

void CameraController::fixCameraJumpInterpolation()
{
  static constexpr auto JumpThreshold = 256_len;
  if(length(m_location.position - m_previousLocation.position) >= JumpThreshold
     || length(m_lookAt.position - m_previousLookAt.position) >= JumpThreshold)
  {
    m_previousLocation.position = m_location.position;
    m_previousLookAt.position = m_lookAt.position;
  }
}
} // namespace engine