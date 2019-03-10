#pragma once

#include "audio/tracktype.h"

#include <sol.hpp>

namespace engine
{
namespace script
{
struct ObjectInfo
{
    bool ai_agent = false;
    int radius = 10;
    int hit_points = -16384;
    int pivot_length = 0;
    int target_update_chance = 0;

    static sol::usertype<ObjectInfo>& userType()
    {
        static sol::usertype<ObjectInfo> userType(
                sol::constructors<ObjectInfo()>(),
                "ai_agent", &ObjectInfo::ai_agent,
                "radius", &ObjectInfo::radius,
                "hit_points", &ObjectInfo::hit_points,
                "pivot_length", &ObjectInfo::pivot_length,
                "target_update_chance", &ObjectInfo::target_update_chance
        );

        return userType;
    }
};


struct TrackInfo
{
    TrackInfo(int id, audio::TrackType type)
            : id{id}
            , type{type}
    {}

    int id;
    audio::TrackType type;

    static sol::usertype<TrackInfo>& userType()
    {
        static sol::usertype<TrackInfo> userType{
                sol::constructors<TrackInfo(int, audio::TrackType)>()
        };

        return userType;
    }
};
}
}
