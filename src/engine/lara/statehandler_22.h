#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_22 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_22(LaraNode& lara)
                    : AbstractStateHandler(lara, LaraStateId::StepLeft)
            {
            }

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                if( getHealth() <= 0 )
                {
                    setTargetState(LaraStateId::Stop);
                    return {};
                }

                if( getLevel().m_inputHandler->getInputState().stepMovement != AxisMovement::Left )
                    setTargetState(LaraStateId::Stop);

                if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                    setYRotationSpeed(std::max(-4_deg, getYRotationSpeed() - 2.25_deg));
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                    setYRotationSpeed(std::min(+4_deg, getYRotationSpeed() + 2.25_deg));

                return {};
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }

            boost::optional<LaraStateId> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(false);
                collisionInfo.passableFloorDistanceBottom = 128;
                collisionInfo.passableFloorDistanceTop = -128;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.yAngle = getRotation().Y - 90_deg;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnpassableSteepUpslant | CollisionInfo::FrobbelFlag_UnwalkableSteepFloor;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                auto nextHandler = stopIfCeilingBlocked(collisionInfo);
                if( nextHandler.is_initialized() )
                    return nextHandler;

                nextHandler = checkWallCollision(collisionInfo);
                if( nextHandler.is_initialized() )
                {
                    setAnimIdGlobal(loader::AnimationId::STAY_SOLID, 185);
                    setTargetState(LaraStateId::Stop);
                    return LaraStateId::Stop;
                }

                if( !tryStartSlide(collisionInfo, nextHandler) )
                    setPosition(getPosition() + core::ExactTRCoordinates(0, gsl::narrow_cast<float>(collisionInfo.current.floor.distance), 0));

                return nextHandler;
            }
        };
    }
}
