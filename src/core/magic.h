#pragma once

#include "units.h"

namespace core
{
constexpr auto SectorSize = 1024_len;

constexpr auto QuarterSectorSize = SectorSize / 4;

constexpr auto HeightLimit = QuarterSectorSize * 127;
constexpr auto InvalidHeight = -HeightLimit;

constexpr auto SteppableHeight = QuarterSectorSize / 2;
constexpr auto ClimbLimit2ClickMin = QuarterSectorSize + SteppableHeight;
constexpr auto ClimbLimit2ClickMax = QuarterSectorSize + ClimbLimit2ClickMin;
constexpr auto ClimbLimit3ClickMax = QuarterSectorSize + ClimbLimit2ClickMax;

static_assert(SteppableHeight < ClimbLimit2ClickMin, "Constants wrong");
static_assert(ClimbLimit2ClickMin < ClimbLimit2ClickMax, "Constants wrong");
static_assert(ClimbLimit2ClickMax < ClimbLimit3ClickMax, "Constants wrong");

constexpr auto LaraWalkHeight = 762_len;
constexpr auto LaraSwimHeight = 700_len;
constexpr auto LaraDiveHeight = 400_len;
constexpr auto LaraDiveGroundElevation = 200_len;
constexpr auto LaraHangingHeight = 870_len;
constexpr auto ScalpToHandsHeight = 160_len;
constexpr auto JumpReachableHeight = ClimbLimit3ClickMax + SectorSize;

constexpr auto FreeFallSpeedThreshold = 131_spd;
constexpr auto DamageFallSpeedThreshold = 140_spd;
constexpr auto DeadlyFallSpeedThreshold = 154_spd;
constexpr auto DeadlyHeadFallSpeedThreshold = 133_spd;

static_assert(FreeFallSpeedThreshold < DamageFallSpeedThreshold, "Constants wrong");
static_assert(FreeFallSpeedThreshold < DeadlyHeadFallSpeedThreshold, "Constants wrong");
static_assert(DamageFallSpeedThreshold < DeadlyFallSpeedThreshold, "Constants wrong");

constexpr auto MaxGrabbableGradient = 60_len;

constexpr auto FrameRate = 30_frame / 1_sec;

constexpr auto LaraAir = FrameRate * 60_sec;
constexpr auto LaraHealth = 1000_hp;
constexpr auto DeadHealth = -16384_hp;

constexpr auto DefaultCollisionRadius = 100_len;
constexpr auto DefaultCollisionRadiusUnderwater = 300_len;

constexpr auto CameraWallDistance = QuarterSectorSize + 50_len;
constexpr auto DefaultCameraLaraDistance = SectorSize * 3 / 2;
constexpr auto CombatCameraLaraDistance = SectorSize + DefaultCameraLaraDistance;

constexpr auto Gravity = 6_spd / 1_frame;
constexpr auto TerminalGravity = 1_spd / 1_frame;
constexpr auto TerminalSpeed = 128_spd;
} // namespace core
