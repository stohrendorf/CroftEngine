#pragma once

#include "loader/larastateid.h"
#include "loader/datatypes.h"
#include "loader/animationids.h"
#include "core/angle.h"
#include "core/interpolatedvalue.h"

#include <memory>


namespace engine
{
    using LaraStateId = loader::LaraStateId;
    struct CollisionInfo;
    class LaraNode;
    struct InputState;
    enum class UnderwaterState;


    namespace lara
    {
        class AbstractStateHandler
        {
        public:
            explicit AbstractStateHandler(LaraNode& lara)
                : m_lara{lara}
            {
            }


            virtual ~AbstractStateHandler() = default;

            virtual boost::optional<LaraStateId> postprocessFrame(CollisionInfo& collisionInfo) = 0;

            void animate(::engine::CollisionInfo& collisionInfo, const std::chrono::microseconds& deltaTimeMs);


            boost::optional<LaraStateId> handleInput(CollisionInfo& collisionInfo)
            {
                m_xMovement = 0;
                m_yMovement = 0;
                m_zMovement = 0;
                m_xRotationSpeed = 0_deg;
                m_yRotationSpeed = 0_deg;
                m_zRotationSpeed = 0_deg;
                return handleInputImpl(collisionInfo);
            }


            static std::unique_ptr<AbstractStateHandler> create(loader::LaraStateId id, LaraNode& lara);

            virtual loader::LaraStateId getId() const noexcept = 0;

        private:
            LaraNode& m_lara;

            friend class StateHandler_2;

            virtual void animateImpl(CollisionInfo& collisionInfo, const std::chrono::microseconds& deltaTime) = 0;

            virtual boost::optional<LaraStateId> handleInputImpl(CollisionInfo& collisionInfo) = 0;

        protected:
            core::InterpolatedValue<core::Angle> m_xRotationSpeed{0_deg};
            core::InterpolatedValue<core::Angle> m_yRotationSpeed{0_deg};
            core::InterpolatedValue<core::Angle> m_zRotationSpeed{0_deg};
            core::InterpolatedValue<float> m_xMovement{0.0f};
            core::InterpolatedValue<float> m_yMovement{0.0f};
            core::InterpolatedValue<float> m_zMovement{0.0f};


            LaraNode& getLara()
            {
                return m_lara;
            }


            const core::InterpolatedValue<float>& getHealth() const noexcept;

            void setHealth(const core::InterpolatedValue<float>& h) noexcept;

            void setAir(const core::InterpolatedValue<float>& a) noexcept;

            void setMovementAngle(core::Angle angle) noexcept;

            core::Angle getMovementAngle() const noexcept;

            void setFallSpeed(const core::InterpolatedValue<float>& spd);

            const core::InterpolatedValue<float>& getFallSpeed() const noexcept;

            bool isFalling() const noexcept;

            void setFalling(bool falling) noexcept;

            int getHandStatus() const noexcept;

            void setHandStatus(int status) noexcept;

            std::chrono::microseconds getCurrentTime() const;

            loader::LaraStateId getCurrentAnimState() const;

            void setAnimIdGlobal(loader::AnimationId anim, const boost::optional<uint16_t>& firstFrame = boost::none);

            const core::TRRotation& getRotation() const noexcept;

            void setHorizontalSpeed(const core::InterpolatedValue<float>& speed);

            const core::InterpolatedValue<float>& getHorizontalSpeed() const;

            const level::Level& getLevel() const;

            void placeOnFloor(const CollisionInfo& collisionInfo);

            const core::ExactTRCoordinates& getPosition() const;

            void setPosition(const core::ExactTRCoordinates& pos);

            long getFloorHeight() const;

            void setFloorHeight(long h);

            void setYRotationSpeed(core::Angle spd);

            core::Angle getYRotationSpeed() const;

            void subYRotationSpeed(const std::chrono::microseconds& deltaTime, core::Angle val,
                                   core::Angle limit = -32768_au);

            void addYRotationSpeed(const std::chrono::microseconds& deltaTime, core::Angle val,
                                   core::Angle limit = 32767_au);

            void setXRotation(core::Angle y);

            void setYRotation(core::Angle y);

            void setZRotation(core::Angle z);

            void setFallSpeedOverride(int v);

            void dampenHorizontalSpeed(const std::chrono::microseconds& deltaTime, float f);

            core::Angle getCurrentSlideAngle() const noexcept;

            void setCurrentSlideAngle(core::Angle a) noexcept;

            void setTargetState(loader::LaraStateId state);

            loader::LaraStateId getTargetState() const;

            boost::optional<LaraStateId> stopIfCeilingBlocked(const CollisionInfo& collisionInfo);

            boost::optional<LaraStateId> tryClimb(CollisionInfo& collisionInfo);

            boost::optional<LaraStateId> checkWallCollision(CollisionInfo& collisionInfo);

            bool tryStartSlide(const CollisionInfo& collisionInfo, boost::optional<LaraStateId>& nextHandler);

            boost::optional<LaraStateId> tryGrabEdge(CollisionInfo& collisionInfo);

            void jumpAgainstWall(CollisionInfo& collisionInfo);

            boost::optional<LaraStateId> checkJumpWallSmash(CollisionInfo& collisionInfo);

            void applyCollisionFeedback(const CollisionInfo& collisionInfo);

            int getRelativeHeightAtDirection(core::Angle angle, int dist) const;

            boost::optional<LaraStateId> commonJumpHandling(CollisionInfo& collisionInfo);

            boost::optional<LaraStateId> commonSlideHandling(CollisionInfo& collisionInfo);

            boost::optional<LaraStateId> commonEdgeHangHandling(CollisionInfo& collisionInfo);

            boost::optional<LaraStateId> tryReach(CollisionInfo& collisionInfo);

            bool canClimbOnto(core::Axis axis) const;

            bool applyLandingDamage();

            gameplay::BoundingBox getBoundingBox() const;

            void addSwimToDiveKeypressDuration(const std::chrono::microseconds& ms) noexcept;

            void setSwimToDiveKeypressDuration(const std::chrono::microseconds& ms) noexcept;

            const boost::optional<std::chrono::microseconds>& getSwimToDiveKeypressDuration() const;

            void setUnderwaterState(UnderwaterState u) noexcept;

            void setCameraRotation(core::Angle x, core::Angle y);

            void setCameraRotationX(core::Angle x);

            void setCameraRotationY(core::Angle y);

            void setCameraDistance(int d);

            void setCameraUnknown1(int k);
        };
    }
}
