#pragma once

#include "pathfinder.h"

#include <boost/range/adaptor/map.hpp>

namespace engine
{
namespace objects
{
class AIAgent;
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

struct AiInfo
{
  loader::file::ZoneId zone_number = 0;
  loader::file::ZoneId enemy_zone = 0;
  bool enemy_unreachable = false;
  core::Area distance{0};
  bool ahead = false;
  bool bite = false;
  core::Angle angle = 0_deg;
  core::Angle enemy_facing = 0_deg;

  AiInfo(World& world, objects::ObjectState& objectState);

  [[nodiscard]] bool canReachEnemyZone() const
  {
    return !enemy_unreachable && zone_number == enemy_zone;
  }
};

struct CreatureInfo
{
  core::TypeId type{uint16_t{0}};
  core::Angle head_rotation = 0_deg;
  core::Angle neck_rotation = 0_deg;
  core::Angle maximum_turn = 1_deg;
  uint16_t flags = 0; // TODO: this is an auxiliary field containing highly object-specific data, move to concrete types
  Mood mood = Mood::Bored;
  PathFinder pathFinder;
  core::TRVec target;

  CreatureInfo(const World& world, core::TypeId type);

  void rotateHead(const core::Angle& angle)
  {
    const auto delta = std::clamp(angle - head_rotation, -5_deg, +5_deg);
    head_rotation = std::clamp(delta + head_rotation, -90_deg, +90_deg);
  }

  void serialize(const serialization::Serializer<World>& ser);
};

std::shared_ptr<CreatureInfo> create(const serialization::TypeId<std::shared_ptr<CreatureInfo>>&,
                                     const serialization::Serializer<World>& ser);

void serialize(std::shared_ptr<CreatureInfo>& data, const serialization::Serializer<World>& ser);

void updateMood(const World& world, const objects::ObjectState& objectState, const AiInfo& aiInfo, bool violent);
} // namespace ai
} // namespace engine
