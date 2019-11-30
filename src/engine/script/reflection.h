#pragma once

#include "audio/tracktype.h"
#include "core/id.h"

#include <sol/sol.hpp>

namespace engine::script
{
struct ObjectInfo
{
  bool ai_agent = false;
  core::Length::type radius = 10;
  core::Health::type hit_points = -16384;
  core::Length::type pivot_length = 0;
  int target_update_chance = 0;

  static void registerUserType(sol::state& lua)
  {
    lua.new_usertype<ObjectInfo>("ObjectInfo",
                                 sol::constructors<ObjectInfo()>(),
                                 "ai_agent",
                                 &ObjectInfo::ai_agent,
                                 "radius",
                                 &ObjectInfo::radius,
                                 "hit_points",
                                 &ObjectInfo::hit_points,
                                 "pivot_length",
                                 &ObjectInfo::pivot_length,
                                 "target_update_chance",
                                 &ObjectInfo::target_update_chance);
  }
};

struct TrackInfo
{
  TrackInfo(core::SoundId::type id, audio::TrackType type)
      : id{id}
      , type{type}
  {
  }

  core::SoundId::type id;
  audio::TrackType type;

  static void registerUserType(sol::state& lua)
  {
    lua.new_usertype<TrackInfo>("TrackInfo", sol::constructors<TrackInfo(core::SoundId::type, audio::TrackType)>());
  }
};
} // namespace engine::script
