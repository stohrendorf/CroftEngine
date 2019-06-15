#pragma once

#include "statehandler_standing.h"
#include "engine/cameracontroller.h"
#include "engine/laranode.h"

namespace engine
{
namespace lara
{
class StateHandler_2 final
    : public StateHandler_Standing
{
public:
    explicit StateHandler_2(LaraNode& lara)
        : StateHandler_Standing{ lara, LaraStateId::Stop }
    {
    }

    void handleInput(CollisionInfo& collisionInfo) override
    {
        if( getLara().m_state.health <= 0_hp )
        {
            setGoalAnimState( LaraStateId::Death );
            return;
        }

        if( getEngine().getInputHandler().getInputState().roll )
        {
            setAnimation( AnimationId::ROLL_BEGIN );
            setGoalAnimState( LaraStateId::Stop );
            return;
        }

        setGoalAnimState( LaraStateId::Stop );

        if( getEngine().getInputHandler().getInputState().freeLook )
        {
            getEngine().getCameraController().setMode( CameraMode::FreeLook );
            getLara().addHeadRotationXY(
                (-hid::FreeLookMouseMovementScale.retype_as<float>()
                    * (getEngine().getInputHandler().getInputState().mouseMovement.y / 2000)).retype_as<core::Angle>(),
                -42_deg, 22_deg,
                (hid::FreeLookMouseMovementScale.retype_as<float>()
                    * (getEngine().getInputHandler().getInputState().mouseMovement.x / 2000)).retype_as<core::Angle>(),
                -44_deg, 44_deg
                                       );
            getLara().setTorsoRotation( getLara().getHeadRotation() );

            return;
        }

        if( getEngine().getCameraController().getMode() == CameraMode::FreeLook )
        {
            getEngine().getCameraController().setMode( CameraMode::Chase );
        }

        if( getEngine().getInputHandler().getInputState().stepMovement == hid::AxisMovement::Left )
        {
            setGoalAnimState( LaraStateId::StepLeft );
        }
        else if( getEngine().getInputHandler().getInputState().stepMovement == hid::AxisMovement::Right )
        {
            setGoalAnimState( LaraStateId::StepRight );
        }

        if( getEngine().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left )
        {
            setGoalAnimState( LaraStateId::TurnLeftSlow );
        }
        else if( getEngine().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right )
        {
            setGoalAnimState( LaraStateId::TurnRightSlow );
        }

        if( getEngine().getInputHandler().getInputState().jump )
        {
            setGoalAnimState( LaraStateId::JumpPrepare );
        }
        else if( getEngine().getInputHandler().getInputState().zMovement == hid::AxisMovement::Forward )
        {
            if( getEngine().getInputHandler().getInputState().moveSlow )
            {
                create( LaraStateId::WalkForward, getLara() )->handleInput( collisionInfo );
            }
            else
            {
                create( LaraStateId::RunForward, getLara() )->handleInput( collisionInfo );
            }
        }
        else if( getEngine().getInputHandler().getInputState().zMovement == hid::AxisMovement::Backward )
        {
            if( getEngine().getInputHandler().getInputState().moveSlow )
            {
                create( LaraStateId::WalkBackward, getLara() )->handleInput( collisionInfo );
            }
            else
            {
                setGoalAnimState( LaraStateId::RunBack );
            }
        }
    }
};
}
}
