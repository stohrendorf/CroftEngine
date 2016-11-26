#pragma once

#include "loader/datatypes.h"

#include <chrono>

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

    constexpr int FrameRate = 30;
    constexpr std::chrono::microseconds FrameTime = std::chrono::microseconds(std::chrono::seconds(1)) / FrameRate;

    constexpr std::chrono::microseconds fromFrame(uint16_t f)
    {
        return std::chrono::microseconds(std::chrono::seconds(f)) / core::FrameRate;
    }

    inline uint16_t toFrame(const std::chrono::microseconds& time)
    {
        return gsl::narrow<uint16_t>(time / core::FrameTime);
    }
}

constexpr std::chrono::microseconds operator""_frame(unsigned long long f)
{
    return core::fromFrame(f);
}
