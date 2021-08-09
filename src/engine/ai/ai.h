#pragma once

#include "engine/world/box.h"
#include "pathfinder.h"

#include <boost/range/adaptor/map.hpp>

namespace engine
{
namespace objects
{
class AIAgent;
}

namespace objects
{
struct ObjectState;
}

namespace ai
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
  case Mood::Bored: return str << "Bored";
  case Mood::Attack: return str << "Attack";
  case Mood::Escape: return str << "Escape";
  case Mood::Stalk: return str << "Stalk";
  default: BOOST_THROW_EXCEPTION(std::runtime_error("Invalid mood"));
  }
}

struct EnemyLocation
{
  world::ZoneId zoneId = 0;
  world::ZoneId enemyZoneId = 0;
  bool enemyUnreachable = false;
  core::Area enemyDistance{0};
  bool enemyAhead = false;
  bool canAttackForward = false;
  core::Angle angleToEnemy = 0_deg;
  core::Angle enemyAngleToSelf = 0_deg;

  EnemyLocation(objects::AIAgent& aiAgent);

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

  CreatureInfo(const world::World& world, core::TypeId type, const gsl::not_null<const world::Box*>& initialBox);

  // serialization constructor
  explicit CreatureInfo(const world::World& world);

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
} // namespace ai
} // namespace engine
