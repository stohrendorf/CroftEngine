#pragma once

#include "audio/tracktype.h"
#include "core/id.h"

namespace engine::script
{
struct ObjectInfo
{
  bool ai_agent = false;
  core::Length::type radius = 10;
  core::Health::type hit_points = -16384;
  core::Length::type pivot_length = 0;
  int target_update_chance = 0;
};

struct TrackInfo
{
  TrackInfo(core::SoundEffectId::type id, audio::TrackType type)
      : id{id}
      , type{type}
  {
  }

  core::SoundEffectId id;
  audio::TrackType type;
};
} // namespace engine::script
