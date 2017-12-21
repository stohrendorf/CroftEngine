#pragma once

#include "heightinfo.h"
#include "core/angle.h"

#include <set>

namespace engine
{
class LaraNode;


struct CollisionInfo
{
    static constexpr int AxisColl_None = 0x00;
    static constexpr int AxisColl_Front = 0x01;
    static constexpr int AxisColl_Left = 0x02;
    static constexpr int AxisColl_Right = 0x04;
    static constexpr int AxisColl_Top = 0x08;
    static constexpr int AxisColl_TopBottom = 0x10;
    static constexpr int AxisColl_TopFront = 0x20;

    //! @name PolicyFlags
    //! @brief Policy flags
    //! @see #policyFlags
    //! @{
    static constexpr int SlopesAreWalls = 0x01;
    static constexpr int SlopesArePits = 0x02;
    static constexpr int LavaIsPit = 0x04;
    static constexpr int EnableBaddiePush = 0x08;
    static constexpr int EnableSpaz = 0x10;
    //! @}

    int collisionType = AxisColl_None;
    mutable core::TRCoordinates shift;
    core::Axis facingAxis = core::Axis::PosZ;
    core::Angle facingAngle = 0_deg; // external
    int collisionRadius = 0; // external
    int policyFlags = 0; // external
    core::TRCoordinates oldPosition; // external
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

    void initHeightInfo(const core::TRCoordinates& laraPos, const level::Level& level, int height);

    static std::set<gsl::not_null<const loader::Room*>>
    collectTouchingRooms(const core::TRCoordinates& position, int radius, int height, const level::Level& level);

    bool checkStaticMeshCollisions(const core::TRCoordinates& position, int height, const level::Level& level);
};
}
