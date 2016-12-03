#include "collisioninfo.h"

#include "core/magic.h"
#include "level/level.h"
#include "laranode.h"

namespace engine
{
    void CollisionInfo::initHeightInfo(const core::ExactTRCoordinates& laraPos, const level::Level& level, int height)
    {
        axisCollisions = AxisColl_None;
        collisionFeedback = {0,0,0};
        orientationAxis = *core::axisFromAngle(yAngle, 45_deg);

        gsl::not_null<const loader::Room*> room = level.m_lara->getCurrentRoom();
        const auto refTestPos = laraPos - core::ExactTRCoordinates(0, height + core::ScalpToHandsHeight, 0);
        gsl::not_null<const loader::Sector*> currentSector = level.findFloorSectorWithClampedPosition(refTestPos.toInexact(), &room);

        current.init(currentSector, refTestPos.toInexact(), level.m_cameraController, laraPos.Y, height);

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
        auto testPos = refTestPos + core::ExactTRCoordinates(frontX, 0, frontZ);
        auto sector = level.findFloorSectorWithClampedPosition(testPos.toInexact(), &room);
        front.init(sector, testPos.toInexact(), level.m_cameraController, laraPos.Y, height);
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
        testPos = refTestPos + core::ExactTRCoordinates(frontLeftX, 0, frontLeftZ);
        sector = level.findFloorSectorWithClampedPosition(testPos.toInexact(), &room);
        frontLeft.init(sector, testPos.toInexact(), level.m_cameraController, laraPos.Y, height);

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
        testPos = refTestPos + core::ExactTRCoordinates(frontRightX, 0, frontRightZ);
        sector = level.findFloorSectorWithClampedPosition(testPos.toInexact(), &room);
        frontRight.init(sector, testPos.toInexact(), level.m_cameraController, laraPos.Y, height);

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
            axisCollisions = AxisColl_FrontForwardBlocked;
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
            axisCollisions = AxisColl_ScalpCollision;
            collisionFeedback.Y = current.ceiling.distance;
        }

        if( front.floor.distance > passableFloorDistanceBottom || front.floor.distance < passableFloorDistanceTop || front.ceiling.distance > neededCeilingDistance )
        {
            axisCollisions = AxisColl_FrontForwardBlocked;
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
            axisCollisions = AxisColl_InsufficientFrontCeilingSpace;
            collisionFeedback = position - laraPos;
            return;
        }

        if( frontLeft.floor.distance > passableFloorDistanceBottom || frontLeft.floor.distance < passableFloorDistanceTop )
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

        if( frontRight.floor.distance > passableFloorDistanceBottom || frontRight.floor.distance < passableFloorDistanceTop )
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

    std::set<const loader::Room*> CollisionInfo::collectNeighborRooms(const core::ExactTRCoordinates& position, int radius, int height, const level::Level& level)
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

    bool CollisionInfo::checkStaticMeshCollisions(const core::ExactTRCoordinates& position, int height, const level::Level& level)
    {
        auto rooms = collectNeighborRooms(position, collisionRadius + 50, height + 50, level);

        gameplay::BoundingBox baseCollisionBox(
                                              position.X - collisionRadius, position.Y - height, position.Z - collisionRadius,
                                              position.X + collisionRadius, position.Y         , position.Z + collisionRadius
                                             );

        hasStaticMeshCollision = false;

        for( const loader::Room* room : rooms )
        {
            for( const loader::RoomStaticMesh& rsm : room->staticMeshes )
            {
                gsl::not_null<const loader::StaticMesh*> sm = level.findStaticMeshById(rsm.meshId);
                if( sm->doNotCollide() )
                    continue;

                gameplay::BoundingBox meshCollisionBox = sm->getCollisionBox(rsm.position, core::Angle{rsm.rotation});

                if( !meshCollisionBox.intersects(baseCollisionBox) )
                    continue;

                auto dx = meshCollisionBox.max.x - baseCollisionBox.min.x;
                if( baseCollisionBox.max.x - meshCollisionBox.min.x < dx )
                    dx = -(baseCollisionBox.max.x - meshCollisionBox.min.x);
                auto dz = meshCollisionBox.max.z - baseCollisionBox.min.z;
                if( baseCollisionBox.max.z - meshCollisionBox.min.z < dz )
                    dz = -(baseCollisionBox.max.z - meshCollisionBox.min.z);

                switch( orientationAxis )
                {
                case core::Axis::PosX:
                    if( dz > collisionRadius || dz < -collisionRadius )
                    {
                        collisionFeedback.X = dx - 1;
                        collisionFeedback.Z = this->position.Z - position.Z;
                        axisCollisions = AxisColl_FrontForwardBlocked;
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
                    if( dx > collisionRadius || dx < -collisionRadius )
                    {
                        collisionFeedback.X = this->position.X - position.X;
                        collisionFeedback.Z = dz - 1;
                        axisCollisions = AxisColl_FrontForwardBlocked;
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
                    if( dx >= 0 || dx < -collisionRadius )
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
                    if( dz > collisionRadius || dz < -collisionRadius )
                    {
                        collisionFeedback.X = dx + 1;
                        collisionFeedback.Z = this->position.Z - position.Z;
                        axisCollisions = AxisColl_FrontForwardBlocked;
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
                    if( dz >= 0 || dz < -collisionRadius )
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
                    if( dx > collisionRadius || dx < -collisionRadius )
                    {
                        collisionFeedback.X = this->position.X - position.X;
                        collisionFeedback.Z = dz + 1;
                        axisCollisions = AxisColl_FrontForwardBlocked;
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
                    if( dx >= 0 || dx < -collisionRadius )
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
