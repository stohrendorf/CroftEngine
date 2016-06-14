#include "larastate.h"

#include "util/vmath.h"
#include "loader/level.h"
#include "loader/trcamerascenenodeanimator.h"
#include "core/magic.h"

void LaraState::initHeightInfo(const loader::TRCoordinates& laraPos, const loader::Level& level, int height)
{
    axisCollisions = AxisColl_None;
    collisionFeedback = {0,0,0};
    orientationAxis = *util::axisFromAngle(yAngle, util::degToAu(45));

    const loader::Room* room = level.m_camera->getCurrentRoom();
    const auto reachablePos = laraPos - loader::TRCoordinates{0, height + core::ScalpToHandsHeight, 0};
    auto currentSector = level.findSectorForPosition(reachablePos, &room);
    BOOST_ASSERT(currentSector != nullptr);

    current.init(currentSector, laraPos, level.m_camera, height);

    std::tie(floorSlantX, floorSlantZ) = level.getFloorSlantInfo(currentSector, laraPos);

    int frontX = 0, frontZ = 0;
    int frontLeftX = 0, frontLeftZ = 0;
    int frontRightX = 0, frontRightZ = 0;

    switch( orientationAxis )
    {
    case util::Axis::PosZ:
        frontX = std::sin(util::auToRad(yAngle)) * collisionRadius;
        frontZ = collisionRadius;
        frontLeftZ = collisionRadius;
        frontLeftX = -collisionRadius;
        frontRightX = collisionRadius;
        frontRightZ = collisionRadius;
        break;
    case util::Axis::PosX:
        frontX = collisionRadius;
        frontZ = std::cos(util::auToRad(yAngle)) * collisionRadius;
        frontLeftX = collisionRadius;
        frontLeftZ = collisionRadius;
        frontRightX = collisionRadius;
        frontRightZ = -collisionRadius;
        break;
    case util::Axis::NegZ:
        frontX = std::sin(util::auToRad(yAngle)) * collisionRadius;
        frontZ = -collisionRadius;
        frontLeftX = collisionRadius;
        frontLeftZ = -collisionRadius;
        frontRightX = -collisionRadius;
        frontRightZ = -collisionRadius;
        break;
    case util::Axis::NegX:
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
    auto sector = level.findSectorForPosition(reachablePos + checkPos, &room);
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
    sector = level.findSectorForPosition(reachablePos + checkPos, &room);
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
    sector = level.findSectorForPosition(reachablePos + checkPos, &room);
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

    checkStaticMeshCollisions(laraPos, height, level);

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
        case util::Axis::PosZ:
        case util::Axis::NegZ:
            collisionFeedback.X = position.X - laraPos.X;
            collisionFeedback.Z = fruityFeedback(frontZ + laraPos.Z, laraPos.Z);
            break;
        case util::Axis::PosX:
        case util::Axis::NegX:
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
        case util::Axis::PosZ:
        case util::Axis::NegZ:
            collisionFeedback.X = fruityFeedback(frontLeftX + laraPos.X, frontX + laraPos.X);
            break;
        case util::Axis::PosX:
        case util::Axis::NegX:
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
        case util::Axis::PosZ:
        case util::Axis::NegZ:
            collisionFeedback.X = fruityFeedback(frontRightX + laraPos.X, frontX + laraPos.X);
            break;
        case util::Axis::PosX:
        case util::Axis::NegX:
            collisionFeedback.Z = fruityFeedback(frontRightZ + laraPos.Z, frontZ + laraPos.Z);
            break;
        }
    }
}

std::set<const loader::Room*> LaraState::collectNeighborRooms(const loader::TRCoordinates& position, int radius, int height, const loader::Level& level)
{
    std::set<const loader::Room*> result;
    result.insert(level.m_camera->getCurrentRoom());
    result.insert(level.findRoomForPosition(position + loader::TRCoordinates(radius, 0, radius), level.m_camera->getCurrentRoom()));
    result.insert(level.findRoomForPosition(position + loader::TRCoordinates(-radius, 0, radius), level.m_camera->getCurrentRoom()));
    result.insert(level.findRoomForPosition(position + loader::TRCoordinates(radius, 0, -radius), level.m_camera->getCurrentRoom()));
    result.insert(level.findRoomForPosition(position + loader::TRCoordinates(-radius, 0, -radius), level.m_camera->getCurrentRoom()));
    result.insert(level.findRoomForPosition(position + loader::TRCoordinates(radius, -height, radius), level.m_camera->getCurrentRoom()));
    result.insert(level.findRoomForPosition(position + loader::TRCoordinates(-radius, -height, radius), level.m_camera->getCurrentRoom()));
    result.insert(level.findRoomForPosition(position + loader::TRCoordinates(radius, -height, -radius), level.m_camera->getCurrentRoom()));
    result.insert(level.findRoomForPosition(position + loader::TRCoordinates(-radius, -height, -radius), level.m_camera->getCurrentRoom()));
    return result;
}

bool LaraState::checkStaticMeshCollisions(const loader::TRCoordinates& position, int height, const loader::Level& level)
{
    auto rooms = collectNeighborRooms(position, collisionRadius + 50, height + 50, level);

    irr::core::aabbox3di baseBB{
        position.X - collisionRadius, position.Y - height, position.Z - collisionRadius,
        position.X + collisionRadius, position.Y + height, position.Z + collisionRadius
    };
    baseBB.repair();

    for(const loader::Room* room : rooms)
    {
        for(const loader::RoomStaticMesh& rsm : room->staticMeshes)
        {
            const loader::StaticMesh* sm = level.findStaticMeshById(rsm.object_id);
            BOOST_ASSERT(sm != nullptr);
            if(sm->doNotCollide())
                continue;

            irr::core::aabbox3di bb = sm->getCollisionBox(rsm.position, rsm.rotation);
            bb.repair();

            if(!bb.intersectsWithBox(baseBB))
                continue;

            auto dx = bb.MaxEdge.X - baseBB.MinEdge.X;
            if(baseBB.MaxEdge.X - bb.MinEdge.X < dx)
                dx = -(baseBB.MaxEdge.X - bb.MinEdge.X);
            auto dz = bb.MaxEdge.X - baseBB.MinEdge.X;
            if(baseBB.MaxEdge.Z - bb.MinEdge.Z < dz)
                dz = -(baseBB.MaxEdge.Z - bb.MinEdge.Z);

            switch(orientationAxis)
            {
                case util::Axis::PosX:
                    if(dz > collisionRadius || -collisionRadius > dz)
                    {
                        collisionFeedback.X = dx;
                        collisionFeedback.Z = this->position.Z - position.Z;
                        axisCollisions = LaraState::AxisColl_InsufficientFrontSpace;
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    if(dz > 0 && dz <= collisionRadius)
                    {
                        collisionFeedback.X = 0;
                        collisionFeedback.Z = dz;
                        axisCollisions = LaraState::AxisColl_FrontRightBump;
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    if(dz >= 0 || -collisionRadius > dz)
                    {
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    collisionFeedback.X = 0;
                    collisionFeedback.Z = dz;
                    axisCollisions = LaraState::AxisColl_FrontLeftBump;
                    hasStaticMeshCollision = true;
                    return true;
                case util::Axis::PosZ:
                    if(dx > collisionRadius || -collisionRadius > dx)
                    {
                        collisionFeedback.X = this->position.X - position.X;
                        collisionFeedback.Z = dz;
                        axisCollisions = LaraState::AxisColl_InsufficientFrontSpace;
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    if(dx > 0 && dx <= collisionRadius)
                    {
                        collisionFeedback.X = dx;
                        collisionFeedback.Z = 0;
                        axisCollisions = LaraState::AxisColl_FrontLeftBump;
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    if(dx >= 0 || -collisionRadius > dx)
                    {
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    collisionFeedback.X = dx;
                    collisionFeedback.Z = 0;
                    axisCollisions = LaraState::AxisColl_FrontRightBump;
                    hasStaticMeshCollision = true;
                    return true;
                case util::Axis::NegX:
                    if(dz > collisionRadius || -collisionRadius > dz)
                    {
                        collisionFeedback.X = dx;
                        collisionFeedback.Z = this->position.Z - position.Z;
                        axisCollisions = LaraState::AxisColl_InsufficientFrontSpace;
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    if(dz > 0 && dz <= collisionRadius)
                    {
                        collisionFeedback.X = 0;
                        collisionFeedback.Z = dz;
                        axisCollisions = LaraState::AxisColl_FrontLeftBump;
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    if(dz >= 0 || -collisionRadius > dz)
                    {
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    collisionFeedback.X = 0;
                    collisionFeedback.Z = dz;
                    axisCollisions = LaraState::AxisColl_FrontRightBump;
                    hasStaticMeshCollision = true;
                    return true;
                case util::Axis::NegZ:
                    if(dx > collisionRadius || -collisionRadius > dx)
                    {
                        collisionFeedback.X = this->position.X - position.X;
                        collisionFeedback.Z = dz;
                        axisCollisions = LaraState::AxisColl_InsufficientFrontSpace;
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    if(dx > 0 && dx <= collisionRadius)
                    {
                        collisionFeedback.X = dx;
                        collisionFeedback.Z = 0;
                        axisCollisions = LaraState::AxisColl_FrontRightBump;
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    if(dx >= 0 || -collisionRadius > dx)
                    {
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    collisionFeedback.X = dx;
                    collisionFeedback.Z = 0;
                    axisCollisions = LaraState::AxisColl_FrontLeftBump;
                    hasStaticMeshCollision = true;
                    return true;
            }

            hasStaticMeshCollision = true;
            return true;
        }
    }

    return false;
}
