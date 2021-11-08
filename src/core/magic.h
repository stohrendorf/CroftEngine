#pragma once

#include "angle.h"
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

constexpr auto MaxGrabbableGradient = 60_len;

constexpr auto AnimFrameRate = (30_frame / 1_sec).cast<int>();
constexpr auto RenderFrameRate = (60_rframe / 1_sec).cast<int>();

template<typename T>
constexpr auto toAnimUnit(const T& x)
{
  return x * core::RenderFrameRate / core::AnimFrameRate;
}

constexpr RenderFrame toAnimUnit(const Frame& x)
{
  return (x * core::RenderFrameRate / core::AnimFrameRate).cast<RenderFrame>();
}

template<typename T>
constexpr auto toRenderUnit(const T& x)
{
  return x * core::AnimFrameRate / core::RenderFrameRate;
}

constexpr Frame toRenderUnit(const RenderFrame& x)
{
  return (x * core::AnimFrameRate / core::RenderFrameRate).cast<Frame>();
}

[[nodiscard]] inline bool isPhysicsFrame(const RenderFrame& f)
{
  return (f % toAnimUnit(1_frame)) == 0_rframe;
}

constexpr auto FreeFallSpeedThreshold = 131_spd;
constexpr auto DamageFallSpeedThreshold = 140_spd;
constexpr auto DeadlyFallSpeedThreshold = 154_spd;
constexpr auto DeadlyHeadFallSpeedThreshold = 133_spd;

static_assert(FreeFallSpeedThreshold < DamageFallSpeedThreshold, "Constants wrong");
static_assert(FreeFallSpeedThreshold < DeadlyHeadFallSpeedThreshold, "Constants wrong");
static_assert(DamageFallSpeedThreshold < DeadlyFallSpeedThreshold, "Constants wrong");

constexpr auto LaraAir = (RenderFrameRate * 60_sec).cast<RenderFrame>();
constexpr auto LaraHealth = 1000_hp;
constexpr auto DeadHealth = -16384_hp;

constexpr auto DefaultCollisionRadius = 100_len;
constexpr auto DefaultCollisionRadiusUnderwater = 300_len;

constexpr auto CameraWallDistance = QuarterSectorSize + 50_len;
constexpr auto DefaultCameraLaraDistance = SectorSize * 1.5f;
constexpr auto CombatCameraLaraDistance = SectorSize + DefaultCameraLaraDistance;

constexpr auto Gravity = 6_spd / 1_frame;
constexpr auto TerminalGravity = 1_spd / 1_frame;
constexpr auto TerminalSpeed = 128_spd;

constexpr auto SlowTurnSpeedAcceleration = toRenderUnit(toRenderUnit(2.25_deg / 1_frame) / 1_frame);
constexpr auto TurnSpeedDeceleration = toRenderUnit(toRenderUnit(2_deg / 1_frame) / 1_frame);
constexpr auto SlowTurnSpeed = toRenderUnit(4_deg / 1_frame);
constexpr auto OnWaterTurnSpeed = toRenderUnit(4_deg / 1_frame);
constexpr auto OnWaterMovementTurnSpeed = toRenderUnit(2_deg / 1_frame);
constexpr auto OnWaterMaxSpeed = 60_spd;
constexpr auto OnWaterAcceleration = 8_spd / 1_frame;
constexpr auto FastTurnSpeed = toRenderUnit(8_deg / 1_frame);
constexpr auto JumpTurnSpeed = toRenderUnit(3_deg / 1_frame);
constexpr auto RunBackTurnSpeed = toRenderUnit(6_deg / 1_frame);
constexpr auto RunTiltReductionSpeed = toRenderUnit(1_deg / 1_frame);
constexpr auto RunTiltAcceleration = toRenderUnit(1.5_deg / 1_frame);
constexpr auto MaxRunTilt = 11_deg;
constexpr auto WaterCollisionRotationSpeedX = toRenderUnit(2_deg / 1_frame);
constexpr auto WaterCollisionRotationSpeedY = toRenderUnit(5_deg / 1_frame);
constexpr auto DiveRotationSpeedX = toRenderUnit(2_deg / 1_frame);
constexpr auto DiveRotationSpeedY = toRenderUnit(6_deg / 1_frame);
constexpr auto DiveRotationSpeedZ = toRenderUnit(3_deg / 1_frame);

constexpr auto FreeLookHeadTurnSpeed = toRenderUnit(2_deg / 1_frame);

constexpr size_t SavegameSlots = 100;
} // namespace core
