#pragma once

#include "core/units.h"
#include "loader/file/animationid.h"
#include "loader/file/larastateid.h"
#include "qs/quantity.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>

namespace engine
{
enum class CameraMode : uint8_t;
enum class CameraModifier : uint8_t;
using LaraStateId = loader::file::LaraStateId;
using AnimationId = loader::file::AnimationId;
struct CollisionInfo;
} // namespace engine

namespace engine::objects
{
class LaraObject;
enum class HandStatus : uint8_t;
enum class UnderwaterState : uint8_t;
} // namespace engine::objects

namespace engine::world
{
class World;
}

namespace engine::lara
{
class AbstractStateHandler
{
public:
  explicit AbstractStateHandler(const gsl::not_null<objects::LaraObject*>& lara, const LaraStateId id)
      : m_lara{lara}
      , m_id{id}
  {
  }

  virtual ~AbstractStateHandler() = default;

  AbstractStateHandler(const AbstractStateHandler&) = delete;
  AbstractStateHandler(AbstractStateHandler&&) = delete;
  AbstractStateHandler& operator=(const AbstractStateHandler&) = delete;
  AbstractStateHandler& operator=(AbstractStateHandler&&) = delete;

  virtual void postprocessFrame(CollisionInfo& collisionInfo) = 0;

  virtual void handleInput(CollisionInfo& collisionInfo) = 0;

  static std::unique_ptr<AbstractStateHandler> create(LaraStateId id, const gsl::not_null<objects::LaraObject*>& lara);

  [[nodiscard]] LaraStateId getId() const noexcept
  {
    return m_id;
  }

private:
  gsl::not_null<objects::LaraObject*> m_lara;
  LaraStateId m_id;

  friend class StateHandler_2;

protected:
  [[nodiscard]] objects::LaraObject& getLara()
  {
    return *m_lara;
  }

  [[nodiscard]] const objects::LaraObject& getLara() const
  {
    return *m_lara;
  }

  void setAir(const core::Frame& a) noexcept;

  void setMovementAngle(const core::Angle& angle) noexcept;

  [[nodiscard]] core::Angle getMovementAngle() const noexcept;

  [[nodiscard]] objects::HandStatus getHandStatus() const noexcept;

  void setHandStatus(objects::HandStatus status) noexcept;

  [[nodiscard]] LaraStateId getCurrentAnimState() const;

  void setAnimation(AnimationId anim, const std::optional<core::Frame>& firstFrame = std::nullopt);

  [[nodiscard]] const world::World& getWorld() const;

  [[nodiscard]] world::World& getWorld();

  void placeOnFloor(const CollisionInfo& collisionInfo);

  void setYRotationSpeed(const core::RotationSpeed& spd);

  [[nodiscard]] core::RotationSpeed getYRotationSpeed() const;

  void subYRotationSpeed(const core::RotationAcceleration& val, const core::RotationSpeed& limit = -32768_au / 1_frame);

  void addYRotationSpeed(const core::RotationAcceleration& val, const core::RotationSpeed& limit = 32767_au / 1_frame);

  void setFallSpeedOverride(const core::Speed& v);

  void dampenHorizontalSpeed(float f);

  [[nodiscard]] core::Angle getCurrentSlideAngle() const noexcept;

  void setCurrentSlideAngle(const core::Angle& a) noexcept;

  void setGoalAnimState(LaraStateId state);
  void setCurrentAnimState(LaraStateId state);

  [[nodiscard]] LaraStateId getGoalAnimState() const;

  bool stopIfCeilingBlocked(const CollisionInfo& collisionInfo);

  bool tryClimb(const CollisionInfo& collisionInfo);

  bool checkWallCollision(const CollisionInfo& collisionInfo);

  bool tryStartSlide(const CollisionInfo& collisionInfo);

  void jumpAgainstWall(CollisionInfo& collisionInfo);

  void checkJumpWallSmash(CollisionInfo& collisionInfo);

  void applyShift(const CollisionInfo& collisionInfo);

  void commonJumpHandling(CollisionInfo& collisionInfo);

  void commonSlideHandling(CollisionInfo& collisionInfo);

  void commonEdgeHangHandling(CollisionInfo& collisionInfo);

  bool applyLandingDamage();

  void addSwimToDiveKeypressDuration(const core::Frame& n) noexcept;

  void setSwimToDiveKeypressDuration(const core::Frame& n) noexcept;

  [[nodiscard]] core::Frame getSwimToDiveKeypressDuration() const;

  void setUnderwaterState(objects::UnderwaterState u) noexcept;

  void laraAdvanceFrame();
};
} // namespace engine::lara
