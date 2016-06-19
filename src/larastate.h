#pragma once

#include <set>

#include "heightinfo.h"
#include "core/angle.h"

class LaraController;

struct LaraState
{
    static constexpr int AxisColl_None = 0x00;
    static constexpr int AxisColl_FrontBlocked = 0x01;
    static constexpr int AxisColl_FrontLeftBlocked = 0x02;
    static constexpr int AxisColl_FrontRightBlocked = 0x04;
    static constexpr int AxisColl_FrontCeilingBlocked = 0x08;
    static constexpr int AxisColl_InsufficientCeilingSpace = 0x10;
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

    int axisCollisions;
    loader::ExactTRCoordinates collisionFeedback;
    util::Axis orientationAxis;
    core::Angle yAngle; // external
    int collisionRadius; // external
    int frobbelFlags; // external
    loader::ExactTRCoordinates position; // external
    int neededFloorDistanceBottom; // external
    int neededFloorDistanceTop; // external
    int neededCeilingDistance; // external

    VerticalInfo current;
    VerticalInfo front;
    VerticalInfo frontLeft;
    VerticalInfo frontRight;

    int8_t floorSlantX;
    int8_t floorSlantZ;

    bool hasStaticMeshCollision = false;

    static float reflectAtSectorBoundary(float target, float current)
    {
        const auto targetSector = gsl::narrow_cast<int>(std::floor(target / loader::SectorSize));
        const auto currentSector = gsl::narrow_cast<int>(std::floor(current / loader::SectorSize));
        if( targetSector == currentSector )
            return 0;

        const auto targetInSector = gsl::narrow_cast<float>(std::fmod(target, loader::SectorSize));
        if(current <= target)
            return -targetInSector;
        else
            return loader::SectorSize - 1 - targetInSector;
    }

    void initHeightInfo(const loader::ExactTRCoordinates& laraPos, const loader::Level& level, int height);

    static std::set<const loader::Room*> collectNeighborRooms(const loader::ExactTRCoordinates& position, int radius, int height, const loader::Level& level);
    bool checkStaticMeshCollisions(const loader::ExactTRCoordinates& position, int height, const loader::Level& level);
};
