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

inline boost::optional<Axis> axisFromAngle(int16_t angle, uint16_t margin)
{
    BOOST_ASSERT(margin <= 0x2000); // 45 degrees
    if(angle <= 0x0000 + margin && angle >= 0x0000 - margin)
        return Axis::PosZ;
    if(angle <= 0x4000 + margin && angle >= 0x4000 - margin)
        return Axis::PosX;
    if(angle <= -0x4000 + margin && angle >= -0x4000 - margin)
        return Axis::NegX;
    if(angle >= 0x8000 - margin || angle <= -0x8000 + margin)
        return Axis::NegZ;

    return {};
}

inline boost::optional<int16_t> alignRotation(int16_t angle, int16_t margin)
{
    auto axis = axisFromAngle(angle, margin);
    if(!axis)
        return {};

    switch(*axis)
    {
        case Axis::PosZ: return 0x0000;
        case Axis::PosX: return 0x4000;
        case Axis::NegZ: return 0x8000;
        case Axis::NegX: return -0x4000;
    }
}

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
    loader::ExactTRCoordinates collisionFeedback;
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
};
