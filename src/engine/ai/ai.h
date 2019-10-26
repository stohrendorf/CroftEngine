#pragma once

#include "pathfinder.h"

#include <boost/range/adaptor/map.hpp>

namespace engine
{
namespace items
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
  loader::file::ZoneId zone_number;

  loader::file::ZoneId enemy_zone;
  bool enemy_unreachable = false;

  core::Area distance{0};

  bool ahead;

  bool bite;

  core::Angle angle;

  core::Angle enemy_facing;

  AiInfo(engine::Engine& engine, items::ItemState& item);

  bool canReachEnemyZone() const
  {
    return !enemy_unreachable && zone_number == enemy_zone;
  }
};

struct CreatureInfo
{
  core::Angle head_rotation = 0_deg;

  core::Angle neck_rotation = 0_deg;

  core::Angle maximum_turn = 1_deg;

  uint16_t flags = 0;

  Mood mood = Mood::Bored;

  PathFinder pathFinder;

  core::TRVec target;

  CreatureInfo(const engine::Engine& engine, core::TypeId type);

  void rotateHead(const core::Angle& angle)
  {
    const auto delta = util::clamp(angle - head_rotation, -5_deg, +5_deg);
    head_rotation = util::clamp(delta + head_rotation, -90_deg, +90_deg);
  }

  static sol::usertype<CreatureInfo>& userType()
  {
    static auto type = sol::usertype<CreatureInfo>(sol::meta_function::construct,
                                                   sol::no_constructor,
                                                   "head_rotation",
                                                   &CreatureInfo::head_rotation,
                                                   "neck_rotation",
                                                   &CreatureInfo::neck_rotation,
                                                   "maximum_turn",
                                                   &CreatureInfo::maximum_turn,
                                                   "flags",
                                                   &CreatureInfo::flags,
                                                   "mood",
                                                   &CreatureInfo::mood,
                                                   "target",
                                                   &CreatureInfo::target);
    return type;
  }

  YAML::Node save(const Engine& engine) const;

  void load(const YAML::Node& n, const Engine& engine);
};

void updateMood(const engine::Engine& engine, const items::ItemState& item, const AiInfo& aiInfo, bool violent);
} // namespace ai
} // namespace engine
