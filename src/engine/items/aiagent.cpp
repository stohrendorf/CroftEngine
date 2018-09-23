#include "aiagent.h"
#include "level/level.h"
#include "engine/laranode.h"

#include <boost/range/adaptors.hpp>

namespace engine
{
namespace items
{
core::Angle AIAgent::rotateTowardsTarget(core::Angle maxRotationSpeed)
{
    if( m_state.speed == 0 || maxRotationSpeed == 0_au )
    {
        return 0_au;
    }

    const auto dx = m_state.creatureInfo->target.X - m_state.position.position.X;
    const auto dz = m_state.creatureInfo->target.Z - m_state.position.position.Z;
    auto turnAngle = core::Angle::fromAtan( dx, dz ) - m_state.rotation.Y;
    if( turnAngle < -90_deg || turnAngle > 90_deg )
    {
        // the target is behind the current item, so we need a U-turn
        const auto relativeSpeed = m_state.speed * (+90_deg).toAU() / maxRotationSpeed.toAU();
        if( util::square( dx ) + util::square( dz ) < util::square( relativeSpeed ) )
        {
            maxRotationSpeed /= 2;
        }
    }

    turnAngle = util::clamp( turnAngle, -maxRotationSpeed, maxRotationSpeed );

    m_state.rotation.Y += turnAngle;
    return turnAngle;
}

bool AIAgent::isPositionOutOfReach(const core::TRVec& testPosition,
                                   const int currentBoxFloor,
                                   const int nextBoxFloor,
                                   const ai::LotInfo& lotInfo) const
{
    const auto sectorBox = getLevel().findRealFloorSector( testPosition, m_state.position.room )->box;
    if( sectorBox == nullptr )
        return true;

    if( lotInfo.block_mask & sectorBox->overlap_index )
        return true;

    const auto stepHeight = currentBoxFloor - sectorBox->floor;

    if( stepHeight > lotInfo.step || stepHeight < lotInfo.drop )
        return true;

    if( stepHeight < -lotInfo.step && sectorBox->floor > nextBoxFloor )
        return true;

    return lotInfo.fly != 0 && testPosition.Y > lotInfo.fly + sectorBox->floor;
}

bool AIAgent::anyMovingEnabledItemInReach() const
{
    for( const auto& item : getLevel().m_itemNodes | boost::adaptors::map_values )
    {
        if( !item->m_isActive || item.get().get() == this || item.get() == getLevel().m_lara )
            continue;

        if( item->m_state.triggerState == TriggerState::Active
            && item->m_state.speed != 0
            && item->m_state.position.position.distanceTo( m_state.position.position ) < m_collisionRadius )
        {
            return true;
        }
    }
    return false;
}

bool AIAgent::animateCreature(const core::Angle angle, core::Angle tilt)
{
    const auto creatureInfo = m_state.creatureInfo;
    if( creatureInfo == nullptr )
        return false;

    const auto& lotInfo = creatureInfo->lot;

    const auto oldPosition = m_state.position.position;

    const auto boxFloor = m_state.box_number->floor;
    const auto zoneRef = loader::Box::getZoneRef( getLevel().roomsAreSwapped, creatureInfo->lot.fly,
                                                  creatureInfo->lot.step );
    ModelItemNode::update();
    if( m_state.triggerState == TriggerState::Deactivated )
    {
        m_state.health = -16384;
        m_state.collidable = false;
        m_state.creatureInfo.reset();
        deactivate();
        return false;
    }

    const auto bbox = getSkeleton()->getBoundingBox( m_state );
    const auto bboxMinY = m_state.position.position.Y + bbox.minY;

    auto room = m_state.position.room;
    auto sector = gsl::make_not_null( getLevel().findRealFloorSector(
            m_state.position.position + core::TRVec{0, bbox.minY, 0},
            gsl::make_not_null( &room ) ) );
    Expects( sector->box != nullptr );
    auto currentFloor = sector->box->floor;

    int nextFloor;
    if( lotInfo.nodes.find( sector->box )->second.exit_box == nullptr )
    {
        nextFloor = currentFloor;
    }
    else
    {
        nextFloor = lotInfo.nodes.find( sector->box )->second.exit_box->floor;
    }

    if( sector->box == nullptr
        || m_state.box_number->*zoneRef != sector->box->*zoneRef
        || boxFloor - currentFloor > lotInfo.step
        || boxFloor - currentFloor < lotInfo.drop )
    {
        const auto newSectorX = m_state.position.position.X / loader::SectorSize;
        const auto newSectorZ = m_state.position.position.Z / loader::SectorSize;

        const auto oldSectorX = oldPosition.X / loader::SectorSize;
        const auto oldSectorZ = oldPosition.Z / loader::SectorSize;

        if( newSectorX < oldSectorX )
            m_state.position.position.X = oldPosition.X & ~0x3ff;
        else if( newSectorX > oldSectorX )
            m_state.position.position.X = oldPosition.X | 0x3ff;

        if( newSectorZ < oldSectorZ )
            m_state.position.position.Z = oldPosition.Z & ~0x3ff;
        else if( newSectorZ > oldSectorZ )
            m_state.position.position.Z = oldPosition.Z | 0x3ff;

        sector = gsl::make_not_null( getLevel().findRealFloorSector(
                core::TRVec{m_state.position.position.X, bboxMinY, m_state.position.position.Z},
                gsl::make_not_null( &room ) ) );

        currentFloor = sector->box->floor;

        if( lotInfo.nodes.find( sector->box )->second.exit_box == nullptr )
        {
            nextFloor = sector->box->floor;
        }
        else
        {
            nextFloor = lotInfo.nodes.find( sector->box )->second.exit_box->floor;
        }
    }

    const auto basePosX = m_state.position.position.X;
    const auto basePosZ = m_state.position.position.Z;

    const auto inSectorX = basePosX % loader::SectorSize;
    const auto inSectorZ = basePosZ % loader::SectorSize;

    sol::table objectInfo = getLevel().m_scriptEngine["getObjectInfo"].call( m_state.object_number );
    const int radius = objectInfo["radius"];

    int moveX = 0;
    int moveZ = 0;

    if( radius > inSectorZ )
    {
        if( isPositionOutOfReach( core::TRVec{basePosX, bboxMinY, basePosZ - radius}, currentFloor,
                                  nextFloor,
                                  lotInfo ) )
        {
            moveZ = radius - inSectorZ;
        }

        if( radius > inSectorX )
        {
            if( isPositionOutOfReach( core::TRVec{basePosX - radius, bboxMinY, basePosZ}, currentFloor,
                                      nextFloor, lotInfo ) )
            {
                moveX = radius - inSectorX;
            }
            else if( moveZ == 0
                     && isPositionOutOfReach(
                             core::TRVec{basePosX - radius, bboxMinY, basePosZ - radius},
                             currentFloor, nextFloor, lotInfo ) )
            {
                if( m_state.rotation.Y > -135_deg && m_state.rotation.Y < 45_deg )
                    moveZ = radius - inSectorZ;
                else
                    moveX = radius - inSectorX;
            }
        }
        else if( loader::SectorSize - radius < inSectorX )
        {
            if( isPositionOutOfReach( core::TRVec{radius + basePosX, bboxMinY, basePosZ}, currentFloor,
                                      nextFloor, lotInfo ) )
            {
                moveX = loader::SectorSize - radius - inSectorX;
            }
            else if( moveZ == 0
                     && isPositionOutOfReach(
                             core::TRVec{radius + basePosX, bboxMinY, basePosZ - radius},
                             currentFloor, nextFloor, lotInfo ) )
            {
                if( m_state.rotation.Y > -45_deg && m_state.rotation.Y < 135_deg )
                {
                    moveZ = radius - inSectorZ;
                }
                else
                {
                    moveX = loader::SectorSize - radius - inSectorX;
                }
            }
        }
    }
    else if( loader::SectorSize - radius < inSectorZ )
    {
        if( isPositionOutOfReach( core::TRVec{basePosX, bboxMinY, basePosZ + radius}, currentFloor,
                                  nextFloor,
                                  lotInfo ) )
        {
            moveZ = loader::SectorSize - radius - inSectorZ;
        }

        if( radius > inSectorX )
        {
            if( isPositionOutOfReach( core::TRVec{basePosX - radius, bboxMinY, basePosZ}, currentFloor,
                                      nextFloor, lotInfo ) )
            {
                moveX = radius - inSectorX;
            }
            else if( moveZ == 0
                     && isPositionOutOfReach(
                             core::TRVec{basePosX - radius, bboxMinY, basePosZ + radius},
                             currentFloor, nextFloor, lotInfo ) )
            {
                if( m_state.rotation.Y < 135_deg && m_state.rotation.Y > -45_deg )
                {
                    moveX = radius - inSectorX;
                }
                else
                {
                    moveZ = loader::SectorSize - radius - inSectorZ;
                }
            }
        }
        else if( loader::SectorSize - radius < inSectorX )
        {
            if( isPositionOutOfReach( core::TRVec{radius + basePosX, bboxMinY, basePosZ}, currentFloor,
                                      nextFloor, lotInfo ) )
            {
                moveX = loader::SectorSize - radius - inSectorX;
            }
            else if( moveZ == 0
                     && isPositionOutOfReach(
                             core::TRVec{radius + basePosX, bboxMinY, basePosZ + radius},
                             currentFloor, nextFloor, lotInfo ) )
            {
                if( m_state.rotation.Y < 45_deg && m_state.rotation.Y > -135_deg )
                {
                    moveX = loader::SectorSize - radius - inSectorX;
                }
                else
                {
                    moveZ = loader::SectorSize - radius - inSectorZ;
                }
            }
        }
    }
    else if( radius > inSectorX )
    {
        if( isPositionOutOfReach( core::TRVec{basePosX - radius, bboxMinY, basePosZ}, currentFloor,
                                  nextFloor, lotInfo ) )
        {
            moveX = radius - inSectorX;
        }
    }
    else if( inSectorX > loader::SectorSize - radius )
    {
        if( isPositionOutOfReach( core::TRVec{basePosX + radius, bboxMinY, basePosZ}, currentFloor,
                                  nextFloor, lotInfo ) )
        {
            moveX = loader::SectorSize - radius - inSectorX;
        }
    }

    m_state.position.position.X += moveX;
    m_state.position.position.Z += moveZ;

    if( moveX != 0 || moveZ != 0 )
    {
        sector = gsl::make_not_null( getLevel().findRealFloorSector(
                core::TRVec{m_state.position.position.X, bboxMinY, m_state.position.position.Z},
                gsl::make_not_null( &room ) ) );

        m_state.rotation.Y += angle;
        m_state.rotation.Z += util::clamp(
                core::Angle{gsl::narrow<int16_t>( 8 * tilt.toAU() )} - m_state.rotation.Z,
                -3_deg, +3_deg );
    }

    if( anyMovingEnabledItemInReach() )
    {
        m_state.position.position = oldPosition;
        return true;
    }

    if( lotInfo.fly != 0 )
    {
        auto moveY = util::clamp<int>( creatureInfo->target.Y - m_state.position.position.Y,
                                       -lotInfo.fly,
                                       lotInfo.fly );

        currentFloor = HeightInfo::fromFloor( sector,
                                              core::TRVec{
                                                      m_state.position.position.X,
                                                      bboxMinY,
                                                      m_state.position.position.Z
                                              },
                                              getLevel().m_itemNodes ).y;

        if( m_state.position.position.Y + moveY > currentFloor )
        {
            // fly target is below floor

            if( m_state.position.position.Y > currentFloor )
            {
                // we're already below the floor, so fix it
                m_state.position.position.X = oldPosition.X;
                m_state.position.position.Z = oldPosition.Z;
                moveY = -lotInfo.fly;
            }
            else
            {
                m_state.position.position.Y = currentFloor;
                moveY = 0;
            }
        }
        else
        {
            const auto ceiling = HeightInfo::fromCeiling( sector,
                                                          core::TRVec{
                                                                  m_state.position.position.X,
                                                                  bboxMinY,
                                                                  m_state.position.position.Z
                                                          },
                                                          getLevel().m_itemNodes ).y;

            const auto y = m_state.object_number == engine::TR1ItemId::CrocodileInWater ? 0 : bbox.minY;

            if( m_state.position.position.Y + y + moveY < ceiling )
            {
                if( m_state.position.position.Y + y < ceiling )
                {
                    m_state.position.position.X = oldPosition.X;
                    m_state.position.position.Z = oldPosition.Z;
                    moveY = lotInfo.fly;
                }
                else
                    moveY = 0;
            }
        }

        m_state.position.position.Y += moveY;
        sector = gsl::make_not_null( getLevel().findRealFloorSector(
                core::TRVec{m_state.position.position.X, bboxMinY, m_state.position.position.Z},
                gsl::make_not_null( &room ) ) );
        m_state.floor = HeightInfo::fromFloor( sector,
                                               core::TRVec{
                                                       m_state.position.position.X,
                                                       bboxMinY,
                                                       m_state.position.position.Z
                                               },
                                               getLevel().m_itemNodes ).y;

        core::Angle yaw{0};
        if( m_state.speed != 0 )
            yaw = core::Angle::fromAtan( -moveY, m_state.speed );

        if( yaw < m_state.rotation.X - 1_deg )
            m_state.rotation.X -= 1_deg;
        else if( yaw > m_state.rotation.X + 1_deg )
            m_state.rotation.X += 1_deg;
        else
            m_state.rotation.X = yaw;

        setCurrentRoom( room );

        return true;
    }

    if( m_state.position.position.Y > m_state.floor )
    {
        m_state.position.position.Y = m_state.floor;
    }
    else if( m_state.floor - m_state.position.position.Y > 64 )
    {
        m_state.position.position.Y += 64;
    }
    else if( m_state.position.position.Y < m_state.floor )
    {
        m_state.position.position.Y = m_state.floor;
    }

    m_state.rotation.X = 0_au;

    sector = gsl::make_not_null(
            getLevel().findRealFloorSector( m_state.position.position, gsl::make_not_null( &room ) ) );
    m_state.floor = HeightInfo::fromFloor( sector, m_state.position.position, getLevel().m_itemNodes ).y;

    setCurrentRoom( room );

    return true;
}

AIAgent::AIAgent(const gsl::not_null<level::Level*>& level,
                 const gsl::not_null<const loader::Room*>& room,
                 const loader::Item& item,
                 const loader::SkeletalModelType& animatedModel)
        : ModelItemNode( level, room, item, true, animatedModel )
        , m_collisionRadius{level->m_scriptEngine["getObjectInfo"].call<sol::table>( m_state.object_number )["radius"]}
{
    m_state.collidable = true;
    const core::Angle v = core::Angle( util::rand15() * 2 );
    m_state.rotation.Y += v;
    m_state.health = level->m_scriptEngine["getObjectInfo"].call<sol::table>( m_state.object_number )["hit_points"];
}

void AIAgent::collide(LaraNode& lara, CollisionInfo& collisionInfo)
{
    if( !isNear( lara, collisionInfo.collisionRadius ) )
        return;

    if( !testBoneCollision( lara ) )
        return;

    if( !(collisionInfo.policyFlags & CollisionInfo::EnableBaddiePush) )
        return;

    const bool enableSpaz = m_state.health > 0 && (collisionInfo.policyFlags & CollisionInfo::EnableSpaz) != 0;
    enemyPush( lara, collisionInfo, enableSpaz, false );
}
}
}
