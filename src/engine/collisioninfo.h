#pragma once

#include "core/angle.h"
#include "core/interval.h"
#include "core/units.h"
#include "core/vec.h"
#include "heightinfo.h"
#include "type_safe/flag_set.hpp"

#include <cstdint>
#include <gsl/gsl-lite.hpp> // IWYU pragma: keep
#include <set>

namespace engine::world
{
class World;
struct Room;
} // namespace engine::world

namespace engine
{
struct CollisionInfo
{
  enum class AxisColl : uint8_t
  {
    None,
    Front,
    FrontLeft,
    FrontRight,
    Top,
    FrontTop,
    Jammed //!< Not enough space between floor and ceiling
  };

  enum class PolicyFlags : uint8_t
  {
    SlopesAreWalls,
    SlopesArePits,
    LavaIsPit,
    EnableBaddiePush,
    EnableSpaz,
    _flag_set_size [[maybe_unused]]
  };

  using PolicyFlagSet = type_safe::flag_set<PolicyFlags>;
  static constexpr const type_safe::flag_combo<PolicyFlags> SlopeBlockingPolicy
    = PolicyFlags::SlopesAreWalls | PolicyFlags::SlopesArePits;
  static constexpr const type_safe::flag_combo<PolicyFlags> SpazPushPolicy
    = PolicyFlags::EnableBaddiePush | PolicyFlags::EnableSpaz;

  AxisColl collisionType = AxisColl::None;
  mutable core::TRVec shift;
  core::Axis facingAxis = core::Axis::PosZ;
  core::Angle facingAngle = 0_deg;                             // external
  core::Length collisionRadius = 0_len;                        // external
  PolicyFlagSet policies;                                      // external
  core::TRVec initialPosition;                                 // external
  core::Interval<core::Length> validFloorHeight{0_len, 0_len}; // external
  core::Length validCeilingHeightMin = 0_len;                  // external

  VerticalDistances mid;
  VerticalDistances front;
  VerticalDistances frontLeft;
  VerticalDistances frontRight;

  int8_t floorSlantX = 0;
  int8_t floorSlantZ = 0;

  bool hasStaticMeshCollision = false;

  void initHeightInfo(const core::TRVec& laraPos, const world::World& world, const core::Length& height);

  static std::set<gsl::not_null<const world::Room*>> collectTouchingRooms(const core::TRVec& position,
                                                                          const core::Length& radius,
                                                                          const core::Length& height,
                                                                          const world::World& world);

  bool checkStaticMeshCollisions(const core::TRVec& objectPos,
                                 const core::Length& objectHeight,
                                 const world::World& world);
};
} // namespace engine
