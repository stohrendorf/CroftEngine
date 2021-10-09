#pragma once

#include "core/angle.h"
#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "pathfinder.h"
#include "qs/qs.h"
#include "serialization/serialization_fwd.h"

#include <algorithm>
#include <boost/throw_exception.hpp>
#include <cstdint>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <ostream>
#include <stdexcept>

namespace engine::objects
{
class AIAgent;
} // namespace engine::objects

namespace engine::world
{
class World;
struct Box;
} // namespace engine::world

namespace engine::ai
{
enum class Mood
{
  Bored,
  Attack,
  Escape,
  Stalk
};

inline std::ostream& operator<<(std::ostream& str, const Mood mood)
{
  switch(mood)
  {
  case Mood::Bored:
    return str << "Bored";
  case Mood::Attack:
    return str << "Attack";
  case Mood::Escape:
    return str << "Escape";
  case Mood::Stalk:
    return str << "Stalk";
  default:
    BOOST_THROW_EXCEPTION(std::runtime_error("Invalid mood"));
  }
}

struct EnemyLocation
{
  uint32_t zoneId = 0;
  uint32_t enemyZoneId = 0;
  bool enemyUnreachable = false;
  core::Area enemyDistance{0};
  bool enemyAhead = false;
  bool canAttackForward = false;
  core::Angle angleToEnemy = 0_deg;
  core::Angle enemyAngleToSelf = 0_deg;

  explicit EnemyLocation(objects::AIAgent& aiAgent);

  [[nodiscard]] bool canReachEnemyZone() const noexcept
  {
    return !enemyUnreachable && zoneId == enemyZoneId;
  }
};

struct CreatureInfo
{
  core::Angle headRotation = 0_deg;
  core::Angle neckRotation = 0_deg;
  core::RotationSpeed maxTurnSpeed = 1_deg / 1_frame;
  Mood mood = Mood::Bored;
  PathFinder pathFinder;
  core::TRVec target;

  CreatureInfo(const world::World& world, const core::TypeId& type, const gsl::not_null<const world::Box*>& initialBox);

  // serialization constructor
  explicit CreatureInfo() = default;

  void rotateHead(const core::Angle& angle)
  {
    const auto delta = std::clamp(angle - headRotation, -5_deg, +5_deg);
    headRotation = std::clamp(delta + headRotation, -90_deg, +90_deg);
  }

  void serialize(const serialization::Serializer<world::World>& ser);
};

std::unique_ptr<CreatureInfo> create(const serialization::TypeId<std::unique_ptr<CreatureInfo>>&,
                                     const serialization::Serializer<world::World>& ser);

void serialize(std::unique_ptr<CreatureInfo>& data, const serialization::Serializer<world::World>& ser);

void updateMood(const objects::AIAgent& aiAgent, const EnemyLocation& enemyLocation, bool violent);
} // namespace engine::ai
