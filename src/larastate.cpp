#include "larastate.h"

#include "util/vmath.h"
#include "loader/level.h"
#include "loader/trcamerascenenodeanimator.h"
#include "core/magic.h"

void LaraState::initHeightInfo(const loader::TRCoordinates& laraPos, const loader::Level& level, int height)
{
    axisCollisions = AxisColl_None;
    collisionFeedback = {0,0,0};
    orientationAxis = *axisFromAngle(yAngle, util::degToAu(45));

    const loader::Room* room = level.m_camera->getCurrentRoom();
    const auto reachablePos = laraPos - loader::TRCoordinates{0, height + core::ScalpToHandsHeight, 0};
    auto currentSector = level.findSectorForPosition(reachablePos, room);
    BOOST_ASSERT(currentSector != nullptr);

    current.init(currentSector, laraPos, level.m_camera, height);

    std::tie(floorSlantX, floorSlantZ) = level.getFloorSlantInfo(currentSector, laraPos);

    int frontX = 0, frontZ = 0;
    int frontLeftX = 0, frontLeftZ = 0;
    int frontRightX = 0, frontRightZ = 0;

    switch( orientationAxis )
    {
    case Axis::PosZ:
        frontX = std::sin(util::auToRad(yAngle)) * collisionRadius;
        frontZ = collisionRadius;
        frontLeftZ = collisionRadius;
        frontLeftX = -collisionRadius;
        frontRightX = collisionRadius;
        frontRightZ = collisionRadius;
        break;
    case Axis::PosX:
        frontX = collisionRadius;
        frontZ = std::cos(util::auToRad(yAngle)) * collisionRadius;
        frontLeftX = collisionRadius;
        frontLeftZ = collisionRadius;
        frontRightX = collisionRadius;
        frontRightZ = -collisionRadius;
        break;
    case Axis::NegZ:
        frontX = std::sin(util::auToRad(yAngle)) * collisionRadius;
        frontZ = -collisionRadius;
        frontLeftX = collisionRadius;
        frontLeftZ = -collisionRadius;
        frontRightX = -collisionRadius;
        frontRightZ = -collisionRadius;
        break;
    case Axis::NegX:
        frontX = -collisionRadius;
        frontZ = std::cos(util::auToRad(yAngle)) * collisionRadius;
        frontLeftX = -collisionRadius;
        frontLeftZ = -collisionRadius;
        frontRightX = -collisionRadius;
        frontRightZ = collisionRadius;
        break;
    }

    // Front
    auto checkPos = loader::TRCoordinates(frontX, 0, frontZ);
    auto sector = level.findSectorForPosition(reachablePos + checkPos, room);
    front.init(sector, laraPos + checkPos, level.m_camera, height);
    if( (frobbelFlags & FrobbelFlag_UnpassableSteepUpslant) != 0 && front.floor.slantClass == SlantClass::Steep && front.floor.distance < 0 )
    {
        front.floor.distance = -32767;
    }
    else if( front.floor.distance > 0
        && (
            ((frobbelFlags & FrobbelFlag_UnwalkableSteepFloor) != 0 && front.floor.slantClass == SlantClass::Steep)
            || ((frobbelFlags & FrobbelFlag_UnwalkableDeadlyFloor) != 0 && front.floor.lastTriggerOrKill != nullptr && loader::extractFDFunction(*front.floor.lastTriggerOrKill) == loader::FDFunction::Death)
        ) )
    {
        front.floor.distance = 2 * loader::QuarterSectorSize;
    }

    // Front left
    checkPos = loader::TRCoordinates(frontLeftX, 0, frontLeftZ);
    sector = level.findSectorForPosition(reachablePos + checkPos, room);
    frontLeft.init(sector, laraPos + checkPos, level.m_camera, height);

    if( (frobbelFlags & FrobbelFlag_UnpassableSteepUpslant) != 0 && frontLeft.floor.slantClass == SlantClass::Steep && frontLeft.floor.distance < 0 )
    {
        frontLeft.floor.distance = -32767;
    }
    else if( frontLeft.floor.distance > 0
        && (
            ((frobbelFlags & FrobbelFlag_UnwalkableSteepFloor) != 0 && frontLeft.floor.slantClass == SlantClass::Steep)
            || ((frobbelFlags & FrobbelFlag_UnwalkableDeadlyFloor) != 0 && frontLeft.floor.lastTriggerOrKill != nullptr && loader::extractFDFunction(*frontLeft.floor.lastTriggerOrKill) == loader::FDFunction::Death)
        ) )
    {
        frontLeft.floor.distance = 2 * loader::QuarterSectorSize;
    }

    // Front right
    checkPos = loader::TRCoordinates(frontRightX, 0, frontRightZ);
    sector = level.findSectorForPosition(reachablePos + checkPos, room);
    frontRight.init(sector, laraPos + checkPos, level.m_camera, height);

    if( (frobbelFlags & FrobbelFlag_UnpassableSteepUpslant) != 0 && frontRight.floor.slantClass == SlantClass::Steep && frontRight.floor.distance < 0 )
    {
        frontRight.floor.distance = -32767;
    }
    else if( frontRight.floor.distance > 0
        && (
            ((frobbelFlags & FrobbelFlag_UnwalkableSteepFloor) != 0 && frontRight.floor.slantClass == SlantClass::Steep)
            || ((frobbelFlags & FrobbelFlag_UnwalkableDeadlyFloor) != 0 && frontRight.floor.lastTriggerOrKill != nullptr && loader::extractFDFunction(*frontRight.floor.lastTriggerOrKill) == loader::FDFunction::Death)
        ) )
    {
        frontRight.floor.distance = 2 * loader::QuarterSectorSize;
    }

    //! @todo check static mesh collisions here

    if( current.floor.distance == -loader::HeightLimit )
    {
        collisionFeedback = position - loader::ExactTRCoordinates(laraPos);
        axisCollisions = AxisColl_InsufficientFrontSpace;
        return;
    }

    if( current.floor.distance <= current.ceiling.distance )
    {
        axisCollisions = AxisColl_CeilingTooLow;
        collisionFeedback = position - loader::ExactTRCoordinates(laraPos);
        return;
    }

    if( current.ceiling.distance >= 0 )
    {
        axisCollisions = AxisColl_HeadInCeiling;
        collisionFeedback.Y = current.ceiling.distance;
    }

    if( front.floor.distance > neededFloorDistanceBottom || front.floor.distance < neededFloorDistanceTop || front.ceiling.distance > neededCeilingDistance )
    {
        axisCollisions = AxisColl_InsufficientFrontSpace;
        switch( orientationAxis )
        {
        case Axis::PosZ:
        case Axis::NegZ:
            collisionFeedback.X = position.X - laraPos.X;
            collisionFeedback.Z = fruityFeedback(frontZ + laraPos.Z, laraPos.Z);
            break;
        case Axis::PosX:
        case Axis::NegX:
            collisionFeedback.X = fruityFeedback(frontX + laraPos.X, laraPos.X);
            collisionFeedback.Z = position.Z - laraPos.Z;
            break;
        }
        return;
    }

    if( front.ceiling.distance >= neededCeilingDistance )
    {
        axisCollisions = AxisColl_BumpHead;
        collisionFeedback = position - loader::ExactTRCoordinates(laraPos);
        return;
    }

    if( frontLeft.floor.distance > neededFloorDistanceBottom || frontLeft.floor.distance < neededFloorDistanceTop )
    {
        axisCollisions = AxisColl_FrontLeftBump;
        switch( orientationAxis )
        {
        case Axis::PosZ:
        case Axis::NegZ:
            collisionFeedback.X = fruityFeedback(frontLeftX + laraPos.X, frontX + laraPos.X);
            break;
        case Axis::PosX:
        case Axis::NegX:
            collisionFeedback.Z = fruityFeedback(frontLeftZ + laraPos.Z, frontZ + laraPos.Z);
            break;
        }
        return;
    }

    if( frontRight.floor.distance > neededFloorDistanceBottom || frontRight.floor.distance < neededFloorDistanceTop )
    {
        axisCollisions = AxisColl_FrontRightBump;
        switch( orientationAxis )
        {
        case Axis::PosZ:
        case Axis::NegZ:
            collisionFeedback.X = fruityFeedback(frontRightX + laraPos.X, frontX + laraPos.X);
            break;
        case Axis::PosX:
        case Axis::NegX:
            collisionFeedback.Z = fruityFeedback(frontRightZ + laraPos.Z, frontZ + laraPos.Z);
            break;
        }
    }
}
