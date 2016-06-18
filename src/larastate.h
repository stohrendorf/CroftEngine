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

    static int fruityFeedback(int a, int b)
    {
        const auto sectorA = a / loader::SectorSize;
        const auto sectorB = b / loader::SectorSize;
        if( sectorA == sectorB )
            return 0;

        const auto localA = (a % loader::SectorSize) + 1;
        if( sectorB <= sectorA )
            return -localA;
        else
            return loader::SectorSize - localA;
    }

    void initHeightInfo(const loader::TRCoordinates& laraPos, const loader::Level& level, int height);

    static std::set<const loader::Room*> collectNeighborRooms(const loader::TRCoordinates& position, int radius, int height, const loader::Level& level);
    bool checkStaticMeshCollisions(const loader::TRCoordinates& position, int height, const loader::Level& level);
};
