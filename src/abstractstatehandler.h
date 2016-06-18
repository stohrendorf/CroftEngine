#pragma once

#include "loader/larastateid.h"
#include "loader/datatypes.h"
#include "loader/animationids.h"
#include "core/angle.h"

#include <memory>
#include "core/interpolatedvalue.h"

struct LaraState;
class LaraController;
struct InputState;

enum class UnderwaterState;

class AbstractStateHandler
{
    LaraController& m_controller;

public:
    explicit AbstractStateHandler(LaraController& controller)
        : m_controller(controller)
    {
    }

    virtual ~AbstractStateHandler() = default;

    virtual std::unique_ptr<AbstractStateHandler> postprocessFrame(LaraState& state) = 0;

    void animate(LaraState& state, int deltaTimeMs);
    std::unique_ptr<AbstractStateHandler> handleInput(LaraState& state)
    {
        m_xMovement = 0;
        m_yMovement = 0;
        m_zMovement = 0;
        m_xRotationSpeed = 0_deg;
        m_yRotationSpeed = 0_deg;
        m_zRotationSpeed = 0_deg;
        return handleInputImpl(state);
    }

    static std::unique_ptr<AbstractStateHandler> create(loader::LaraStateId id, LaraController& controller);
    std::unique_ptr<AbstractStateHandler> createWithRetainedAnimation(loader::LaraStateId id) const;

    virtual loader::LaraStateId getId() const noexcept = 0;

private:
    friend class StateHandler_2;

    virtual void animateImpl(LaraState& state, int deltaTimeMs) = 0;
    virtual std::unique_ptr<AbstractStateHandler> handleInputImpl(LaraState& state) = 0;

protected:
    core::InterpolatedValue<core::Angle> m_xRotationSpeed{ 0_deg };
    core::InterpolatedValue<core::Angle> m_yRotationSpeed{ 0_deg };
    core::InterpolatedValue<core::Angle> m_zRotationSpeed{ 0_deg };
    core::InterpolatedValue<float> m_xMovement{ 0.0f };
    core::InterpolatedValue<float> m_yMovement{ 0.0f };
    core::InterpolatedValue<float> m_zMovement{ 0.0f };

    LaraController& getController()
    {
        return m_controller;
    }

    const core::InterpolatedValue<float>& getHealth() const noexcept;
    
    void setHealth(const core::InterpolatedValue<float>& h) noexcept;
    void setAir(const core::InterpolatedValue<float>& a) noexcept;

    const InputState& getInputState() const noexcept;

    void setMovementAngle(core::Angle angle) noexcept;

    core::Angle getMovementAngle() const noexcept;

    void setFallSpeed(const core::InterpolatedValue<float>& spd);

    const core::InterpolatedValue<float>& getFallSpeed() const noexcept;

    bool isFalling() const noexcept;

    void setFalling(bool falling) noexcept;

    int getHandStatus() const noexcept;

    void setHandStatus(int status) noexcept;

    uint32_t getCurrentFrame() const;
    
    loader::LaraStateId getCurrentAnimState() const;

    void playAnimation(loader::AnimationId anim, const boost::optional<irr::u32>& firstFrame = boost::none);

    const irr::core::vector3d<core::Angle>& getRotation() const noexcept;

    void setHorizontalSpeed(const core::InterpolatedValue<float>& speed);

    const core::InterpolatedValue<float>& getHorizontalSpeed() const;

    const loader::Level& getLevel() const;

    void placeOnFloor(const LaraState& state);

    loader::TRCoordinates getPosition() const;
    const loader::ExactTRCoordinates& getExactPosition() const;

    void setPosition(const loader::ExactTRCoordinates& pos);

    int getFloorHeight() const noexcept;

    void setFloorHeight(int h) noexcept;

    void setYRotationSpeed(core::Angle spd);

    core::Angle getYRotationSpeed() const;

    void subYRotationSpeed(core::Angle val, core::Angle limit = -32768_au);

    void addYRotationSpeed(core::Angle val, core::Angle limit = 32767_au);

    void setXRotation(core::Angle y);
    
    void setYRotation(core::Angle y);

    void setZRotation(core::Angle z);

    void setFallSpeedOverride(int v);

    void dampenHorizontalSpeed(float f);

    core::Angle getCurrentSlideAngle() const noexcept;

    void setCurrentSlideAngle(core::Angle a) noexcept;

    void setTargetState(loader::LaraStateId state);
    loader::LaraStateId getTargetState() const;

    std::unique_ptr<AbstractStateHandler> stopIfCeilingBlocked(LaraState& state);
    std::unique_ptr<AbstractStateHandler> tryClimb(LaraState& state);
    std::unique_ptr<AbstractStateHandler> checkWallCollision(LaraState& state);
    bool tryStartSlide(LaraState& state, std::unique_ptr<AbstractStateHandler>& nextHandler);
    std::unique_ptr<AbstractStateHandler> tryGrabEdge(LaraState& state);
    void jumpAgainstWall(LaraState& state);
    std::unique_ptr<AbstractStateHandler> checkJumpWallSmash(LaraState& state);

    void applyCollisionFeedback(LaraState& state);
    int getRelativeHeightAtDirection(core::Angle angle, int dist) const;
    std::unique_ptr<AbstractStateHandler> commonJumpHandling(LaraState& state);
    std::unique_ptr<AbstractStateHandler> commonSlideHandling(LaraState& state);
    std::unique_ptr<AbstractStateHandler> commonEdgeHangHandling(LaraState& state);
    std::unique_ptr<AbstractStateHandler> tryReach(LaraState& state);
    bool canClimbOnto(util::Axis axis) const;

    bool applyLandingDamage();

    irr::scene::ISceneNode* getLara();
    irr::core::aabbox3di getBoundingBox() const;

    void addSwimToDiveKeypressDuration(int ms) noexcept;

    void setSwimToDiveKeypressDuration(int ms) noexcept;

    int getSwimToDiveKeypressDuration() const noexcept;

    void setUnderwaterState(UnderwaterState u) noexcept;

    void setCameraRotation(core::Angle x, core::Angle y);
    void setCameraRotationX(core::Angle x);
    void setCameraRotationY(core::Angle y);
};
