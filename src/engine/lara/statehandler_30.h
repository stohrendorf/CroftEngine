#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_30 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_30(LaraNode& lara)
                    : AbstractStateHandler(lara, LaraStateId::ShimmyLeft)
            {
            }


            void handleInput(CollisionInfo& collisionInfo) override
            {
                setCameraRotation(-60_deg, 0_deg);
                collisionInfo.policyFlags &= ~(CollisionInfo::EnableBaddiePush | CollisionInfo::EnableSpaz);
                if( getLevel().m_inputHandler->getInputState().xMovement != AxisMovement::Left && getLevel().m_inputHandler->getInputState().stepMovement != AxisMovement::Left )
                    setTargetState(LaraStateId::Hang);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setMovementAngle(getRotation().Y - 90_deg);
                commonEdgeHangHandling(collisionInfo);
                setMovementAngle(getRotation().Y - 90_deg);
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/) override
            {
            }
        };
    }
}
