#pragma once

#include "core/angle.h"
#include "core/units.h"
#include "engine/weapontype.h"
#include "serialization/serialization_fwd.h"

#include <cstddef>

namespace loader::file
{
struct AnimFrame;
}

namespace engine::world
{
class World;
}

namespace engine::objects
{
struct Weapon;
class LaraObject;

constexpr auto TwoWeaponsIdle = 0_frame;
constexpr auto TwoWeaponsAiming = 4_frame;
constexpr auto DrawTwoWeaponsAnimStart = 5_frame;
constexpr auto TwoWeaponsTouchingHolsters = 12_frame;
constexpr auto DrawTwoWeaponsAnimEnd = 23_frame;
constexpr auto TwoWeaponsRecoilAnimStart = 24_frame;

constexpr auto ShotgunIdle = 0_frame;
constexpr auto ShotgunIdleToAimAnimEnd = 12_frame;

constexpr auto ShotgunIdleToAimAnimStart = 13_frame;
constexpr auto ShotgunTakeHolster = 23_frame;
constexpr auto ShotgunReadyToShoot = 47_frame;

constexpr auto ShotgunAfterShotAnimStart = 48_frame;
constexpr auto ShotgunReload = 56_frame;
constexpr auto ShotgunAfterShotIdle = 59_frame;
constexpr auto ShotgunAfterShotAnimEnd = 79_frame;

constexpr auto HolsterShotgunAnimStart = 80_frame;
constexpr auto ShotgunPutHolster = 100_frame;
constexpr auto HolsterShotgunAnimEnd = 113_frame;

constexpr auto ShotgunAimToIdleAnimStart = 114_frame;
constexpr auto ShotgunAimToIdleAnimEnd = 126_frame;

struct AimInfo
{
  size_t handBoneId{std::numeric_limits<size_t>::max()};
  size_t thighBoneId{std::numeric_limits<size_t>::max()};

  const loader::file::AnimFrame* weaponAnimData = nullptr;
  core::Frame frame = 0_frame;
  bool aiming = false;
  core::TRRotationXY aimRotation{};
  core::Frame flashTimeout = 0_frame;

  void updateAnimTwoWeapons(LaraObject& lara, const Weapon& weapon);
  void updateAnimShotgun(LaraObject& lara);
  void holsterTwoWeapons(LaraObject& lara, WeaponType weaponType);
  void holsterShotgun(LaraObject& lara);
  void overrideHolsterTwoWeaponsMeshes(LaraObject& lara, WeaponType weaponType);
  void overrideDrawTwoWeaponsMeshes(LaraObject& lara, WeaponType weaponType);

  void updateAimAngles(const Weapon& weapon, const core::TRRotationXY& weaponTargetVector) noexcept;

  void serialize(const serialization::Serializer<world::World>& ser) const;
  void deserialize(const serialization::Deserializer<world::World>& ser);

  void reset() noexcept
  {
    frame = 0_frame;
    aiming = false;
    aimRotation.X = 0_deg;
    aimRotation.Y = 0_deg;
  }

private:
  void updateAnimShotgunAiming(LaraObject& lara);
};
} // namespace engine::objects
