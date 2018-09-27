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
    bool jump = false;
    bool moveSlow = false;
    bool roll = false;
    bool action = false;
    bool freeLook = false;
    bool debug = false;
    bool holster = false;
    bool _1 = false;
    bool _2 = false;
    bool _3 = false;
    bool _4 = false;
    bool _5 = false;
    bool _6 = false;
    bool save = false;
    bool load = false;
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
