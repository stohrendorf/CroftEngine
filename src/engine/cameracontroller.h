#pragma once

#include "audio/listener.h"
#include "core/angle.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "floordata/types.h"
#include "location.h"
#include "qs/quantity.h"
#include "serialization/serialization_fwd.h"

#include <cstdint>
#include <glm/fwd.hpp>
#include <gsl-lite/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <unordered_set>

namespace render::scene
{
class Camera;
}

namespace engine::world
{
class World;
struct Portal;
struct CinematicFrame;
} // namespace engine::world

namespace engine
{
namespace floordata
{
enum class SequenceCondition : uint8_t;
struct CameraParameters;
} // namespace floordata

namespace objects
{
class Object;
}

enum class CameraMode : uint8_t
{
  /// Camera follows Lara.
  Chase,
  /// Camera is fixed at a position.
  FixedPosition,
  /// User controls the camera while it moves around Lara.
  FreeLook,
  /// Like Chase, but at a farther distance, and looking at the focused enemy
  Combat,
  /// Camera is fully driven by a fixed position/angle sequence, relative to Lara
  Cinematic,
  /// Like FixedPosition, but disables camera command sequence handling
  HeavyFixedPosition
};

enum class CameraModifier : uint8_t
{
  None,
  FollowCenter,
  AllowSteepSlants,
  Chase
};

class CameraController final : public audio::Listener
{
  gslu::nn_shared<render::scene::Camera> m_camera;

  gsl_lite::not_null<world::World*> m_world;

  //! @brief Global camera position.
  Location m_location;
  Location m_previousLocation;
  //! @brief The point the camera moves around.
  Location m_lookAt;
  Location m_previousLookAt;
  CameraMode m_mode = CameraMode::Chase;

  CameraModifier m_modifier = CameraModifier::None;

  //! @brief Tracks if the previous logic tick used a fixed camera mode.
  //! @note Used to determine if we should snap or smooth the camera when transitioning between modes.
  bool m_wasFixedMode = false;

  /**
* @brief If <0, shake randomly around +/- @c m_shakeAmplitude/2, increasing value by 5 each frame; if >0, do a single Y shake downwards by @c m_shakeAmplitude.
*/
  core::Length m_shakeAmplitude = 0_len;

  //! @brief Goal distance between the pivot point and the camera.
  core::Length m_distance = core::DefaultCameraLaraDistance;

  //! @brief Base rotation for cinematic frames (captured from Lara at start of cinematic).
  core::TRRotation m_cinematicBaseRotation;

  //! @brief Rotation of the camera around the look-at point.
  //! @note In Chase mode, this is an offset from the object's rotation.
  //!       In FreeLook/Combat mode, this is an absolute rotation.
  core::TRRotation m_rotationAroundLara;

  //! @brief An object to point the camera to.
  //! @note Also modifies Lara's head and torso rotation.
  std::shared_ptr<objects::Object> m_lookAtObject = nullptr;
  std::shared_ptr<objects::Object> m_previousLookAtObject = nullptr;
  //! @brief Movement smoothness for adjusting the pivot position.
  core::Frame m_smoothingFactor = 8_frame;
  int m_fixedCameraId = -1;
  int m_currentFixedCameraId = -1;
  core::Frame m_overrideTimeout{-1_frame};

  core::Frame m_cinematicFrame = 0_frame;
  core::TRVec m_cinematicPos{0_len, 0_len, 0_len};
  core::TRRotation m_cinematicRot;
  core::Radians m_previousCinematicRoll{0.0f};
  core::Radians m_cinematicRoll{0.0f};
  core::Radians m_previousCinematicFov{core::DefaultFov};
  core::Radians m_cinematicFov{core::DefaultFov};

  void handleFixedCamera();

  core::Length clampToVerticalBounds(Location& goal, const core::Length& margin) const;

  void updatePosition(const Location& goal, const core::Frame& smoothingFactor);

  void chaseObject(const objects::Object& object);

  void handleFreeLook();
  void handleEnemy();
 void fixCameraJumpInterpolation();

public:
  explicit CameraController(const gsl_lite::not_null<world::World*>& world,
                            gslu::nn_shared<render::scene::Camera> camera);

  explicit CameraController(const gsl_lite::not_null<world::World*>& world,
                            gslu::nn_shared<render::scene::Camera> camera,
                            bool noLaraTag);

  const gsl_lite::not_null<world::World*>& getWorld() const noexcept
  {
    return m_world;
  }

  void setRotationAroundLara(const core::Angle& x, const core::Angle& y) noexcept;

  void setRotationAroundLaraX(const core::Angle& x) noexcept;

  void setRotationAroundLaraY(const core::Angle& y) noexcept;

  void setCinematicBaseRotation(const core::Angle& x, const core::Angle& y) noexcept
  {
    m_cinematicBaseRotation.X = x;
    m_cinematicBaseRotation.Y = y;
  }

  const core::TRRotation& getCinematicBaseRotation() const noexcept
  {
    return m_cinematicBaseRotation;
  }

  void setDistance(const core::Length& d) noexcept
  {
    m_distance = d;
  }

  void setModifier(const CameraModifier k) noexcept
  {
    m_modifier = k;
  }

  void setCamOverride(const floordata::CameraParameters& camParams,
                      uint16_t camId,
                      floordata::SequenceCondition condition,
                      bool fromHeavy,
                      const core::Frame& timeout,
                      bool switchIsOn);

  void setLookAtObject(const gslu::nn_shared<objects::Object>& object)
  {
    m_lookAtObject = object;
  }

  void handleCommandSequence(const floordata::FloorDataValue* cmdSequence);

  void updateGameLogic(bool inGameCamera);

  void setMode(const CameraMode mode) noexcept
  {
    m_mode = mode;
  }

  CameraMode getMode() const noexcept
  {
    return m_mode;
  }

  [[nodiscard]] glm::vec3 getPosition() const override;

  const Location& getLookAt() const noexcept
  {
    return m_lookAt;
  }

  [[nodiscard]] glm::vec3 getFrontVector() const override;

  [[nodiscard]] glm::vec3 getUpVector() const override;

  const auto& getCurrentRoom() const noexcept
  {
    return m_location.room;
  }

  void setPosition(const core::TRVec& p) noexcept
  {
    m_location.position = p;
  }

  const Location& getTRLocation() const noexcept
  {
    return m_location;
  }

  const core::TRVec& getCinematicPos() const noexcept
  {
    return m_cinematicPos;
  }

  void setLocation(const Location& location)
  {
    m_location = location;
  }

  void setShake(const core::Length& amplitude) noexcept
  {
    m_shakeAmplitude = amplitude;
  }

  const auto& getCamera() const noexcept
  {
    return m_camera;
  }

  bool tickCinematic(const std::vector<world::CinematicFrame>& frames, bool inGameCamera);
  std::unordered_set<const world::Portal*> getWaterEntryPortals() const;
  std::unordered_set<const world::Portal*> traceWaterSurfacePortals() const;

  void interpolateCameraTransform(float interTickFactor);

  void serialize(const serialization::Serializer<world::World>& ser) const;
  void deserialize(const serialization::Deserializer<world::World>& ser);

  void startCinematic(const core::TRVec& pos, const core::TRRotation& rot);
};
} // namespace engine