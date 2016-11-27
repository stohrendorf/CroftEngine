#pragma once

#include <set>

#include "heightinfo.h"
#include "core/angle.h"

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
        static constexpr int AxisColl40 = 0x40;
        static constexpr int AxisColl80 = 0x80;

        static constexpr int FrobbelFlag_UnpassableSteepUpslant = 0x01;
        static constexpr int FrobbelFlag_UnwalkableSteepFloor = 0x02;
        static constexpr int FrobbelFlag_UnwalkableDeadlyFloor = 0x04;
        static constexpr int FrobbelFlag08 = 0x08;
        static constexpr int FrobbelFlag10 = 0x10;
        static constexpr int FrobbelFlag20 = 0x20;
        static constexpr int FrobbelFlag40 = 0x40;
        static constexpr int FrobbelFlag80 = 0x80;

        int axisCollisions = AxisColl_None;
        mutable core::ExactTRCoordinates collisionFeedback;
        core::Axis orientationAxis = core::Axis::PosZ;
        core::Angle yAngle = 0_deg; // external
        int collisionRadius = 0; // external
        int frobbelFlags = 0; // external
        core::ExactTRCoordinates position; // external
        int neededFloorDistanceBottom = 0; // external
        int neededFloorDistanceTop = 0; // external
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
