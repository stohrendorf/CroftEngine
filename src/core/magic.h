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

    using FrameRatio = std::ratio<1, FrameRate>;
    using Frame = std::chrono::duration<int, FrameRatio>;

    inline Frame toFrame(const std::chrono::microseconds& duration)
    {
        return std::chrono::duration_cast<Frame>(duration);
    }

    inline std::chrono::microseconds toTime(const Frame& frame)
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(frame);
    }
}

inline core::Frame operator""_frame(unsigned long long f)
{
    return core::Frame{ gsl::narrow<int>(f) };
}
