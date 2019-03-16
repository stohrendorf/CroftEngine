#pragma once

#include "core/angle.h"

namespace engine
{
namespace
{
const core::Angle FreeLookMouseMovementScale{50_deg};
}

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


    struct Button
    {
        bool current = false;
        bool previous = false;

        Button& operator=(bool state)
        {
            previous = std::exchange( current, state );
            return *this;
        }

        operator bool() const
        {
            return current;
        }

        bool justPressed() const
        {
            return current && current != previous;
        }
    };


    Button jump;
    Button moveSlow;
    Button roll;
    Button action;
    Button freeLook;
    Button debug;
    Button holster;
    Button _1;
    Button _2;
    Button _3;
    Button _4;
    Button _5;
    Button _6;
    Button save;
    Button load;
    glm::vec2 mouseMovement;

    void setXAxisMovement(const bool left, const bool right)
    {
        if( left < right )
            xMovement = AxisMovement::Right;
        else if( left > right )
            xMovement = AxisMovement::Left;
        else
            xMovement = AxisMovement::Null;
    }

    void setStepMovement(const bool left, const bool right)
    {
        if( left < right )
            stepMovement = AxisMovement::Right;
        else if( left > right )
            stepMovement = AxisMovement::Left;
        else
            stepMovement = AxisMovement::Null;
    }

    void setZAxisMovement(const bool back, const bool forward)
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
