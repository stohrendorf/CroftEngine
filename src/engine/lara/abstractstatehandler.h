#pragma once

#include "engine/skeletalmodelnode.h"
#include "loader/larastateid.h"
#include "loader/datatypes.h"
#include "loader/animationid.h"
#include "core/angle.h"

#include <memory>

namespace engine
{
enum class CameraMode;
enum class CameraModifier;
using LaraStateId = loader::LaraStateId;
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

    void setAir(int a) noexcept;

    void setMovementAngle(core::Angle angle) noexcept;

    core::Angle getMovementAngle() const noexcept;

    HandStatus getHandStatus() const noexcept;

    void setHandStatus(HandStatus status) noexcept;

    LaraStateId getCurrentAnimState() const;

    void setAnimation(loader::AnimationId anim, const boost::optional<uint16_t>& firstFrame = boost::none);

    const level::Level& getLevel() const;

    level::Level& getLevel();

    void placeOnFloor(const CollisionInfo& collisionInfo);

    void setYRotationSpeed(core::Angle spd);

    core::Angle getYRotationSpeed() const;

    void subYRotationSpeed(core::Angle val,
                           core::Angle limit = -32768_au);

    void addYRotationSpeed(core::Angle val,
                           core::Angle limit = 32767_au);

    void setFallSpeedOverride(int v);

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

    int getRelativeHeightAtDirection(core::Angle angle, int dist) const;

    void commonJumpHandling(CollisionInfo& collisionInfo);

    void commonSlideHandling(CollisionInfo& collisionInfo);

    void commonEdgeHangHandling(CollisionInfo& collisionInfo);

    bool tryReach(CollisionInfo& collisionInfo);

    bool canClimbOnto(core::Axis axis) const;

    bool applyLandingDamage();

    loader::BoundingBox getBoundingBox() const;

    void addSwimToDiveKeypressDuration(int n) noexcept;

    void setSwimToDiveKeypressDuration(int n) noexcept;

    int getSwimToDiveKeypressDuration() const;

    void setUnderwaterState(UnderwaterState u) noexcept;

    void setCameraRotationAroundCenter(const core::Angle x, const core::Angle y);

    void setCameraRotationAroundCenterX(const core::Angle x);

    void setCameraRotationAroundCenterY(const core::Angle y);

    void setCameraEyeCenterDistance(int d);

    void setCameraModifier(const CameraModifier k);

    void laraUpdateImpl();
};
}
}
