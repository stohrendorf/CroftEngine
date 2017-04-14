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


            void handleInputImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                if( getRotation().X < 0_deg )
                    m_xRotationSpeed = -2_deg;
                else if( getRotation().X > 0_deg )
                    m_xRotationSpeed = 2_deg;
                else
                    m_xRotationSpeed = 0_deg;
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTimeMs) override
            {
                setFallSpeed(std::max(core::makeInterpolatedValue(0.0f), getFallSpeed() - core::makeInterpolatedValue(8.0f).getScaled(deltaTimeMs)));
            }


            void postprocessFrame(CollisionInfo& collisionInfo, const std::chrono::microseconds& deltaTime) override
            {
                setHealth(core::makeInterpolatedValue(-1.0f));
                setAir(core::makeInterpolatedValue(-1.0f));
                setHandStatus(1);
                auto h = getLara().getWaterSurfaceHeight();
                if( h && *h < getPosition().Y - 100 )
                    setPosition(getPosition() - core::ExactTRCoordinates(0, 5, 0));

                StateHandler_Underwater::postprocessFrame(collisionInfo, deltaTime);
            }
        };
    }
}
