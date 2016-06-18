#pragma once

#include "animationids.h"
#include "larastateid.h"
#include "larastate.h"
#include "util/vmath.h"
#include "inputstate.h"
#include "abstractstatehandler.h"

#include <irrlicht.h>

struct LaraState;

enum class UnderwaterState
{
    OnLand,
    Diving,
    Swimming
};

class LaraController final : public irr::scene::ISceneNodeAnimator
{
    using LaraStateId = loader::LaraStateId;

private:
    const loader::Level* const m_level;
    std::shared_ptr<loader::AnimationController> m_dispatcher;
    const std::string m_name;

    irr::scene::IAnimatedMeshSceneNode* const m_sceneNode;

    // Lara's vars
    core::InterpolatedValue<float> m_health{ 1000.0f };
    //! @brief Additional rotation in AU per TR Engine Frame
    core::InterpolatedValue<core::Angle> m_yRotationSpeed{ 0_deg };
    bool m_falling = false;
    core::InterpolatedValue<float> m_fallSpeed{ 0.0f };
    core::InterpolatedValue<float> m_horizontalSpeed{ 0.0f };
    int m_fallSpeedOverride = 0;
    core::Angle m_movementAngle{0};
    core::InterpolatedValue<float> m_air{ 1800.0f };
    core::Angle m_currentSlideAngle{0};

    InputState m_inputState;

    int m_handStatus = 0;
    int m_floorHeight = 0;
    int m_lastFrameTime = -1;
    int m_lastEngineFrameTime = -1;
    int m_currentFrameTime = 0;
    int m_lastAnimFrame = -1;
    int m_uvAnimTime = 0;

    UnderwaterState m_underwaterState = UnderwaterState::OnLand;
    std::unique_ptr<AbstractStateHandler> m_currentStateHandler = nullptr;

    int getCurrentDeltaTime() const
    {
        BOOST_ASSERT(m_lastFrameTime < m_currentFrameTime);
        return m_currentFrameTime - m_lastFrameTime;
    }

    // needed for YPR rotation, because the scene node uses XYZ rotation
    irr::core::vector3d<core::Angle> m_rotation;

    loader::ExactTRCoordinates m_position;

    const loader::Room* m_currentRoom;

public:
    LaraController(const loader::Level* level, const std::shared_ptr<loader::AnimationController>& dispatcher, irr::scene::IAnimatedMeshSceneNode* lara, const std::string& name)
        : m_level(level), m_dispatcher(dispatcher), m_name(name), m_sceneNode(lara)
    {
        BOOST_ASSERT(level != nullptr);
        BOOST_ASSERT(dispatcher != nullptr);
        BOOST_ASSERT(lara != nullptr);
        playAnimation(loader::AnimationId::STAY_IDLE);

        auto laraRot = lara->getRotation();
        m_rotation.X = core::degToAngle(laraRot.X);
        m_rotation.Y = core::degToAngle(laraRot.Y);
        m_rotation.Z = core::degToAngle(laraRot.Z);

        setMovementAngle(m_rotation.Y);

        m_sceneNode->updateAbsolutePosition();
        m_position = loader::ExactTRCoordinates(m_sceneNode->getAbsolutePosition());
    }

    ~LaraController();

    void animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs) override;

    enum class AnimCommandOpcode : uint16_t
    {
        SetPosition = 1,
        SetVelocity = 2,
        EmptyHands = 3,
        Kill = 4,
        PlaySound = 5,
        PlayEffect = 6,
        Interact = 7
    };

    std::unique_ptr<AbstractStateHandler> processAnimCommands();

    ISceneNodeAnimator* createClone(irr::scene::ISceneNode* /*node*/, irr::scene::ISceneManager* /*newManager*/ = nullptr) override
    {
        BOOST_ASSERT(false);
        return nullptr;
    }

    void setInputState(const InputState& state)
    {
        m_inputState = state;
    }

    irr::scene::IAnimatedMeshSceneNode* getSceneNode() const noexcept
    {
        return m_sceneNode;
    }

    const loader::ExactTRCoordinates& getPosition() const noexcept
    {
        return m_position;
    }

    const loader::Room* getCurrentRoom() const noexcept
    {
        return m_currentRoom;
    }

    void setCurrentRoom(const loader::Room* newRoom);

private:
    void handleLaraStateOnLand(bool newFrame);
    void handleLaraStateDiving(bool newFrame);
    void handleLaraStateSwimming(bool newFrame);
    int m_swimToDiveKeypressDuration = 0;

    ///////////////////////////////////////

public:
    const core::InterpolatedValue<float>& getHealth() const noexcept
    {
        return m_health;
    }

    void setHealth(const core::InterpolatedValue<float>& h) noexcept
    {
        m_health = h;
    }

    void setAir(const core::InterpolatedValue<float>& a) noexcept
    {
        m_air = a;
    }

    const InputState& getInputState() const noexcept
    {
        return m_inputState;
    }

    void setMovementAngle(core::Angle angle) noexcept
    {
        m_movementAngle = angle;
    }

    core::Angle getMovementAngle() const noexcept
    {
        return m_movementAngle;
    }

    void setFallSpeed(const core::InterpolatedValue<float>& spd)
    {
        m_fallSpeed = spd;
    }

    const core::InterpolatedValue<float>& getFallSpeed() const noexcept
    {
        return m_fallSpeed;
    }

    bool isFalling() const noexcept
    {
        return m_falling;
    }

    void setFalling(bool falling) noexcept
    {
        m_falling = falling;
    }

    int getHandStatus() const noexcept
    {
        return m_handStatus;
    }

    void setHandStatus(int status) noexcept
    {
        m_handStatus = status;
    }

    irr::u32 getCurrentFrame() const;
    irr::u32 getAnimEndFrame() const;

    const irr::core::vector3d<core::Angle>& getRotation() const noexcept
    {
        return m_rotation;
    }

    void setHorizontalSpeed(const core::InterpolatedValue<float>& speed)
    {
        m_horizontalSpeed = speed;
    }

    const core::InterpolatedValue<float>& getHorizontalSpeed() const
    {
        return m_horizontalSpeed;
    }

    const loader::Level& getLevel() const
    {
        BOOST_ASSERT(m_level != nullptr);
        return *m_level;
    }

    void placeOnFloor(const LaraState& state);

    void rotate(core::Angle dx, core::Angle dy, core::Angle dz)
    {
        m_rotation.X += dx;
        m_rotation.Y += dy;
        m_rotation.Z += dz;
    }

    void move(float dx, float dy, float dz)
    {
        m_position.X += dx;
        m_position.Y += dy;
        m_position.Z += dz;
    }

    void moveLocal(float dx, float dy, float dz)
    {
        const auto sin = getRotation().Y.sin();
        const auto cos = getRotation().Y.cos();
        m_position.X += dz * sin + dx * cos;
        m_position.Y += dy;
        m_position.Z += dz * cos - dx * sin;
    }

    void setPosition(const loader::ExactTRCoordinates& pos)
    {
        m_position = pos;
    }

    int getFloorHeight() const noexcept
    {
        return m_floorHeight;
    }

    void setFloorHeight(int h) noexcept
    {
        m_floorHeight = h;
    }

    void setYRotationSpeed(core::Angle spd)
    {
        m_yRotationSpeed = spd;
    }

    core::Angle getYRotationSpeed() const
    {
        return static_cast<core::Angle>(m_yRotationSpeed);
    }

    void subYRotationSpeed(core::Angle val, core::Angle limit = -32768_au)
    {
        m_yRotationSpeed.sub(val, getCurrentDeltaTime()).limitMin(limit);
    }

    void addYRotationSpeed(core::Angle val, core::Angle limit = 32767_au)
    {
        m_yRotationSpeed.add(val, getCurrentDeltaTime()).limitMax(limit);
    }

    void setXRotation(core::Angle x)
    {
        m_rotation.X = x;
    }

    void addXRotation(core::Angle x)
    {
        m_rotation.X += x;
    }

    void setYRotation(core::Angle y)
    {
        m_rotation.Y = y;
    }

    void addYRotation(core::Angle v)
    {
        m_rotation.Y += v;
    }

    void setZRotation(core::Angle z)
    {
        m_rotation.Z = z;
    }

    void addZRotation(core::Angle z)
    {
        m_rotation.Z += z;
    }

    void setFallSpeedOverride(int v)
    {
        m_fallSpeedOverride = v;
    }

    void dampenHorizontalSpeed(float f)
    {
        m_horizontalSpeed.sub(m_horizontalSpeed * f, getCurrentDeltaTime());
    }

    core::Angle getCurrentSlideAngle() const noexcept
    {
        return m_currentSlideAngle;
    }

    void setCurrentSlideAngle(core::Angle a) noexcept
    {
        m_currentSlideAngle = a;
    }

    LaraStateId getTargetState() const;
    void setTargetState(loader::LaraStateId st);
    loader::LaraStateId getCurrentState() const;
    loader::LaraStateId getCurrentAnimState() const;
    void playAnimation(loader::AnimationId anim, const boost::optional<irr::u32>& firstFrame = boost::none);
    void applyRotation();
    void updateFloorHeight(int dy);
    void handleTriggers(const uint16_t* floorData, bool skipFirstTriggers);

    irr::core::aabbox3di getBoundingBox() const;

    boost::optional<int> getWaterSurfaceHeight() const;

    void addSwimToDiveKeypressDuration(int ms) noexcept
    {
        m_swimToDiveKeypressDuration += ms;
    }

    void setSwimToDiveKeypressDuration(int ms) noexcept
    {
        m_swimToDiveKeypressDuration = ms;
    }

    int getSwimToDiveKeypressDuration() const noexcept
    {
        return m_swimToDiveKeypressDuration;
    }

    void setUnderwaterState(UnderwaterState u) noexcept
    {
        m_underwaterState = u;
    }

    void setCameraRotation(core::Angle x, core::Angle y);

    void setCameraRotationX(core::Angle x);
    
    void setCameraRotationY(core::Angle y);
};
