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

        int axisCollisions = AxisColl_None;
        mutable core::ExactTRCoordinates collisionFeedback;
        core::Axis orientationAxis = core::Axis::PosZ;
        core::Angle yAngle = 0_deg; // external
        int collisionRadius = 0; // external
        int policyFlags = 0; // external
        core::ExactTRCoordinates oldPosition; // external
        //! The deepest floor distance considered passable.
        int passableFloorDistanceBottom = 0; // external
        //! The highest floor distance considered passable.
        int passableFloorDistanceTop = 0; // external
        int neededCeilingDistance = 0; // external

        VerticalInfo current;
        VerticalInfo front;
        VerticalInfo frontLeft;
        VerticalInfo frontRight;

        int8_t floorSlantX = 0;
        int8_t floorSlantZ = 0;

        bool hasStaticMeshCollision = false;

        static float reflectAtSectorBoundary(float target, float current)
        {
            const auto targetSector = gsl::narrow_cast<int>(std::floor(target / loader::SectorSize));
            const auto currentSector = gsl::narrow_cast<int>(std::floor(current / loader::SectorSize));
            if( targetSector == currentSector )
                return 0;

            const auto targetInSector = gsl::narrow_cast<float>(std::fmod(target, loader::SectorSize));
            if( current <= target )
                return -targetInSector;
            else
                return loader::SectorSize - 1 - targetInSector;
        }

        void initHeightInfo(const core::ExactTRCoordinates& laraPos, const level::Level& level, int height);

        static std::set<const loader::Room*> collectNeighborRooms(const core::ExactTRCoordinates& position, int radius, int height, const level::Level& level);
        bool checkStaticMeshCollisions(const core::ExactTRCoordinates& position, int height, const level::Level& level);
    };
}
