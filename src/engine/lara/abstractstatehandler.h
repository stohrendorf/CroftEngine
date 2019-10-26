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

class LaraNode;

struct InputState;
enum class UnderwaterState;
enum class HandStatus;

namespace lara
{
class AbstractStateHandler
{
  public:
  explicit AbstractStateHandler(LaraNode& lara, const LaraStateId id)
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

  static std::unique_ptr<AbstractStateHandler> create(LaraStateId id, LaraNode& lara);

  LaraStateId getId() const noexcept
  {
    return m_id;
  }

  private:
  LaraNode& m_lara;
  const LaraStateId m_id;

  friend class StateHandler_2;

  protected:
  // ReSharper disable once CppMemberFunctionMayBeConst
  LaraNode& getLara()
  {
    return m_lara;
  }

  void setAir(core::Frame a) noexcept;

  void setMovementAngle(core::Angle angle) noexcept;

  core::Angle getMovementAngle() const noexcept;

  HandStatus getHandStatus() const noexcept;

  void setHandStatus(HandStatus status) noexcept;

  LaraStateId getCurrentAnimState() const;

  void setAnimation(AnimationId anim, const boost::optional<core::Frame>& firstFrame = boost::none);

  const engine::Engine& getEngine() const;

  engine::Engine& getEngine();

  void placeOnFloor(const CollisionInfo& collisionInfo);

  void setYRotationSpeed(core::Angle spd);

  core::Angle getYRotationSpeed() const;

  void subYRotationSpeed(core::Angle val, core::Angle limit = -32768_au);

  void addYRotationSpeed(core::Angle val, core::Angle limit = 32767_au);

  void setFallSpeedOverride(core::Speed v);

  void dampenHorizontalSpeed(float f);

  core::Angle getCurrentSlideAngle() const noexcept;

  void setCurrentSlideAngle(core::Angle a) noexcept;

  void setGoalAnimState(LaraStateId state);

  LaraStateId getGoalAnimState() const;

  bool stopIfCeilingBlocked(const CollisionInfo& collisionInfo);

  bool tryClimb(CollisionInfo& collisionInfo);

  bool checkWallCollision(CollisionInfo& collisionInfo);

  bool tryStartSlide(const CollisionInfo& collisionInfo);

  bool tryGrabEdge(CollisionInfo& collisionInfo);

  void jumpAgainstWall(CollisionInfo& collisionInfo);

  void checkJumpWallSmash(CollisionInfo& collisionInfo);

  void applyShift(const CollisionInfo& collisionInfo);

  core::Length getRelativeHeightAtDirection(core::Angle angle, core::Length dist) const;

  void commonJumpHandling(CollisionInfo& collisionInfo);

  void commonSlideHandling(CollisionInfo& collisionInfo);

  void commonEdgeHangHandling(CollisionInfo& collisionInfo);

  bool tryReach(CollisionInfo& collisionInfo);

  bool canClimbOnto(core::Axis axis) const;

  bool applyLandingDamage();

  loader::file::BoundingBox getBoundingBox() const;

  void addSwimToDiveKeypressDuration(core::Frame n) noexcept;

  void setSwimToDiveKeypressDuration(core::Frame n) noexcept;

  core::Frame getSwimToDiveKeypressDuration() const;

  void setUnderwaterState(UnderwaterState u) noexcept;

  void setCameraRotationAroundCenter(const core::Angle x, const core::Angle y);

  void setCameraRotationAroundCenterX(const core::Angle x);

  void setCameraRotationAroundCenterY(const core::Angle y);

  void setCameraEyeCenterDistance(core::Length d);

  void setCameraModifier(const CameraModifier k);

  void laraUpdateImpl();
};
} // namespace lara
} // namespace engine
