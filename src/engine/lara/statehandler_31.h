#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_31 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_31(LaraNode& lara)
                : AbstractStateHandler(lara, LaraStateId::ShimmyRight)
            {
            }


            void handleInput(CollisionInfo& collisionInfo) override
            {
                collisionInfo.policyFlags &= ~(CollisionInfo::EnableBaddiePush | CollisionInfo::EnableSpaz);
                setCameraRotation(-60_deg, 0_deg);
                if( getLevel().m_inputHandler->getInputState().xMovement != AxisMovement::Right && getLevel().m_inputHandler->getInputState().stepMovement != AxisMovement::Right )
                    setTargetState(LaraStateId::Hang);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setMovementAngle(getRotation().Y + 90_deg);
                commonEdgeHangHandling(collisionInfo);
                setMovementAngle(getRotation().Y + 90_deg);
            }
        };
    }
}
