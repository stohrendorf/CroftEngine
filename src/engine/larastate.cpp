#include "larastate.h"

#include "core/magic.h"
#include "level/level.h"
#include "util/vmath.h"
#include "laracontroller.h"

namespace engine
{
    void LaraState::initHeightInfo(const core::ExactTRCoordinates& laraPos, const level::Level& level, int height)
    {
        axisCollisions = AxisColl_None;
        collisionFeedback = {0,0,0};
        orientationAxis = *core::axisFromAngle(yAngle, 45_deg);

        gsl::not_null<const loader::Room*> room = level.m_lara->getCurrentRoom();
        const auto reachablePos = laraPos - core::ExactTRCoordinates(0, height + core::ScalpToHandsHeight, 0);
        gsl::not_null<const loader::Sector*> currentSector = level.findFloorSectorWithClampedPosition(reachablePos.toInexact(), &room);

        current.init(currentSector, laraPos.toInexact(), level.m_cameraController, height);

        std::tie(floorSlantX, floorSlantZ) = level.getFloorSlantInfo(currentSector, laraPos.toInexact());

        float frontX = 0, frontZ = 0;
        float frontLeftX = 0, frontLeftZ = 0;
        float frontRightX = 0, frontRightZ = 0;

        switch( orientationAxis )
        {
        case core::Axis::PosZ:
            frontX = yAngle.sin() * collisionRadius;
            frontZ = collisionRadius;
            frontLeftZ = collisionRadius;
            frontLeftX = -collisionRadius;
            frontRightX = collisionRadius;
            frontRightZ = collisionRadius;
            break;
        case core::Axis::PosX:
            frontX = collisionRadius;
            frontZ = yAngle.cos() * collisionRadius;
            frontLeftX = collisionRadius;
            frontLeftZ = collisionRadius;
            frontRightX = collisionRadius;
            frontRightZ = -collisionRadius;
            break;
        case core::Axis::NegZ:
            frontX = yAngle.sin() * collisionRadius;
            frontZ = -collisionRadius;
            frontLeftX = collisionRadius;
            frontLeftZ = -collisionRadius;
            frontRightX = -collisionRadius;
            frontRightZ = -collisionRadius;
            break;
        case core::Axis::NegX:
            frontX = -collisionRadius;
            frontZ = yAngle.cos() * collisionRadius;
            frontLeftX = -collisionRadius;
            frontLeftZ = -collisionRadius;
            frontRightX = -collisionRadius;
            frontRightZ = collisionRadius;
            break;
        }

        // Front
        auto checkPos = core::ExactTRCoordinates(frontX, 0, frontZ);
        auto sector = level.findFloorSectorWithClampedPosition((reachablePos + checkPos).toInexact(), &room);
        front.init(sector, (laraPos + checkPos).toInexact(), level.m_cameraController, height);
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
        checkPos = core::ExactTRCoordinates(frontLeftX, 0, frontLeftZ);
        sector = level.findFloorSectorWithClampedPosition((reachablePos + checkPos).toInexact(), &room);
        frontLeft.init(sector, (laraPos + checkPos).toInexact(), level.m_cameraController, height);

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
        checkPos = core::ExactTRCoordinates(frontRightX, 0, frontRightZ);
        sector = level.findFloorSectorWithClampedPosition((reachablePos + checkPos).toInexact(), &room);
        frontRight.init(sector, (laraPos + checkPos).toInexact(), level.m_cameraController, height);

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
            collisionFeedback = position - laraPos;
            axisCollisions = AxisColl_FrontBlocked;
            return;
        }

        if( current.floor.distance <= current.ceiling.distance )
        {
            axisCollisions = AxisColl_InvalidPosition;
            collisionFeedback = position - laraPos;
            return;
        }

        if( current.ceiling.distance >= 0 )
        {
            axisCollisions = AxisColl_FrontCeilingBlocked;
            collisionFeedback.Y = current.ceiling.distance;
        }

        if( front.floor.distance > neededFloorDistanceBottom || front.floor.distance < neededFloorDistanceTop || front.ceiling.distance > neededCeilingDistance )
        {
            axisCollisions = AxisColl_FrontBlocked;
            switch( orientationAxis )
            {
            case core::Axis::PosZ:
            case core::Axis::NegZ:
                collisionFeedback.X = position.X - laraPos.X;
                collisionFeedback.Z = reflectAtSectorBoundary(frontZ + laraPos.Z, laraPos.Z);
                break;
            case core::Axis::PosX:
            case core::Axis::NegX:
                collisionFeedback.X = reflectAtSectorBoundary(frontX + laraPos.X, laraPos.X);
                collisionFeedback.Z = position.Z - laraPos.Z;
                break;
            }
            return;
        }

        if( front.ceiling.distance >= neededCeilingDistance )
        {
            axisCollisions = AxisColl_InsufficientCeilingSpace;
            collisionFeedback = position - laraPos;
            return;
        }

        if( frontLeft.floor.distance > neededFloorDistanceBottom || frontLeft.floor.distance < neededFloorDistanceTop )
        {
            axisCollisions = AxisColl_FrontLeftBlocked;
            switch( orientationAxis )
            {
            case core::Axis::PosZ:
            case core::Axis::NegZ:
                collisionFeedback.X = reflectAtSectorBoundary(frontLeftX + laraPos.X, frontX + laraPos.X);
                break;
            case core::Axis::PosX:
            case core::Axis::NegX:
                collisionFeedback.Z = reflectAtSectorBoundary(frontLeftZ + laraPos.Z, frontZ + laraPos.Z);
                break;
            }
            return;
        }

        if( frontRight.floor.distance > neededFloorDistanceBottom || frontRight.floor.distance < neededFloorDistanceTop )
        {
            axisCollisions = AxisColl_FrontRightBlocked;
            switch( orientationAxis )
            {
            case core::Axis::PosZ:
            case core::Axis::NegZ:
                collisionFeedback.X = reflectAtSectorBoundary(frontRightX + laraPos.X, frontX + laraPos.X);
                break;
            case core::Axis::PosX:
            case core::Axis::NegX:
                collisionFeedback.Z = reflectAtSectorBoundary(frontRightZ + laraPos.Z, frontZ + laraPos.Z);
                break;
            }
        }
    }

    std::set<const loader::Room*> LaraState::collectNeighborRooms(const core::ExactTRCoordinates& position, int radius, int height, const level::Level& level)
    {
        std::set<const loader::Room*> result;
        result.insert(level.m_lara->getCurrentRoom());
        result.insert(level.findRoomForPosition(position + core::ExactTRCoordinates(radius, 0, radius), level.m_lara->getCurrentRoom()));
        result.insert(level.findRoomForPosition(position + core::ExactTRCoordinates(-radius, 0, radius), level.m_lara->getCurrentRoom()));
        result.insert(level.findRoomForPosition(position + core::ExactTRCoordinates(radius, 0, -radius), level.m_lara->getCurrentRoom()));
        result.insert(level.findRoomForPosition(position + core::ExactTRCoordinates(-radius, 0, -radius), level.m_lara->getCurrentRoom()));
        result.insert(level.findRoomForPosition(position + core::ExactTRCoordinates(radius, -height, radius), level.m_lara->getCurrentRoom()));
        result.insert(level.findRoomForPosition(position + core::ExactTRCoordinates(-radius, -height, radius), level.m_lara->getCurrentRoom()));
        result.insert(level.findRoomForPosition(position + core::ExactTRCoordinates(radius, -height, -radius), level.m_lara->getCurrentRoom()));
        result.insert(level.findRoomForPosition(position + core::ExactTRCoordinates(-radius, -height, -radius), level.m_lara->getCurrentRoom()));
        return result;
    }

    bool LaraState::checkStaticMeshCollisions(const core::ExactTRCoordinates& position, int height, const level::Level& level)
    {
        auto rooms = collectNeighborRooms(position, collisionRadius + 50, height + 50, level);

        irr::core::aabbox3di baseCollisionBox(
                                              position.X - collisionRadius, position.Y - height, position.Z - collisionRadius,
                                              position.X + collisionRadius, position.Y + height, position.Z + collisionRadius
                                             );
        baseCollisionBox.repair();

        for( const loader::Room* room : rooms )
        {
            for( const loader::RoomStaticMesh& rsm : room->staticMeshes )
            {
                gsl::not_null<const loader::StaticMesh*> sm = level.findStaticMeshById(rsm.object_id);
                if( sm->doNotCollide() )
                    continue;

                irr::core::aabbox3di meshCollisionBox = sm->getCollisionBox(rsm.position, core::Angle{rsm.rotation});

                if( !meshCollisionBox.intersectsWithBox(baseCollisionBox) )
                    continue;

                auto dx = meshCollisionBox.MaxEdge.X - baseCollisionBox.MinEdge.X;
                if( baseCollisionBox.MaxEdge.X - meshCollisionBox.MinEdge.X < dx )
                    dx = -(baseCollisionBox.MaxEdge.X - meshCollisionBox.MinEdge.X);
                auto dz = meshCollisionBox.MaxEdge.Z - baseCollisionBox.MinEdge.Z;
                if( baseCollisionBox.MaxEdge.Z - meshCollisionBox.MinEdge.Z < dz )
                    dz = -(baseCollisionBox.MaxEdge.Z - meshCollisionBox.MinEdge.Z);

                switch( orientationAxis )
                {
                case core::Axis::PosX:
                    if( dz > collisionRadius || -collisionRadius > dz )
                    {
                        collisionFeedback.X = dx;
                        collisionFeedback.Z = this->position.Z - position.Z;
                        axisCollisions = AxisColl_FrontBlocked;
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    if( dz > 0 && dz <= collisionRadius )
                    {
                        collisionFeedback.X = 0;
                        collisionFeedback.Z = dz;
                        axisCollisions = AxisColl_FrontRightBlocked;
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    if( dz >= 0 || dz < -collisionRadius )
                    {
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    collisionFeedback.X = 0;
                    collisionFeedback.Z = dz;
                    axisCollisions = AxisColl_FrontLeftBlocked;
                    hasStaticMeshCollision = true;
                    return true;
                case core::Axis::PosZ:
                    if( dx > collisionRadius || -collisionRadius > dx )
                    {
                        collisionFeedback.X = this->position.X - position.X;
                        collisionFeedback.Z = dz;
                        axisCollisions = AxisColl_FrontBlocked;
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    if( dx > 0 && dx <= collisionRadius )
                    {
                        collisionFeedback.X = dx;
                        collisionFeedback.Z = 0;
                        axisCollisions = AxisColl_FrontLeftBlocked;
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    if( dx >= 0 || -collisionRadius > dx )
                    {
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    collisionFeedback.X = dx;
                    collisionFeedback.Z = 0;
                    axisCollisions = AxisColl_FrontRightBlocked;
                    hasStaticMeshCollision = true;
                    return true;
                case core::Axis::NegX:
                    if( dz > collisionRadius || -collisionRadius > dz )
                    {
                        collisionFeedback.X = dx;
                        collisionFeedback.Z = this->position.Z - position.Z;
                        axisCollisions = AxisColl_FrontBlocked;
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    if( dz > 0 && dz <= collisionRadius )
                    {
                        collisionFeedback.X = 0;
                        collisionFeedback.Z = dz;
                        axisCollisions = AxisColl_FrontLeftBlocked;
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    if( dz >= 0 || -collisionRadius > dz )
                    {
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    collisionFeedback.X = 0;
                    collisionFeedback.Z = dz;
                    axisCollisions = AxisColl_FrontRightBlocked;
                    hasStaticMeshCollision = true;
                    return true;
                case core::Axis::NegZ:
                    if( dx > collisionRadius || -collisionRadius > dx )
                    {
                        collisionFeedback.X = this->position.X - position.X;
                        collisionFeedback.Z = dz;
                        axisCollisions = AxisColl_FrontBlocked;
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    if( dx > 0 && dx <= collisionRadius )
                    {
                        collisionFeedback.X = dx;
                        collisionFeedback.Z = 0;
                        axisCollisions = AxisColl_FrontRightBlocked;
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    if( dx >= 0 || -collisionRadius > dx )
                    {
                        hasStaticMeshCollision = true;
                        return true;
                    }
                    collisionFeedback.X = dx;
                    collisionFeedback.Z = 0;
                    axisCollisions = AxisColl_FrontLeftBlocked;
                    hasStaticMeshCollision = true;
                    return true;
                }

                hasStaticMeshCollision = true;
                return true;
            }
        }

        return false;
    }
}
