#pragma once

#include "statehandler_onwater.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_34 final : public StateHandler_OnWater
        {
        public:
            explicit StateHandler_34(LaraNode& lara)
                : StateHandler_OnWater(lara, LaraStateId::OnWaterForward)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                if( getHealth() <= 0 )
                {
                    setTargetState(LaraStateId::WaterDeath);
                    return;
                }

                setSwimToDiveKeypressDuration(0);

                if (getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left)
                    getLara().addYRotation(-4_deg);
                else if (getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right)
                    getLara().addYRotation(4_deg);

                if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward )
                    setTargetState(LaraStateId::OnWaterStop);

                if( getLevel().m_inputHandler->getInputState().jump )
                    setTargetState(LaraStateId::OnWaterStop);

                setFallSpeed(std::min(60, getFallSpeed() + 8));
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setMovementAngle(getRotation().Y);
                commonOnWaterHandling(collisionInfo);
            }
        };
    }
}
