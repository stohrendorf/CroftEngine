#pragma once

#include "pathfinder.h"

#include <boost/range/adaptor/map.hpp>
#include <sol/sol.hpp>

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

  AiInfo(Engine& engine, objects::ObjectState& objectState);

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
  uint16_t flags = 0;
  Mood mood = Mood::Bored;
  PathFinder pathFinder;
  core::TRVec target;

  CreatureInfo(const Engine& engine, core::TypeId type);

  void rotateHead(const core::Angle& angle)
  {
    const auto delta = util::clamp(angle - head_rotation, -5_deg, +5_deg);
    head_rotation = util::clamp(delta + head_rotation, -90_deg, +90_deg);
  }

  static void registerUserType(sol::state& lua)
  {
    lua.new_usertype<CreatureInfo>("CreatureInfo",
                                   sol::meta_function::construct,
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
  }

  void serialize(const serialization::Serializer& ser);
};

std::shared_ptr<CreatureInfo> create(const serialization::TypeId<std::shared_ptr<CreatureInfo>>&,
                                     const serialization::Serializer& ser);

void serialize(std::shared_ptr<CreatureInfo>& data, const serialization::Serializer& ser);

void updateMood(const Engine& engine, const objects::ObjectState& objectState, const AiInfo& aiInfo, bool violent);
} // namespace ai
} // namespace engine
