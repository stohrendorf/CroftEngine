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


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                if( getHealth() <= 0 )
                {
                    setTargetState(LaraStateId::WaterDeath);
                    return;
                }

                setSwimToDiveKeypressDuration(0);

                if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Backward )
                    setTargetState(LaraStateId::OnWaterStop);

                if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                    getLara().addYRotation(-2_deg);
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                    getLara().addYRotation(2_deg);

                setFallSpeed(std::min(60, getFallSpeed() + 8));
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setMovementAngle(getRotation().Y + 180_deg);
                commonOnWaterHandling(collisionInfo);
            }
        };
    }
}
