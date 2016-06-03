#pragma once

#include "loader/larastateid.h"
#include "loader/datatypes.h"
#include "loader/animationids.h"

#include <memory>

struct LaraState;
class LaraStateHandler;
struct InputState;

class AbstractStateHandler
{
    LaraStateHandler& m_stateHandler;

public:
    explicit AbstractStateHandler(LaraStateHandler& lara)
        : m_stateHandler(lara)
    {
    }

    virtual ~AbstractStateHandler() = default;

    virtual std::unique_ptr<AbstractStateHandler> handleInput(LaraState& state) = 0;
    virtual std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) = 0;

    void animate(LaraState& state, int deltaTimeMs);

    static std::unique_ptr<AbstractStateHandler> create(loader::LaraStateId id, LaraStateHandler& lara);

    virtual loader::LaraStateId getId() const noexcept = 0;

private:
    virtual void animateImpl(LaraState& state, int deltaTimeMs) = 0;

protected:
    SpeedValue<int16_t> m_yRotationSpeed = 0;
    SpeedValue<int> m_xMovement = 0;
    SpeedValue<int> m_yMovement = 0;
    SpeedValue<int> m_zMovement = 0;

    LaraStateHandler& getStateHandler()
    {
        return m_stateHandler;
    }

    int getHealth() const noexcept;
    
    void setHealth(int h) noexcept;

    const InputState& getInputState() const noexcept;

    void setMovementAngle(int16_t angle) noexcept;

    int16_t getMovementAngle() const noexcept;

    void setFallSpeed(int spd);

    const SpeedValue<int>& getFallSpeed() const noexcept;

    bool isFalling() const noexcept;

    void setFalling(bool falling) noexcept;

    int getHandStatus() const noexcept;

    void setHandStatus(int status) noexcept;

    uint32_t getCurrentFrame() const;
    
    loader::LaraStateId getCurrentAnimState() const;

    void playAnimation(loader::AnimationId anim, const boost::optional<irr::u32>& firstFrame = boost::none);

    const irr::core::vector3df& getRotation() const noexcept;

    void setHorizontalSpeed(int speed);

    const loader::Level& getLevel() const;

    void placeOnFloor(const LaraState& state);

    loader::TRCoordinates getPosition() const;

    void setPosition(const loader::ExactTRCoordinates& pos);

    int getFloorHeight() const noexcept;

    void setFloorHeight(int h) noexcept;

    void setYRotationSpeed(int spd);

    int getYRotationSpeed() const;

    void subYRotationSpeed(int val, int limit = std::numeric_limits<int>::min());

    void addYRotationSpeed(int val, int limit = std::numeric_limits<int>::max());

    void setYRotation(int16_t y);

    void setZRotation(int16_t z);

    void setZRotationExact(float z);

    void setFallSpeedOverride(int v);

    void dampenHorizontalSpeed(int nom, int den);

    int16_t getCurrentSlideAngle() const noexcept;

    void setCurrentSlideAngle(int16_t a) noexcept;

    void setTargetState(loader::LaraStateId state);
    loader::LaraStateId getTargetState() const;

    std::unique_ptr<AbstractStateHandler> tryStopOnFloor(LaraState& state);
    std::unique_ptr<AbstractStateHandler> tryClimb(LaraState& state);
    std::unique_ptr<AbstractStateHandler> checkWallCollision(LaraState& state);
    bool tryStartSlide(LaraState& state, std::unique_ptr<AbstractStateHandler>& nextHandler);
    std::unique_ptr<AbstractStateHandler> tryGrabEdge(LaraState& state);
    void jumpAgainstWall(LaraState& state);
    std::unique_ptr<AbstractStateHandler> checkJumpWallSmash(LaraState& state);

    void applyCollisionFeedback(LaraState& state);
    int getRelativeHeightAtDirection(int16_t angle, int dist) const;
    std::unique_ptr<AbstractStateHandler> commonJumpHandling(LaraState& state);
    std::unique_ptr<AbstractStateHandler> commonSlideHandling(LaraState& state);
    std::unique_ptr<AbstractStateHandler> tryReach(LaraState& state);
    bool canClimbOnto(int16_t angle) const;

    bool applyLandingDamage();

    irr::scene::ISceneNode* getLara();
};
