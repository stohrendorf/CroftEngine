#pragma once

#include "statehandler_underwater.h"

#include "engine/laranode.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_44 final : public StateHandler_Underwater
        {
        public:
            explicit StateHandler_44(LaraNode& lara)
                : StateHandler_Underwater(lara, LaraStateId::WaterDeath)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                setFallSpeed(std::max(0, getFallSpeed() - 8));

                if( getRotation().X < 0_deg )
                {
                    if( getRotation().X >= 2_deg )
                        getLara().addXRotation(-2_deg);
                    else
                        setXRotation(0_deg);
                }
                else if( getRotation().X > 0_deg )
                {
                    if( getRotation().X <= -2_deg )
                        getLara().addXRotation(2_deg);
                    else
                        setXRotation(0_deg);
                }
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setHealth(-1);
                setAir(-1);
                setHandStatus(1);
                auto h = getLara().getWaterSurfaceHeight();
                if( h && *h < getPosition().Y - 100 )
                    setPosition(getPosition() - core::TRCoordinates(0, 5, 0));

                StateHandler_Underwater::postprocessFrame(collisionInfo);
            }
        };
    }
}
