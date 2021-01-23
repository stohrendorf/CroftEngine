#pragma once

#include "audio/soundengine.h"
#include "core/angle.h"
#include "loader/file/datatypes.h"
#include "render/scene/camera.h"

namespace engine
{
namespace floordata
{
enum class SequenceCondition;

class ActivationState;

struct CameraParameters;
} // namespace floordata

namespace objects
{
class Object;
class LaraObject;
} // namespace objects

class ObjectManager;
class World;

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
  gsl::not_null<std::shared_ptr<render::scene::Camera>> m_camera;

  const gsl::not_null<World*> m_world;

  //! @brief Global camera position.
  core::RoomBoundPosition m_position;
  //! @brief The point the camera moves around.
  core::RoomBoundPosition m_lookAt;
  CameraMode m_mode = CameraMode::Chase;

  //! @brief Additional height of the camera above the real position.
  core::Length m_positionYOffset = 0_len;

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
  std::shared_ptr<objects::Object> m_enemy = nullptr;
  //! @brief Movement smoothness for adjusting the pivot position.
  int m_smoothness = 8;
  int m_fixedCameraId = -1;
  int m_currentFixedCameraId = -1;
  core::Frame m_camOverrideTimeout{-1_frame};

public:
  explicit CameraController(const gsl::not_null<World*>& world,
                            gsl::not_null<std::shared_ptr<render::scene::Camera>> camera);

  explicit CameraController(const gsl::not_null<World*>& world,
                            gsl::not_null<std::shared_ptr<render::scene::Camera>> camera,
                            bool noLaraTag);

  const gsl::not_null<World*>& getWorld() const noexcept
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

  void setLookAtObject(const std::shared_ptr<objects::Object>& object)
  {
    if(object != nullptr && (m_mode == CameraMode::FixedPosition || m_mode == CameraMode::HeavyFixedPosition))
      m_lookAtObject = object;
  }

  void handleCommandSequence(const floordata::FloorDataValue* cmdSequence);

  std::unordered_set<const loader::file::Portal*> update();

  void setMode(const CameraMode t)
  {
    m_mode = t;
  }

  CameraMode getMode() const noexcept
  {
    return m_mode;
  }

  [[nodiscard]] glm::vec3 getPosition() const override
  {
    return m_camera->getPosition();
  }

  const core::RoomBoundPosition& getLookAt() const
  {
    return m_lookAt;
  }

  [[nodiscard]] glm::vec3 getFrontVector() const override
  {
    return m_camera->getFrontVector();
  }

  [[nodiscard]] glm::vec3 getUpVector() const override
  {
    return m_camera->getUpVector();
  }

  const auto& getCurrentRoom() const
  {
    return m_position.room;
  }

  void setPosition(const core::TRVec& p)
  {
    m_position.position = p;
  }

  const core::RoomBoundPosition& getTRPosition() const
  {
    return m_position;
  }

  void setPosition(const core::RoomBoundPosition& p)
  {
    m_position = p;
  }

  void setBounce(const core::Length& bounce)
  {
    m_bounce = bounce;
  }

  const auto& getCamera() const
  {
    return m_camera;
  }

  std::unordered_set<const loader::file::Portal*> updateCinematic(const loader::file::CinematicFrame& frame,
                                                                  bool ingame);

  void serialize(const serialization::Serializer& ser);

  size_t m_cinematicFrame = 0;
  core::TRVec m_cinematicPos{0_len, 0_len, 0_len};
  core::TRRotation m_cinematicRot{0_deg, 0_deg, 0_deg};

private:
  std::unordered_set<const loader::file::Portal*> tracePortals();

  void handleFixedCamera();

  core::Length moveIntoGeometry(core::RoomBoundPosition& goal, const core::Length& margin) const;

  void updatePosition(const core::RoomBoundPosition& positionGoal, int smoothFactor);

  void chaseObject(const objects::Object& object);

  void handleFreeLook(const objects::Object& object);

  void handleEnemy(objects::Object& object);
};

/**
   * @brief Clamps a point between two endpoints if there is a floordata-defined obstacle
   * @param[in] start Starting point
   * @param[in] goal Destination of the movement, clamped if necessary
   * @retval false if clamped
   *
   * @warning Please be aware that the return value is reverted and not what you might expect...
   */
extern bool raycastLineOfSight(const core::RoomBoundPosition& start,
                               core::RoomBoundPosition& goal,
                               const ObjectManager& objectManager);

} // namespace engine
