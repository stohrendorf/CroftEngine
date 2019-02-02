#pragma once

#include "heightinfo.h"
#include "core/angle.h"

#include "type_safe/flag_set.hpp"

#include <set>

namespace engine
{
class LaraNode;


struct CollisionInfo
{
    enum class AxisColl
    {
        None,
        Front,
        Left,
        Right,
        Top,
        TopBottom,
        TopFront
    };

    enum class PolicyFlags
    {
        SlopesAreWalls,
        SlopesArePits,
        LavaIsPit,
        EnableBaddiePush,
        EnableSpaz,
        _flag_set_size
    };

    using PolicyFlagSet = type_safe::flag_set<PolicyFlags>;
    static constexpr const type_safe::flag_combo<PolicyFlags> SlopeBlockingPolicy = PolicyFlags::SlopesAreWalls
                                                                                    | PolicyFlags::SlopesArePits;
    static constexpr const type_safe::flag_combo<PolicyFlags> SpazPushPolicy = PolicyFlags::EnableBaddiePush
                                                                               | PolicyFlags::EnableSpaz;

    AxisColl collisionType = AxisColl::None;
    mutable core::TRVec shift;
    core::Axis facingAxis = core::Axis::PosZ;
    core::Angle facingAngle = 0_deg; // external
    int collisionRadius = 0; // external
    PolicyFlagSet policyFlags; // external
    core::TRVec oldPosition; // external
    //! The deepest floor distance considered passable.
    int badPositiveDistance = 0; // external
    //! The highest floor distance considered passable.
    int badNegativeDistance = 0; // external
    int badCeilingDistance = 0; // external

    VerticalInfo mid;
    VerticalInfo front;
    VerticalInfo frontLeft;
    VerticalInfo frontRight;

    int8_t floorSlantX = 0;
    int8_t floorSlantZ = 0;

    bool hasStaticMeshCollision = false;

    void initHeightInfo(const core::TRVec& laraPos, const level::Level& level, int height);

    static std::set<gsl::not_null<const loader::Room*>>
    collectTouchingRooms(const core::TRVec& position, int radius, int height, const level::Level& level);

    bool checkStaticMeshCollisions(const core::TRVec& position, int height, const level::Level& level);
};


inline const char* toString(CollisionInfo::AxisColl value)
{
    switch( value )
    {

        case CollisionInfo::AxisColl::None:
            return "None";
        case CollisionInfo::AxisColl::Front:
            return "Front";
        case CollisionInfo::AxisColl::Left:
            return "Left";
        case CollisionInfo::AxisColl::Right:
            return "Right";
        case CollisionInfo::AxisColl::Top:
            return "Top";
        case CollisionInfo::AxisColl::TopBottom:
            return "TopBottom";
        case CollisionInfo::AxisColl::TopFront:
            return "TopFront";
        default:
            return "<null>";
    }
}

}
