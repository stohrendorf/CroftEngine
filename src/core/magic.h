#pragma once

#include "units.h"

#include <chrono>
#include <cstddef>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>
#include <gsl-lite/gsl-lite.hpp>

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
constexpr auto MenuFrameRate = 60_mframe / 1_sec;
constexpr auto LogicRate = 30_tick / 1_sec;
constexpr auto TimePerFrame
  = std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::seconds{1}) / FrameRate.get();

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

constexpr auto SlowTurnSpeedAcceleration = 2.25_deg / 1_frame / 1_frame;
constexpr auto TurnSpeedDeceleration = 2_deg / 1_frame / 1_frame;
constexpr auto SlowTurnSpeed = 4_deg / 1_frame;
constexpr auto OnWaterTurnSpeed = 4_deg / 1_frame;
constexpr auto OnWaterMovementTurnSpeed = 2_deg / 1_frame;
constexpr auto OnWaterMaxSpeed = 60_spd;
constexpr auto OnWaterAcceleration = 8_spd / 1_frame;
constexpr auto FastTurnSpeed = 8_deg / 1_frame;
constexpr auto JumpTurnSpeed = 3_deg / 1_frame;
constexpr auto RunBackTurnSpeed = 6_deg / 1_frame;
constexpr auto RunTiltAcceleration = 1.5_deg / 1_frame;
constexpr auto MaxRunTilt = 11_deg;
constexpr auto WaterCollisionRotationSpeedX = 2_deg / 1_frame;
constexpr auto WaterCollisionRotationSpeedY = 5_deg / 1_frame;
constexpr auto DiveRotationSpeedX = 2_deg / 1_frame;
constexpr auto DiveRotationSpeedY = 6_deg / 1_frame;
constexpr auto DiveRotationSpeedZ = 3_deg / 1_frame;

constexpr auto FreeLookHeadTurnSpeed = 2_deg / 1_frame;

constexpr auto HitAnimationLastFrame = 34_frame;

constexpr size_t SavegameSlots = 99;

inline const auto DefaultFov = Radians{glm::radians(60.0f)};
constexpr float DefaultNearPlane = 20.0f;
constexpr float DefaultFarPlane = 20480.0f;
constexpr Frame DefaultHealthBarTimeout = FrameRate * 1_sec * 4 / 3;

constexpr auto AnimFramesPerTick = 1_frame / 1_tick;

[[nodiscard]] constexpr Length operator""_sectors(unsigned long long value) noexcept
{
  return gsl_lite::narrow_cast<Length::type>(value) * SectorSize;
}

[[nodiscard]] constexpr auto sectorOf(const Length& l)
{
  return l / 1_sectors;
}

[[nodiscard]] constexpr auto toSectorLocal(const Length& l)
{
  if(l < 0_len)
    return 1_sectors - (-l) % 1_sectors;
  else
    return l % 1_sectors;
}

[[nodiscard]] constexpr auto snappedSector(const Length& l)
{
  return sectorOf(l) * 1_sectors;
}

inline const auto RenderAxisRight = glm::vec3{1, 0, 0};
inline const auto RenderAxisUp = glm::vec3{0, 1, 0};
} // namespace core

using core::operator""_sectors;