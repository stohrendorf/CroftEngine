#pragma once

namespace engine
{
    enum class AxisMovement
    {
        Positive,
        Right = Positive,
        Forward = Positive,
        Null,
        Negative,
        Left = Negative,
        Backward = Negative
    };

    struct InputState
    {
        AxisMovement xMovement = AxisMovement::Null;
        AxisMovement zMovement = AxisMovement::Null;
        AxisMovement stepMovement = AxisMovement::Null;
        bool jump = false;
        bool moveSlow = false;
        bool roll = false;
        bool action = false;

        void setXAxisMovement(bool left, bool right)
        {
            if( left < right )
                xMovement = AxisMovement::Right;
            else if( left > right )
                xMovement = AxisMovement::Left;
            else
                xMovement = AxisMovement::Null;
        }

        void setStepMovement(bool left, bool right)
        {
            if( left < right )
                stepMovement = AxisMovement::Right;
            else if( left > right )
                stepMovement = AxisMovement::Left;
            else
                stepMovement = AxisMovement::Null;
        }

        void setZAxisMovement(bool back, bool forward)
        {
            if( back < forward )
                zMovement = AxisMovement::Forward;
            else if( back > forward )
                zMovement = AxisMovement::Backward;
            else
                zMovement = AxisMovement::Null;
        }
    };
}
