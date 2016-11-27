#pragma once

#include "statehandler_underwater.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_17 final : public StateHandler_Underwater
        {
        public:
            explicit StateHandler_17(LaraNode& lara)
                    : StateHandler_Underwater(lara)
            {
            }

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                if( getHealth() < 0 )
                {
                    setTargetState(LaraStateId::WaterDeath);
                    return {};
                }

                handleDiveInput();

                if( !getLevel().m_inputHandler->getInputState().jump )
                    setTargetState(LaraStateId::UnderwaterInertia);

                return {};
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTimeMs) override
            {
                setFallSpeed((getFallSpeed() + core::makeInterpolatedValue(8.0f).getScaled(deltaTimeMs)).limitMax(200.0f));
            }

            loader::LaraStateId getId() const noexcept override
            {
                return LaraStateId::UnderwaterForward;
            }
        };
    }
}
