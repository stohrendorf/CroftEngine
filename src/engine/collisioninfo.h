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
        static constexpr int AxisColl_FrontForwardBlocked = 0x01;
        static constexpr int AxisColl_FrontLeftBlocked = 0x02;
        static constexpr int AxisColl_FrontRightBlocked = 0x04;
        static constexpr int AxisColl_ScalpCollision = 0x08;
        static constexpr int AxisColl_InsufficientFrontCeilingSpace = 0x10;
        static constexpr int AxisColl_InvalidPosition = 0x20;

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
        //! The deepest floor distance considered passable (aka @c bad_pos).
        int passableFloorDistanceBottom = 0; // external
        //! The highest floor distance considered passable (aka @c bad_neg).
        int passableFloorDistanceTop = 0; // external
        //! (aka @c bad_ceiling).
        int neededCeilingDistance = 0; // external

        VerticalInfo mid;
        VerticalInfo front;
        VerticalInfo frontLeft;
        VerticalInfo frontRight;

        int8_t floorSlantX = 0;
        int8_t floorSlantZ = 0;

        bool hasStaticMeshCollision = false;

        void initHeightInfo(const core::TRCoordinates& laraPos, const level::Level& level, int height);

        static std::set<const loader::Room*> collectNeighborRooms(const core::TRCoordinates& position, int radius, int height, const level::Level& level);
        bool checkStaticMeshCollisions(const core::TRCoordinates& position, int height, const level::Level& level);
    };
}
