#include "collisioninfo.h"

#include "core/magic.h"
#include "level/level.h"
#include "laranode.h"

namespace engine
{
namespace
{
int reflectAtSectorBoundary(int target, int current)
{
    const auto targetSector = target / loader::SectorSize;
    const auto currentSector = current / loader::SectorSize;
    if( targetSector == currentSector )
        return 0;

    const auto targetInSector = target % loader::SectorSize;
    if( currentSector <= targetSector )
        return -(targetInSector + 1);
    return loader::SectorSize - (targetInSector - 1);
}
}

void CollisionInfo::initHeightInfo(const core::TRCoordinates& laraPos, const level::Level& level, int height)
{
    collisionType = AxisColl_None;
    shift = {0, 0, 0};
    facingAxis = *axisFromAngle( facingAngle, 45_deg );

    auto room = level.m_lara->m_state.position.room;
    const auto refTestPos = laraPos - core::TRCoordinates( 0, height + core::ScalpToHandsHeight, 0 );
    auto currentSector = to_not_null( level.findRealFloorSector( refTestPos, to_not_null( &room ) ) );

    mid.init( currentSector, refTestPos, level.m_itemNodes, laraPos.Y, height );

    std::tie( floorSlantX, floorSlantZ ) = level.getFloorSlantInfo( currentSector, laraPos );

    int frontX = 0, frontZ = 0;
    int frontLeftX = 0, frontLeftZ = 0;
    int frontRightX = 0, frontRightZ = 0;

    switch( facingAxis )
    {
        case core::Axis::PosZ:
            frontX = facingAngle.sin() * collisionRadius;
            frontZ = collisionRadius;
            frontLeftZ = collisionRadius;
            frontLeftX = -collisionRadius;
            frontRightX = collisionRadius;
            frontRightZ = collisionRadius;
            break;
        case core::Axis::PosX:
            frontX = collisionRadius;
            frontZ = facingAngle.cos() * collisionRadius;
            frontLeftX = collisionRadius;
            frontLeftZ = collisionRadius;
            frontRightX = collisionRadius;
            frontRightZ = -collisionRadius;
            break;
        case core::Axis::NegZ:
            frontX = facingAngle.sin() * collisionRadius;
            frontZ = -collisionRadius;
            frontLeftX = collisionRadius;
            frontLeftZ = -collisionRadius;
            frontRightX = -collisionRadius;
            frontRightZ = -collisionRadius;
            break;
        case core::Axis::NegX:
            frontX = -collisionRadius;
            frontZ = facingAngle.cos() * collisionRadius;
            frontLeftX = -collisionRadius;
            frontLeftZ = -collisionRadius;
            frontRightX = -collisionRadius;
            frontRightZ = collisionRadius;
            break;
    }

    // Front
    auto testPos = refTestPos + core::TRCoordinates( frontX, 0, frontZ );
    auto sector = to_not_null( level.findRealFloorSector( testPos, to_not_null( &room ) ) );
    front.init( sector, testPos, level.m_itemNodes, laraPos.Y, height );
    if( (policyFlags & SlopesAreWalls) != 0 && front.floor.slantClass == SlantClass::Steep && front.floor.distance < 0 )
    {
        front.floor.distance = -32767; // This is not a typo, it is really -32767
    }
    else if( front.floor.distance > 0
             && (
                     ((policyFlags & SlopesArePits) != 0 && front.floor.slantClass == SlantClass::Steep)
                     || ((policyFlags & LavaIsPit) != 0 && front.floor.lastCommandSequenceOrDeath != nullptr
                         && floordata::FloorDataChunk::extractType( *front.floor.lastCommandSequenceOrDeath )
                            == floordata::FloorDataChunkType::Death)
             ) )
    {
        front.floor.distance = 2 * loader::QuarterSectorSize;
    }

    // Front left
    testPos = refTestPos + core::TRCoordinates( frontLeftX, 0, frontLeftZ );
    sector = to_not_null( level.findRealFloorSector( testPos, to_not_null( &room ) ) );
    frontLeft.init( sector, testPos, level.m_itemNodes, laraPos.Y, height );

    if( (policyFlags & SlopesAreWalls) != 0 && frontLeft.floor.slantClass == SlantClass::Steep
        && frontLeft.floor.distance < 0 )
    {
        frontLeft.floor.distance = -32767; // This is not a typo, it is really -32767
    }
    else if( frontLeft.floor.distance > 0
             && (
                     ((policyFlags & SlopesArePits) != 0 && frontLeft.floor.slantClass == SlantClass::Steep)
                     || ((policyFlags & LavaIsPit) != 0 && frontLeft.floor.lastCommandSequenceOrDeath != nullptr
                         && floordata::FloorDataChunk::extractType( *frontLeft.floor.lastCommandSequenceOrDeath )
                            == floordata::FloorDataChunkType::Death)
             ) )
    {
        frontLeft.floor.distance = 2 * loader::QuarterSectorSize;
    }

    // Front right
    testPos = refTestPos + core::TRCoordinates( frontRightX, 0, frontRightZ );
    sector = to_not_null( level.findRealFloorSector( testPos, to_not_null( &room ) ) );
    frontRight.init( sector, testPos, level.m_itemNodes, laraPos.Y, height );

    if( (policyFlags & SlopesAreWalls) != 0 && frontRight.floor.slantClass == SlantClass::Steep
        && frontRight.floor.distance < 0 )
    {
        frontRight.floor.distance = -32767; // This is not a typo, it is really -32767
    }
    else if( frontRight.floor.distance > 0
             && (
                     ((policyFlags & SlopesArePits) != 0 && frontRight.floor.slantClass == SlantClass::Steep)
                     || ((policyFlags & LavaIsPit) != 0 && frontRight.floor.lastCommandSequenceOrDeath != nullptr
                         && floordata::FloorDataChunk::extractType( *frontRight.floor.lastCommandSequenceOrDeath )
                            == floordata::FloorDataChunkType::Death)
             ) )
    {
        frontRight.floor.distance = 2 * loader::QuarterSectorSize;
    }

    checkStaticMeshCollisions( laraPos, height, level );

    if( mid.floor.distance == -loader::HeightLimit )
    {
        shift = oldPosition - laraPos;
        collisionType = AxisColl_Front;
        return;
    }

    if( mid.floor.distance <= mid.ceiling.distance )
    {
        collisionType = AxisColl_TopFront;
        shift = oldPosition - laraPos;
        return;
    }

    if( mid.ceiling.distance >= 0 )
    {
        collisionType = AxisColl_Top;
        shift.Y = mid.ceiling.distance;
    }

    if( front.floor.distance > badPositiveDistance || front.floor.distance < badNegativeDistance
        || front.ceiling.distance > badCeilingDistance )
    {
        collisionType = AxisColl_Front;
        switch( facingAxis )
        {
            case core::Axis::PosZ:
            case core::Axis::NegZ:
                shift.X = oldPosition.X - laraPos.X;
                shift.Z = reflectAtSectorBoundary( frontZ + laraPos.Z, laraPos.Z );
                break;
            case core::Axis::PosX:
            case core::Axis::NegX:
                shift.X = reflectAtSectorBoundary( frontX + laraPos.X, laraPos.X );
                shift.Z = oldPosition.Z - laraPos.Z;
                break;
        }
        return;
    }

    if( front.ceiling.distance >= badCeilingDistance )
    {
        collisionType = AxisColl_TopBottom;
        shift = oldPosition - laraPos;
        return;
    }

    if( frontLeft.floor.distance > badPositiveDistance || frontLeft.floor.distance < badNegativeDistance )
    {
        collisionType = AxisColl_Left;
        switch( facingAxis )
        {
            case core::Axis::PosZ:
            case core::Axis::NegZ:
                shift.X = reflectAtSectorBoundary( frontLeftX + laraPos.X, frontX + laraPos.X );
                break;
            case core::Axis::PosX:
            case core::Axis::NegX:
                shift.Z = reflectAtSectorBoundary( frontLeftZ + laraPos.Z, frontZ + laraPos.Z );
                break;
        }
        return;
    }

    if( frontRight.floor.distance > badPositiveDistance || frontRight.floor.distance < badNegativeDistance )
    {
        collisionType = AxisColl_Right;
        switch( facingAxis )
        {
            case core::Axis::PosZ:
            case core::Axis::NegZ:
                shift.X = reflectAtSectorBoundary( frontRightX + laraPos.X, frontX + laraPos.X );
                break;
            case core::Axis::PosX:
            case core::Axis::NegX:
                shift.Z = reflectAtSectorBoundary( frontRightZ + laraPos.Z, frontZ + laraPos.Z );
                break;
        }
    }
}

std::set<gsl::not_null<const loader::Room*>>
CollisionInfo::collectTouchingRooms(const core::TRCoordinates& position, int radius, int height,
                                    const level::Level& level)
{
    std::set<gsl::not_null<const loader::Room*>> result;
    auto room = level.m_lara->m_state.position.room;
    result.emplace( room );
    result.emplace( level.findRoomForPosition( position + core::TRCoordinates( radius, 0, radius ), room ) );
    result.emplace( level.findRoomForPosition( position + core::TRCoordinates( -radius, 0, radius ), room ) );
    result.emplace( level.findRoomForPosition( position + core::TRCoordinates( radius, 0, -radius ), room ) );
    result.emplace( level.findRoomForPosition( position + core::TRCoordinates( -radius, 0, -radius ), room ) );
    result.emplace( level.findRoomForPosition( position + core::TRCoordinates( radius, -height, radius ), room ) );
    result.emplace( level.findRoomForPosition( position + core::TRCoordinates( -radius, -height, radius ), room ) );
    result.emplace( level.findRoomForPosition( position + core::TRCoordinates( radius, -height, -radius ), room ) );
    result.emplace( level.findRoomForPosition( position + core::TRCoordinates( -radius, -height, -radius ), room ) );
    return result;
}

bool
CollisionInfo::checkStaticMeshCollisions(const core::TRCoordinates& position, int height, const level::Level& level)
{
    const auto rooms = collectTouchingRooms( position, collisionRadius + 50, height + 50, level );

    const core::BoundingBox inBox{
            {position.X - collisionRadius, position.Y - height, position.Z - collisionRadius},
            {position.X + collisionRadius, position.Y,          position.Z + collisionRadius}
    };

    hasStaticMeshCollision = false;

    for( const auto& room : rooms )
    {
        for( const loader::RoomStaticMesh& rsm : room->staticMeshes )
        {
            auto sm = to_not_null( level.findStaticMeshById( rsm.meshId ) );
            if( sm->doNotCollide() )
                continue;

            const auto meshBox = sm->getCollisionBox( rsm.position, core::Angle{rsm.rotation} );

            if( !meshBox.intersects( inBox ) )
                continue;

            int dx, dz;
            {
                auto left = inBox.max.X - meshBox.min.X;
                auto right = meshBox.max.X - inBox.min.X;
                if( left < right )
                    dx = -left;
                else
                    dx = right;

                left = inBox.max.Z - meshBox.min.Z;
                right = meshBox.max.Z - inBox.min.Z;
                if( left < right )
                    dz = -left;
                else
                    dz = right;
            }

            switch( facingAxis )
            {
                case core::Axis::PosX:
                    if( std::abs( dz ) > collisionRadius )
                    {
                        shift.X = dx;
                        shift.Z = this->oldPosition.Z - position.Z;
                        collisionType = AxisColl_Front;
                    }
                    else if( dz > 0 && dz <= collisionRadius )
                    {
                        shift.X = 0;
                        shift.Z = dz;
                        collisionType = AxisColl_Right;
                    }
                    else if( dz < 0 && dz >= -collisionRadius )
                    {
                        shift.X = 0;
                        shift.Z = dz;
                        collisionType = AxisColl_Left;
                    }
                    break;
                case core::Axis::PosZ:
                    if( std::abs( dx ) > collisionRadius )
                    {
                        shift.X = this->oldPosition.X - position.X;
                        shift.Z = dz;
                        collisionType = AxisColl_Front;
                    }
                    else if( dx > 0 && dx <= collisionRadius )
                    {
                        shift.X = dx;
                        shift.Z = 0;
                        collisionType = AxisColl_Left;
                    }
                    else if( dx < 0 && dx >= -collisionRadius )
                    {
                        shift.X = dx;
                        shift.Z = 0;
                        collisionType = AxisColl_Right;
                    }
                    break;
                case core::Axis::NegX:
                    if( std::abs( dz ) > collisionRadius )
                    {
                        shift.X = dx;
                        shift.Z = this->oldPosition.Z - position.Z;
                        collisionType = AxisColl_Front;
                    }
                    else if( dz > 0 && dz <= collisionRadius )
                    {
                        shift.X = 0;
                        shift.Z = dz;
                        collisionType = AxisColl_Left;
                    }
                    else if( dz < 0 && dz >= -collisionRadius )
                    {
                        shift.X = 0;
                        shift.Z = dz;
                        collisionType = AxisColl_Right;
                    }
                    break;
                case core::Axis::NegZ:
                    if( std::abs( dx ) > collisionRadius )
                    {
                        shift.X = this->oldPosition.X - position.X;
                        shift.Z = dz + 1;
                        collisionType = AxisColl_Front;
                    }
                    else if( dx > 0 && dx <= collisionRadius )
                    {
                        shift.X = dx;
                        shift.Z = 0;
                        collisionType = AxisColl_Right;
                    }
                    else if( dx < 0 && dx >= -collisionRadius )
                    {
                        shift.X = dx;
                        shift.Z = 0;
                        collisionType = AxisColl_Left;
                    }
                    break;
            }

            hasStaticMeshCollision = true;
            return true;
        }
    }

    return false;
}
}
