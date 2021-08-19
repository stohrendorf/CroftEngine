#pragma once

#include "core/angle.h"
#include "engine/skeletalmodelnode.h"
#include "loader/file/animationid.h"
#include "loader/file/datatypes.h"
#include "loader/file/larastateid.h"

#include <memory>

namespace engine
{
enum class CameraMode;
enum class CameraModifier;
using LaraStateId = loader::file::LaraStateId;
using AnimationId = loader::file::AnimationId;
struct CollisionInfo;

namespace objects
{
class LaraObject;
enum class HandStatus;
enum class UnderwaterState;
} // namespace objects

namespace lara
{
class AbstractStateHandler
{
public:
  explicit AbstractStateHandler(objects::LaraObject& lara, const LaraStateId id)
      : m_lara{lara}
      , m_id{id}
  {
  }

  AbstractStateHandler(const AbstractStateHandler&) = delete;

  AbstractStateHandler(AbstractStateHandler&&) = delete;

  AbstractStateHandler& operator=(const AbstractStateHandler&) = delete;

  AbstractStateHandler& operator=(AbstractStateHandler&&) = delete;

  virtual ~AbstractStateHandler() = default;

  virtual void postprocessFrame(CollisionInfo& collisionInfo) = 0;

  virtual void handleInput(CollisionInfo& collisionInfo) = 0;

  static std::unique_ptr<AbstractStateHandler> create(LaraStateId id, objects::LaraObject& lara);

  [[nodiscard]] LaraStateId getId() const noexcept
  {
    return m_id;
  }

private:
  objects::LaraObject& m_lara;
  const LaraStateId m_id;

  friend class StateHandler_2;

protected:
  // ReSharper disable once CppMemberFunctionMayBeConst
  objects::LaraObject& getLara()
  {
    return m_lara;
  }

  void setAir(const core::Frame& a) noexcept;

  void setMovementAngle(const core::Angle& angle) noexcept;

  [[nodiscard]] core::Angle getMovementAngle() const noexcept;

  [[nodiscard]] objects::HandStatus getHandStatus() const noexcept;

  void setHandStatus(objects::HandStatus status) noexcept;

  [[nodiscard]] LaraStateId getCurrentAnimState() const;

  void setAnimation(AnimationId anim, const std::optional<core::Frame>& firstFrame = std::nullopt);

  [[nodiscard]] const world::World& getWorld() const;

  world::World& getWorld();

  void placeOnFloor(const CollisionInfo& collisionInfo);

  void setYRotationSpeed(const core::Angle& spd);

  [[nodiscard]] core::Angle getYRotationSpeed() const;

  void subYRotationSpeed(const core::Angle& val, const core::Angle& limit = -32768_au);

  void addYRotationSpeed(const core::Angle& val, const core::Angle& limit = 32767_au);

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

  bool tryGrabEdge(const CollisionInfo& collisionInfo);

  void jumpAgainstWall(CollisionInfo& collisionInfo);

  void checkJumpWallSmash(CollisionInfo& collisionInfo);

  void applyShift(const CollisionInfo& collisionInfo);

  [[nodiscard]] core::Length getRelativeHeightAtDirection(const core::Angle& angle, const core::Length& dist) const;

  void commonJumpHandling(CollisionInfo& collisionInfo);

  void commonSlideHandling(CollisionInfo& collisionInfo);

  void commonEdgeHangHandling(CollisionInfo& collisionInfo);

  bool tryReach(CollisionInfo& collisionInfo);

  [[nodiscard]] bool canClimbOnto(core::Axis axis) const;

  bool applyLandingDamage();

  [[nodiscard]] loader::file::BoundingBox getBoundingBox() const;

  void addSwimToDiveKeypressDuration(const core::Frame& n) noexcept;

  void setSwimToDiveKeypressDuration(const core::Frame& n) noexcept;

  [[nodiscard]] core::Frame getSwimToDiveKeypressDuration() const;

  void setUnderwaterState(objects::UnderwaterState u) noexcept;

  void setCameraRotationAroundLara(const core::Angle& x, const core::Angle& y);

  void setCameraRotationAroundLaraX(const core::Angle& x);

  void setCameraRotationAroundLaraY(const core::Angle& y);

  void setCameraDistance(const core::Length& d);

  void setCameraModifier(CameraModifier k);

  void laraUpdateImpl();
};
} // namespace lara
} // namespace engine
