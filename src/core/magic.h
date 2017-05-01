#pragma once

#include "loader/datatypes.h"

namespace core
{
    constexpr int SteppableHeight = loader::QuarterSectorSize / 2;
    constexpr int ClimbLimit2ClickMin = loader::QuarterSectorSize + SteppableHeight;
    constexpr int ClimbLimit2ClickMax = loader::QuarterSectorSize + ClimbLimit2ClickMin;
    constexpr int ClimbLimit3ClickMax = loader::QuarterSectorSize + ClimbLimit2ClickMax;

    static_assert(SteppableHeight < ClimbLimit2ClickMin, "Constants wrong");
    static_assert(ClimbLimit2ClickMin < ClimbLimit2ClickMax, "Constants wrong");
    static_assert(ClimbLimit2ClickMax < ClimbLimit3ClickMax, "Constants wrong");

    constexpr int ScalpHeight = 762;
    constexpr int ScalpToHandsHeight = 160;
    constexpr int JumpReachableHeight = 896 + loader::SectorSize;

    constexpr int FreeFallSpeedThreshold = 131;
    constexpr int DamageFallSpeedThreshold = 140;
    constexpr int DeadlyFallSpeedThreshold = 154;

    static_assert(FreeFallSpeedThreshold < DamageFallSpeedThreshold, "Constants wrong");
    static_assert(DamageFallSpeedThreshold < DeadlyFallSpeedThreshold, "Constants wrong");

    constexpr int MaxGrabbableGradient = 60;

    constexpr int FrameRate = 30;

    constexpr int LaraAir = 1800;
    constexpr int LaraHealth = 1000;
}
