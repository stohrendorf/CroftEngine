#pragma once

#include "audio/tracktype.h"
#include "core/id.h"

#include <sol.hpp>

namespace engine
{
namespace script
{
struct ObjectInfo
{
    bool ai_agent = false;
    core::Length::type radius = 10;
    core::Health::type hit_points = -16384;
    core::Length::type pivot_length = 0;
    int target_update_chance = 0;

    static sol::usertype<ObjectInfo>& userType()
    {
        static sol::usertype<ObjectInfo> userType(sol::constructors<ObjectInfo()>(),
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

        return userType;
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

    static sol::usertype<TrackInfo>& userType()
    {
        static sol::usertype<TrackInfo> userType{sol::constructors<TrackInfo(core::SoundId::type, audio::TrackType)>()};

        return userType;
    }
};
} // namespace script
} // namespace engine
