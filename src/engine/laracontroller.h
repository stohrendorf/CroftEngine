#pragma once

#include "loader/animationids.h"
#include "loader/larastateid.h"
#include "collisioninfo.h"
#include "abstractstatehandler.h"
#include "itemcontroller.h"

namespace engine
{
    struct CollisionInfo;

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

        int m_handStatus = 0;
        std::chrono::microseconds m_uvAnimTime{ 0 };

        UnderwaterState m_underwaterState = UnderwaterState::OnLand;
        std::unique_ptr<AbstractStateHandler> m_currentStateHandler = nullptr;

    public:
        LaraController(const gsl::not_null<level::Level*>& level,
                       const std::string& name,
                       const gsl::not_null<const loader::Room*>& room,
                       const gsl::not_null<loader::Item*>& item,
                       const loader::AnimatedModel& animatedModel)
            : ItemController(level, name, room, item, false, 0x3c, animatedModel)
        {
            playAnimation(loader::AnimationId::STAY_IDLE);
            setTargetState(loader::LaraStateId::Stop);
            setMovementAngle(getRotation().Y);
        }

        ~LaraController();

        void animateImpl(bool isNewFrame) override;

        std::unique_ptr<AbstractStateHandler> processLaraAnimCommands(bool advanceFrame = false);

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
        void testInteractions();
        //! @brief If "none", we are not allowed to dive until the "Dive" action key is released
        //! @remarks This happens e.g. just after dive-to-swim transition, when players still
        //!          keep the "Dive Forward" action key pressed; in this case, you usually won't go
        //!          diving immediately again.
        boost::optional<std::chrono::microseconds> m_swimToDiveKeypressDuration = boost::none;
        uint16_t m_secretsFoundBitmask = 0;

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

        void placeOnFloor(const CollisionInfo& collisionInfo);

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
        void playAnimation(loader::AnimationId anim, const boost::optional<core::Frame>& firstFrame = boost::none);
        void updateFloorHeight(int dy);
        void handleTriggers(const uint16_t* floorData, bool skipFirstTriggers);

        boost::optional<int> getWaterSurfaceHeight() const;

        void addSwimToDiveKeypressDuration(const std::chrono::microseconds& ms) noexcept
        {
            if(!m_swimToDiveKeypressDuration)
                return;

            *m_swimToDiveKeypressDuration += ms;
        }

        void setSwimToDiveKeypressDuration(const std::chrono::microseconds& ms) noexcept
        {
            m_swimToDiveKeypressDuration = ms;
        }

        const boost::optional<std::chrono::microseconds>& getSwimToDiveKeypressDuration() const noexcept
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
        void setCameraUnknown1(int k);
        void processAnimCommands(bool /*advanceFrame*/) override
        {
            // no-op
        }

#ifndef NDEBUG
        CollisionInfo lastUsedCollisionInfo;
#endif
    };
}
