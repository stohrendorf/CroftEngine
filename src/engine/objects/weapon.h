#pragma once

#include "core/interval.h"
#include "core/units.h"
#include "engine/soundeffects_tr1.h"
#include "engine/weapontype.h"
#include "qs/qs.h"

namespace engine::objects
{
struct RangeXY
{
  core::Interval<core::Angle> y{};
  core::Interval<core::Angle> x{};
};

struct Weapon
{
  WeaponType type = WeaponType::None;
  RangeXY lockAngles{};
  RangeXY leftAngles{};
  RangeXY rightAngles{};
  core::RotationSpeed aimSpeed = 0_deg / 1_frame;
  core::Angle shotInaccuracy = 0_deg;
  core::Length weaponHeight = 0_len;
  core::Health damage = 0_hp;
  core::Length targetDist = 0_len;
  core::Frame recoilDuration = 0_frame;
  core::Frame flashTime = 0_frame;
  TR1SoundEffect shotSound = TR1SoundEffect::LaraFootstep;

  static const Weapon& get(WeaponType weaponType);
};
} // namespace engine::objects
