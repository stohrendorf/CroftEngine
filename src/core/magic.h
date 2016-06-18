#pragma once

namespace core
{
constexpr int SteppableHeight = loader::QuarterSectorSize / 2;
constexpr int ClimbLimit2ClickMin = loader::QuarterSectorSize + SteppableHeight;
constexpr int ClimbLimit2ClickMax = loader::QuarterSectorSize + ClimbLimit2ClickMin;
constexpr int ClimbLimit3ClickMax = loader::QuarterSectorSize + ClimbLimit2ClickMax;

constexpr int ScalpHeight = 762;
constexpr int ScalpToHandsHeight = 160;
constexpr int JumpReachableHeight = 896 + loader::SectorSize;

constexpr int FreeFallSpeedThreshold = 131;
constexpr int MaxGrabbableGradient = 60;
}
