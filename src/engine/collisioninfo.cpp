#include "collisioninfo.h"

#include "core/magic.h"
#include "level/level.h"
#include "laranode.h"

namespace engine
{
namespace
{
core::Length reflectAtSectorBoundary(const core::Length target, const core::Length current)
{
    const auto targetSector = target / core::SectorSize;
    const auto currentSector = current / core::SectorSize;
    if( targetSector == currentSector )
        return 0_len;

    const auto targetInSector = target % core::SectorSize;
    if( currentSector <= targetSector )
        return -(targetInSector + 1_len);
    return core::SectorSize - (targetInSector - 1_len);
}
}

void CollisionInfo::initHeightInfo(const core::TRVec& laraPos, const level::Level& level, const core::Length& height)
{
    collisionType = AxisColl::None;
    shift = core::TRVec{};
    facingAxis = *axisFromAngle( facingAngle, 45_deg );

    auto room = level.m_lara->m_state.position.room;
    const auto refTestPos = laraPos - core::TRVec( 0_len, height + core::ScalpToHandsHeight, 0_len );
    const auto currentSector = level::Level::findRealFloorSector( refTestPos, &room );

    mid.init( currentSector, refTestPos, level.m_itemNodes, laraPos.Y, height );

    std::tie( floorSlantX, floorSlantZ ) = level.getFloorSlantInfo( currentSector, laraPos );

    core::Length frontX = 0_len, frontZ = 0_len;
    core::Length frontLeftX = 0_len, frontLeftZ = 0_len;
    core::Length frontRightX = 0_len, frontRightZ = 0_len;

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
    auto testPos = refTestPos + core::TRVec( frontX, 0_len, frontZ );
    auto sector = level::Level::findRealFloorSector( testPos, &room );
    front.init( sector, testPos, level.m_itemNodes, laraPos.Y, height );
    if( policyFlags.is_set( PolicyFlags::SlopesAreWalls ) && front.floor.slantClass == SlantClass::Steep
        && front.floor.y < 0_len )
    {
        front.floor.y = -32767_len; // This is not a typo, it is really -32767
    }
    else if( front.floor.y > 0_len
             && (
                     (policyFlags.is_set( PolicyFlags::SlopesArePits ) && front.floor.slantClass == SlantClass::Steep)
                     || (policyFlags.is_set( PolicyFlags::LavaIsPit )
                         && front.floor.lastCommandSequenceOrDeath != nullptr
                         && floordata::FloorDataChunk::extractType( *front.floor.lastCommandSequenceOrDeath )
                            == floordata::FloorDataChunkType::Death)
             ) )
    {
        front.floor.y = 2 * core::QuarterSectorSize;
    }

    // Front left
    testPos = refTestPos + core::TRVec( frontLeftX, 0_len, frontLeftZ );
    sector = level::Level::findRealFloorSector( testPos, &room );
    frontLeft.init( sector, testPos, level.m_itemNodes, laraPos.Y, height );

    if( policyFlags.is_set( PolicyFlags::SlopesAreWalls ) && frontLeft.floor.slantClass == SlantClass::Steep
        && frontLeft.floor.y < 0_len )
    {
        frontLeft.floor.y = -32767_len; // This is not a typo, it is really -32767
    }
    else if( frontLeft.floor.y > 0_len
             && (
                     (policyFlags.is_set( PolicyFlags::SlopesArePits )
                      && frontLeft.floor.slantClass == SlantClass::Steep)
                     || (policyFlags.is_set( PolicyFlags::LavaIsPit )
                         && frontLeft.floor.lastCommandSequenceOrDeath != nullptr
                         && floordata::FloorDataChunk::extractType( *frontLeft.floor.lastCommandSequenceOrDeath )
                            == floordata::FloorDataChunkType::Death)
             ) )
    {
        frontLeft.floor.y = 2 * core::QuarterSectorSize;
    }

    // Front right
    testPos = refTestPos + core::TRVec( frontRightX, 0_len, frontRightZ );
    sector = level::Level::findRealFloorSector( testPos, &room );
    frontRight.init( sector, testPos, level.m_itemNodes, laraPos.Y, height );

    if( policyFlags.is_set( PolicyFlags::SlopesAreWalls ) && frontRight.floor.slantClass == SlantClass::Steep
        && frontRight.floor.y < 0_len )
    {
        frontRight.floor.y = -32767_len; // This is not a typo, it is really -32767
    }
    else if( frontRight.floor.y > 0_len
             && (
                     (policyFlags.is_set( PolicyFlags::SlopesArePits )
                      && frontRight.floor.slantClass == SlantClass::Steep)
                     || (policyFlags.is_set( PolicyFlags::LavaIsPit )
                         && frontRight.floor.lastCommandSequenceOrDeath != nullptr
                         && floordata::FloorDataChunk::extractType( *frontRight.floor.lastCommandSequenceOrDeath )
                            == floordata::FloorDataChunkType::Death)
             ) )
    {
        frontRight.floor.y = 2 * core::QuarterSectorSize;
    }

    checkStaticMeshCollisions( laraPos, height, level );

    if( mid.floor.y == -core::HeightLimit )
    {
        shift = oldPosition - laraPos;
        collisionType = AxisColl::Front;
        return;
    }

    if( mid.floor.y <= mid.ceiling.y )
    {
        collisionType = AxisColl::TopFront;
        shift = oldPosition - laraPos;
        return;
    }

    if( mid.ceiling.y >= 0_len )
    {
        collisionType = AxisColl::Top;
        shift.Y = mid.ceiling.y;
    }

    if( front.floor.y > badPositiveDistance || front.floor.y < badNegativeDistance
        || front.ceiling.y > badCeilingDistance )
    {
        collisionType = AxisColl::Front;
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

    if( front.ceiling.y >= badCeilingDistance )
    {
        collisionType = AxisColl::TopBottom;
        shift = oldPosition - laraPos;
        return;
    }

    if( frontLeft.floor.y > badPositiveDistance || frontLeft.floor.y < badNegativeDistance )
    {
        collisionType = AxisColl::Left;
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

    if( frontRight.floor.y > badPositiveDistance || frontRight.floor.y < badNegativeDistance )
    {
        collisionType = AxisColl::Right;
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
CollisionInfo::collectTouchingRooms(const core::TRVec& position, const core::Length& radius, const core::Length& height,
                                    const level::Level& level)
{
    std::set<gsl::not_null<const loader::Room*>> result;
    auto room = level.m_lara->m_state.position.room;
    result.emplace( room );
    result.emplace( level.findRoomForPosition( position + core::TRVec( radius, 0_len, radius ), room ) );
    result.emplace( level.findRoomForPosition( position + core::TRVec( -radius, 0_len, radius ), room ) );
    result.emplace( level.findRoomForPosition( position + core::TRVec( radius, 0_len, -radius ), room ) );
    result.emplace( level.findRoomForPosition( position + core::TRVec( -radius, 0_len, -radius ), room ) );
    result.emplace( level.findRoomForPosition( position + core::TRVec( radius, -height, radius ), room ) );
    result.emplace( level.findRoomForPosition( position + core::TRVec( -radius, -height, radius ), room ) );
    result.emplace( level.findRoomForPosition( position + core::TRVec( radius, -height, -radius ), room ) );
    result.emplace( level.findRoomForPosition( position + core::TRVec( -radius, -height, -radius ), room ) );
    return result;
}

bool
CollisionInfo::checkStaticMeshCollisions(const core::TRVec& position, const core::Length& height, const level::Level& level)
{
    const auto rooms = collectTouchingRooms( position, collisionRadius + 50_len, height + 50_len, level );

    const core::BoundingBox inBox{
            {position.X - collisionRadius, position.Y - height, position.Z - collisionRadius},
            {position.X + collisionRadius, position.Y,          position.Z + collisionRadius}
    };

    hasStaticMeshCollision = false;

    for( const auto& room : rooms )
    {
        for( const loader::RoomStaticMesh& rsm : room->staticMeshes )
        {
            const auto sm = level.findStaticMeshById( rsm.meshId );
            if( sm->doNotCollide() )
                continue;

            const auto meshBox = sm->getCollisionBox( rsm.position, core::Angle{rsm.rotation} );

            if( !meshBox.intersects( inBox ) )
                continue;

            core::Length dx = 0_len, dz = 0_len;
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
                    if( abs( dz ) > collisionRadius )
                    {
                        shift.X = dx;
                        shift.Z = this->oldPosition.Z - position.Z;
                        collisionType = AxisColl::Front;
                    }
                    else if( dz > 0_len && dz <= collisionRadius )
                    {
                        shift.X = 0_len;
                        shift.Z = dz;
                        collisionType = AxisColl::Right;
                    }
                    else if( dz < 0_len && dz >= -collisionRadius )
                    {
                        shift.X = 0_len;
                        shift.Z = dz;
                        collisionType = AxisColl::Left;
                    }
                    break;
                case core::Axis::PosZ:
                    if( abs( dx ) > collisionRadius )
                    {
                        shift.X = this->oldPosition.X - position.X;
                        shift.Z = dz;
                        collisionType = AxisColl::Front;
                    }
                    else if( dx > 0_len && dx <= collisionRadius )
                    {
                        shift.X = dx;
                        shift.Z = 0_len;
                        collisionType = AxisColl::Left;
                    }
                    else if( dx < 0_len && dx >= -collisionRadius )
                    {
                        shift.X = dx;
                        shift.Z = 0_len;
                        collisionType = AxisColl::Right;
                    }
                    break;
                case core::Axis::NegX:
                    if( abs( dz ) > collisionRadius )
                    {
                        shift.X = dx;
                        shift.Z = this->oldPosition.Z - position.Z;
                        collisionType = AxisColl::Front;
                    }
                    else if( dz > 0_len && dz <= collisionRadius )
                    {
                        shift.X = 0_len;
                        shift.Z = dz;
                        collisionType = AxisColl::Left;
                    }
                    else if( dz < 0_len && dz >= -collisionRadius )
                    {
                        shift.X = 0_len;
                        shift.Z = dz;
                        collisionType = AxisColl::Right;
                    }
                    break;
                case core::Axis::NegZ:
                    if( abs( dx ) > collisionRadius )
                    {
                        shift.X = this->oldPosition.X - position.X;
                        shift.Z = dz + 1_len;
                        collisionType = AxisColl::Front;
                    }
                    else if( dx > 0_len && dx <= collisionRadius )
                    {
                        shift.X = dx;
                        shift.Z = 0_len;
                        collisionType = AxisColl::Right;
                    }
                    else if( dx < 0_len && dx >= -collisionRadius )
                    {
                        shift.X = dx;
                        shift.Z = 0_len;
                        collisionType = AxisColl::Left;
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
