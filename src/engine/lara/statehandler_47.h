#pragma once

#include "statehandler_onwater.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_47 final : public StateHandler_OnWater
        {
        public:
            explicit StateHandler_47(LaraNode& lara)
                    : StateHandler_OnWater(lara, LaraStateId::OnWaterBackward)
            {
            }

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                if( getHealth() <= 0 )
                {
                    setTargetState(LaraStateId::WaterDeath);
                    return {};
                }

                setSwimToDiveKeypressDuration(std::chrono::microseconds::zero());

                if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Backward )
                    setTargetState(LaraStateId::OnWaterStop);

                return {};
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTimeMs) override
            {
                setFallSpeed(std::min(core::makeInterpolatedValue(60.0f), getFallSpeed() + core::makeInterpolatedValue(8.0f).getScaled(deltaTimeMs)));

                if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                    m_yRotationSpeed = -2_deg;
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                    m_yRotationSpeed = 2_deg;
                else
                    m_yRotationSpeed = 0_deg;
            }

            boost::optional<LaraStateId> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setMovementAngle(getRotation().Y + 180_deg);
                return commonOnWaterHandling(collisionInfo);
            }
        };
    }
}
