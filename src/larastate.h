#pragma once

#include "heightinfo.h"

class LaraStateHandler;

enum class Axis
{
    PosZ,
    PosX,
    NegZ,
    NegX
};

struct LaraState
{
    static constexpr int AxisColl_None = 0x00;
    static constexpr int AxisColl_InsufficientFrontSpace = 0x01;
    static constexpr int AxisColl_FrontLeftBump = 0x02;
    static constexpr int AxisColl_FrontRightBump = 0x04;
    static constexpr int AxisColl_HeadInCeiling = 0x08;
    static constexpr int AxisColl_BumpHead = 0x10;
    static constexpr int AxisColl_CeilingTooLow = 0x20;
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
    loader::TRCoordinates collisionFeedback;
    Axis orientationAxis;
    irr::s16 yAngle; // external
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
};
