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
#include <gsl/gsl-lite.hpp>
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
enum class SequenceCondition;
struct CameraParameters;
} // namespace floordata

namespace objects
{
class Object;
}

enum class CameraMode
{
  Chase,
  FixedPosition,
  FreeLook,
  Combat,
  Cinematic,
  HeavyFixedPosition //!< like FixedPosition, but disables camera command sequence handling
};

enum class CameraModifier
{
  None,
  FollowCenter,
  AllowSteepSlants,
  Chase
};

class CameraController final : public audio::Listener
{
private:
  gslu::nn_shared<render::scene::Camera> m_camera;

  const gsl::not_null<world::World*> m_world;

  //! @brief Global camera position.
  Location m_location;
  //! @brief The point the camera moves around.
  Location m_lookAt;
  CameraMode m_mode = CameraMode::Chase;

  CameraModifier m_modifier = CameraModifier::None;

  bool m_isCompletelyFixed = false;

  /**
     * @brief If <0, bounce randomly around +/- @c m_bounce/2, increasing value by 5 each frame; if >0, do a single Y bounce downwards by @c m_bounce.
     */
  core::Length m_bounce = 0_len;

  //! @brief Goal distance between the pivot point and the camera.
  core::Length m_distance = core::DefaultCameraLaraDistance;

  core::TRRotation m_eyeRotation;

  //! @brief Global camera rotation.
  core::TRRotation m_rotationAroundLara;

  //! @brief An object to point the camera to.
  //! @note Also modifies Lara's head and torso rotation.
  std::shared_ptr<objects::Object> m_lookAtObject = nullptr;
  std::shared_ptr<objects::Object> m_previousLookAtObject = nullptr;
  //! @brief Movement smoothness for adjusting the pivot position.
  core::Frame m_smoothness = 8_frame;
  int m_fixedCameraId = -1;
  int m_currentFixedCameraId = -1;
  core::Frame m_camOverrideTimeout{-1_frame};

public:
  explicit CameraController(const gsl::not_null<world::World*>& world, gslu::nn_shared<render::scene::Camera> camera);

  explicit CameraController(const gsl::not_null<world::World*>& world,
                            gslu::nn_shared<render::scene::Camera> camera,
                            bool noLaraTag);

  const gsl::not_null<world::World*>& getWorld() const noexcept
  {
    return m_world;
  }

  void setRotationAroundLara(const core::Angle& x, const core::Angle& y);

  void setRotationAroundLaraX(const core::Angle& x);

  void setRotationAroundLaraY(const core::Angle& y);

  void setEyeRotation(const core::Angle& x, const core::Angle& y)
  {
    m_eyeRotation.X = x;
    m_eyeRotation.Y = y;
  }

  const core::TRRotation& getEyeRotation() const
  {
    return m_eyeRotation;
  }

  void setDistance(const core::Length& d)
  {
    m_distance = d;
  }

  void setModifier(const CameraModifier k)
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

  std::unordered_set<const world::Portal*> update();

  void setMode(const CameraMode t)
  {
    m_mode = t;
  }

  CameraMode getMode() const noexcept
  {
    return m_mode;
  }

  [[nodiscard]] glm::vec3 getPosition() const override;

  const Location& getLookAt() const
  {
    return m_lookAt;
  }

  [[nodiscard]] glm::vec3 getFrontVector() const override;

  [[nodiscard]] glm::vec3 getUpVector() const override;

  const auto& getCurrentRoom() const
  {
    return m_location.room;
  }

  void setPosition(const core::TRVec& p)
  {
    m_location.position = p;
  }

  const Location& getTRLocation() const
  {
    return m_location;
  }

  void setLocation(const Location& location)
  {
    m_location = location;
  }

  void setBounce(const core::Length& bounce)
  {
    m_bounce = bounce;
  }

  const auto& getCamera() const
  {
    return m_camera;
  }

  std::unordered_set<const world::Portal*> updateCinematic(const world::CinematicFrame& frame, bool ingame);

  void serialize(const serialization::Serializer<world::World>& ser);

  core::Frame m_cinematicFrame = 0_frame;
  core::TRVec m_cinematicPos{0_len, 0_len, 0_len};
  core::TRRotation m_cinematicRot{};

private:
  std::unordered_set<const world::Portal*> tracePortals();

  void handleFixedCamera();

  core::Length moveIntoBox(Location& goal, const core::Length& margin) const;

  void updatePosition(const Location& goal, const core::Frame& smoothFactor);

  void chaseObject(const objects::Object& object);

  void handleFreeLook();
  void handleEnemy();
};
} // namespace engine
