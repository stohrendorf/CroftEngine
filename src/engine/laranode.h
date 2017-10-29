#pragma once

#include "loader/animationid.h"
#include "loader/larastateid.h"
#include "collisioninfo.h"
#include "engine/lara/abstractstatehandler.h"
#include "engine/items/itemnode.h"
#include "cameracontroller.h"
#include "ai/ai.h"


namespace engine
{
    struct CollisionInfo;


    enum class UnderwaterState
    {
        OnLand,
        Diving,
        Swimming
    };


    class LaraNode final : public items::ModelItemNode
    {
        using LaraStateId = loader::LaraStateId;

    private:
        int m_health{core::LaraHealth};
        //! @brief Additional rotation per TR Engine Frame
        core::Angle m_yRotationSpeed{0_deg};
        int m_fallSpeedOverride = 0;
        core::Angle m_movementAngle{0};
        int m_air{core::LaraAir};
        core::Angle m_currentSlideAngle{0};

        int m_handStatus = 0;
        //! @todo Move this to the Level.
        int m_uvAnimTime{0};

        UnderwaterState m_underwaterState = UnderwaterState::OnLand;

    public:
        LaraNode(const gsl::not_null<level::Level*>& level,
                 const std::string& name,
                 const gsl::not_null<const loader::Room*>& room,
                 const core::Angle& angle,
                 const core::TRCoordinates& position,
                 uint16_t activationState,
                 int16_t darkness,
                 const loader::SkeletalModelType& animatedModel)
            : ModelItemNode(level, name, room, angle, position, activationState, false, SaveHitpoints | SaveFlags | SavePosition | NonLot, darkness, animatedModel)
            , routePlanner{0, -20 * loader::SectorSize, 20*loader::SectorSize, loader::QuarterSectorSize}
        {
            setAnimIdGlobal(loader::AnimationId::STAY_IDLE);
            setTargetState(LaraStateId::Stop);
            setMovementAngle(m_state.rotation.Y);
        }


        ~LaraNode() override;


        bool isInWater() const
        {
            return m_underwaterState == UnderwaterState::Swimming || m_underwaterState == UnderwaterState::Diving;
        }


        bool isDiving() const
        {
            return m_underwaterState == UnderwaterState::Diving;
        }


        int getAir() const
        {
            return m_air;
        }


        void updateImpl();

        void update() override;

    private:
        void handleLaraStateOnLand();

        void handleLaraStateDiving();

        void handleLaraStateSwimming();

        void testInteractions();

        //! @brief If "none", we are not allowed to dive until the "Dive" action key is released
        //! @remarks This happens e.g. just after dive-to-swim transition, when players still
        //!          keep the "Dive Forward" action key pressed; in this case, you usually won't go
        //!          diving immediately again.
        int m_swimToDiveKeypressDuration = 0;
        uint16_t m_secretsFoundBitmask = 0;

    public:
        void setAir(int a) noexcept
        {
            m_air = a;
        }


        void setMovementAngle(core::Angle angle) noexcept
        {
            m_movementAngle = angle;
        }


        core::Angle getMovementAngle() const override
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
            return m_yRotationSpeed;
        }


        void subYRotationSpeed(core::Angle val, core::Angle limit = -32768_au)
        {
            m_yRotationSpeed = std::max(m_yRotationSpeed - val, limit);
        }


        void addYRotationSpeed(core::Angle val, core::Angle limit = 32767_au)
        {
            m_yRotationSpeed = std::min(m_yRotationSpeed + val, limit);
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

        loader::LaraStateId getCurrentAnimState() const;

        void setAnimIdGlobal(loader::AnimationId anim, const boost::optional<uint16_t>& firstFrame = boost::none);

        void updateFloorHeight(int dy);

        void handleCommandSequence(const uint16_t* floorData, bool skipFirstTriggers);

        boost::optional<int> getWaterSurfaceHeight() const;


        void addSwimToDiveKeypressDuration(int n) noexcept
        {
            m_swimToDiveKeypressDuration += n;
        }


        void setSwimToDiveKeypressDuration(int n) noexcept
        {
            m_swimToDiveKeypressDuration = n;
        }


        int getSwimToDiveKeypressDuration() const noexcept
        {
            return m_swimToDiveKeypressDuration;
        }


        void setUnderwaterState(UnderwaterState u) noexcept
        {
            m_underwaterState = u;
        }


        void setCameraCurrentRotation(core::Angle x, core::Angle y);

        void setCameraCurrentRotationX(core::Angle x);

        void setCameraCurrentRotationY(core::Angle y);

        void setCameraTargetDistance(int d);

        void setCameraOldMode(CameraMode k);


        void addHeadRotationXY(const core::Angle& x, const core::Angle& y)
        {
            m_headRotation.X += x;
            m_headRotation.Y += y;
        }


        void setHeadRotationX(const core::Angle& v)
        {
            m_headRotation.X = v;
        }


        void setHeadRotationY(const core::Angle& v)
        {
            m_headRotation.Y = v;
        }


        void addHeadRotationXY(const core::Angle& x, const core::Angle& minX, const core::Angle& maxX, const core::Angle& y, const core::Angle& minY, const core::Angle& maxY)
        {
            m_headRotation.X = util::clamp(m_headRotation.X + x, minX, maxX);
            m_headRotation.Y = util::clamp(m_headRotation.Y + y, minY, maxY);
        }


        const core::TRRotation& getHeadRotation() const noexcept
        {
            return m_headRotation;
        }


        void setTorsoRotation(const core::TRRotation& r)
        {
            m_torsoRotation = r;
        }


        void setHeadRotation(const core::TRRotation& r)
        {
            m_headRotation = r;
        }


        const core::TRRotation& getTorsoRotation() const noexcept
        {
            return m_torsoRotation;
        }


        void resetHeadTorsoRotation()
        {
            m_headRotation = {0_deg, 0_deg, 0_deg};
            m_torsoRotation = {0_deg, 0_deg, 0_deg};
        }


        core::TRRotation m_headRotation;
        core::TRRotation m_torsoRotation;

#ifndef NDEBUG
        CollisionInfo lastUsedCollisionInfo;
#endif

        ai::RoutePlanner routePlanner;
        int m_underwaterCurrentStrength = 0;

        void handleUnderwaterCurrent(CollisionInfo& collisionInfo);
    };
}
