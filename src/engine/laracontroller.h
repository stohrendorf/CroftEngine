#pragma once

#include "loader/animationids.h"
#include "loader/larastateid.h"
#include "larastate.h"
#include "inputstate.h"
#include "abstractstatehandler.h"
#include "itemcontroller.h"

#include <irrlicht.h>

namespace engine
{
    struct LaraState;

    enum class UnderwaterState
    {
        OnLand,
        Diving,
        Swimming
    };

    class LaraController final : public ItemController
    {
        using LaraStateId = loader::LaraStateId;

    private:
        // Lara's vars
        core::InterpolatedValue<float> m_health{1000.0f};
        //! @brief Additional rotation in AU per TR Engine Frame
        core::InterpolatedValue<core::Angle> m_yRotationSpeed{0_deg};
        int m_fallSpeedOverride = 0;
        core::Angle m_movementAngle{0};
        core::InterpolatedValue<float> m_air{1800.0f};
        core::Angle m_currentSlideAngle{0};

        InputState m_inputState;

        int m_handStatus = 0;
        int m_floorHeight = 0;
        int m_uvAnimTime = 0;

        UnderwaterState m_underwaterState = UnderwaterState::OnLand;
        std::unique_ptr<AbstractStateHandler> m_currentStateHandler = nullptr;

    public:
        LaraController(gsl::not_null<level::Level*> level,
                       const std::shared_ptr<engine::AnimationController>& dispatcher,
                       gsl::not_null<irr::scene::ISceneNode*> lara,
                       const std::string& name,
                       gsl::not_null<const loader::Room*> room,
                       gsl::not_null<loader::Item*> item)
            : ItemController(level, dispatcher, lara, name, room, item, false)
        {
            playAnimation(loader::AnimationId::STAY_IDLE);
            setMovementAngle(getRotation().Y);
        }

        ~LaraController();

        void animate(bool isNewFrame) override;

        std::unique_ptr<AbstractStateHandler> processLaraAnimCommands(bool advanceFrame = false);

        void setInputState(const InputState& state)
        {
            m_inputState = state;
        }

        bool isInWater() const
        {
            return m_underwaterState == UnderwaterState::Swimming || m_underwaterState == UnderwaterState::Diving;
        }

        float getAir() const
        {
            return m_air.getCurrentValue();
        }

    private:
        void handleLaraStateOnLand(bool newFrame);
        void handleLaraStateDiving(bool newFrame);
        void handleLaraStateSwimming(bool newFrame);
        void testInteractions(LaraState& state);
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

        int getHandStatus() const noexcept
        {
            return m_handStatus;
        }

        void setHandStatus(int status) noexcept
        {
            m_handStatus = status;
        }

        void placeOnFloor(const LaraState& state);

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

        void setFallSpeedOverride(int v)
        {
            m_fallSpeedOverride = v;
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
        void updateFloorHeight(int dy);
        void handleTriggers(const uint16_t* floorData, bool skipFirstTriggers);

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
        void setCameraDistance(int d);
    };
}
