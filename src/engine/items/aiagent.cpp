#include "aiagent.h"
#include "engine/heightinfo.h"
#include "level/level.h"

#include <boost/range/adaptors.hpp>

namespace engine
{
namespace items
{
core::Angle AIAgent::rotateTowardsMoveTarget(core::Angle maxRotationSpeed)
{
    if( m_state.speed == 0 || maxRotationSpeed == 0_au )
    {
        return 0_au;
    }

    const auto dx = m_state.creatureInfo->target.X - m_state.position.position.X;
    const auto dz = m_state.creatureInfo->target.Z - m_state.position.position.Z;
    auto rotation = core::Angle::fromAtan( dx, dz ) - m_state.rotation.Y;
    if( rotation > 90_deg || rotation < -90_deg )
    {
        // the move target is behind the NPC
        auto relativeSpeed = m_state.speed * (1 << 14) / maxRotationSpeed.toAU();
        if( util::square( dx ) + util::square( dz ) < util::square( relativeSpeed ) )
        {
            maxRotationSpeed /= 2;
        }
    }

    if( rotation > maxRotationSpeed )
    {
        rotation = maxRotationSpeed;
    }
    else if( rotation < -maxRotationSpeed )
    {
        rotation = -maxRotationSpeed;
    }

    m_state.rotation.Y += rotation;
    return rotation;
}

bool AIAgent::isPositionOutOfReach(const core::TRCoordinates& testPosition, int currentBoxFloor,
                                   const ai::LotInfo& lotInfo) const
{
    const auto sectorBoxIdx = getLevel().findRealFloorSector( testPosition, m_state.position.room )->boxIndex;
    if( sectorBoxIdx == 0xffff )
    {
        return true;
    }

    const auto& sectorBox = getLevel().m_boxes[sectorBoxIdx];
    if( lotInfo.block_mask & sectorBox.overlap_index )
    {
        return true;
    }

    const auto testBoxFloor = currentBoxFloor - sectorBox.floor;
    if( testBoxFloor >= lotInfo.drop && testBoxFloor <= lotInfo.step )
    {
        // We could step up to/drop down to the test sector.
        return lotInfo.fly != 0 && testPosition.Y > lotInfo.fly + sectorBox.floor;
    }

    return true;
}

bool AIAgent::anyMovingEnabledItemInReach() const
{
    for( const std::shared_ptr<ItemNode>& item : getLevel().m_itemNodes | boost::adaptors::map_values )
    {
        if( !item->m_isActive || item.get() == this )
        {
            continue;
        }

        if( item->m_state.triggerState == items::TriggerState::Enabled
            && item->m_state.speed != 0
            && item->m_state.position.position.distanceTo( m_state.position.position ) < m_collisionRadius )
        {
            return true;
        }
    }
    return false;
}

bool AIAgent::animateCreature(core::Angle rotationToMoveTarget, core::Angle roll)
{
    if( m_state.triggerState == engine::items::TriggerState::Activated )
    {
        m_health = -16384;
        m_state.collidable = false;
        m_state.creatureInfo.reset();
        deactivate();
        return false;
    }

    ModelItemNode::update();

    const auto initialPos = m_state.position.position;
    auto bboxTop = getSkeleton()->getBoundingBox( m_state ).minY;
    const loader::Room* npcRoom = m_state.position.room;
    auto npcSector = getLevel().findRealFloorSector(
            m_state.position.position + core::TRCoordinates( 0, bboxTop, 0 ),
            &npcRoom );

    if( npcSector->boxIndex == 0xffff )
    {
        return false;
    }

    const auto npcBoxFloor = getLevel().m_boxes[npcSector->boxIndex].floor;

    const auto inSectorX = m_state.position.position.X % loader::SectorSize;
    const auto inSectorZ = m_state.position.position.Z % loader::SectorSize;
    int moveZ = 0;
    int moveX = 0;
    if( inSectorZ < m_collisionRadius )
    {
        // Clamp movement to -Z

        if( isPositionOutOfReach(
                m_state.position.position + core::TRCoordinates( 0, bboxTop, -m_collisionRadius ),
                npcBoxFloor,
                m_state.creatureInfo->lot ) )
        {
            moveZ = m_collisionRadius - inSectorZ;
        }

        if( inSectorX < m_collisionRadius )
        {
            // Clamp movement to -X

            if( isPositionOutOfReach(
                    m_state.position.position + core::TRCoordinates( -m_collisionRadius, bboxTop, 0 ),
                    npcBoxFloor,
                    m_state.creatureInfo->lot ) )
            {
                moveX = m_collisionRadius - inSectorX;
            }
            else if( moveZ == 0
                     && isPositionOutOfReach(
                             m_state.position.position
                             + core::TRCoordinates( -m_collisionRadius, bboxTop, -m_collisionRadius ),
                             npcBoxFloor,
                             m_state.creatureInfo->lot ) )
            {
                // -X/-Z must be clamped. Clamp the lateral direction to allow better forward movement.
                if( m_state.rotation.Y > -135_deg && m_state.rotation.Y < 45_deg )
                {
                    // We're facing -X/+Z
                    moveZ = m_collisionRadius - inSectorZ;
                }
                else
                {
                    moveX = m_collisionRadius - inSectorX;
                }
            }
        }
        else if( inSectorX > loader::SectorSize - m_collisionRadius )
        {
            // Clamp movement to +X

            if( isPositionOutOfReach(
                    m_state.position.position + core::TRCoordinates( m_collisionRadius, bboxTop, 0 ),
                    npcBoxFloor,
                    m_state.creatureInfo->lot ) )
            {
                moveX = loader::SectorSize - m_collisionRadius - inSectorX;
            }
            else if( moveZ == 0
                     && isPositionOutOfReach(
                             m_state.position.position
                             + core::TRCoordinates( m_collisionRadius, bboxTop, -m_collisionRadius ),
                             npcBoxFloor,
                             m_state.creatureInfo->lot ) )
            {
                // +X/-Z
                if( m_state.rotation.Y > -45_deg && m_state.rotation.Y < 135_deg )
                {
                    moveZ = m_collisionRadius - inSectorZ;
                }
                else
                {
                    moveX = loader::SectorSize - m_collisionRadius - inSectorX;
                }
            }
        }
    }
    else if( inSectorZ > loader::SectorSize - m_collisionRadius )
    {
        if( isPositionOutOfReach(
                m_state.position.position + core::TRCoordinates( 0, bboxTop, m_collisionRadius ),
                npcBoxFloor,
                m_state.creatureInfo->lot ) )
        {
            moveZ = loader::SectorSize - m_collisionRadius - inSectorZ;
        }

        if( inSectorX < m_collisionRadius )
        {
            if( isPositionOutOfReach(
                    m_state.position.position + core::TRCoordinates( -m_collisionRadius, bboxTop, 0 ),
                    npcBoxFloor,
                    m_state.creatureInfo->lot ) )
            {
                moveX = m_collisionRadius - inSectorX;
            }
            else if( moveZ == 0
                     && isPositionOutOfReach(
                             m_state.position.position
                             + core::TRCoordinates( -m_collisionRadius, bboxTop, -m_collisionRadius ),
                             npcBoxFloor,
                             m_state.creatureInfo->lot ) )
            {
                if( m_state.rotation.Y > -45_deg && m_state.rotation.Y < 135_deg )
                {
                    moveX = m_collisionRadius - inSectorX;
                }
                else
                {
                    moveZ = loader::SectorSize - m_collisionRadius - inSectorZ;
                }
            }
        }
        else if( inSectorX > loader::SectorSize - m_collisionRadius )
        {
            if( isPositionOutOfReach(
                    m_state.position.position + core::TRCoordinates( m_collisionRadius, bboxTop, 0 ),
                    npcBoxFloor,
                    m_state.creatureInfo->lot ) )
            {
                moveX = loader::SectorSize - m_collisionRadius - inSectorX;
            }
            else if( moveZ == 0
                     && isPositionOutOfReach(
                             m_state.position.position
                             + core::TRCoordinates( m_collisionRadius, bboxTop, m_collisionRadius ),
                             npcBoxFloor,
                             m_state.creatureInfo->lot ) )
            {
                if( m_state.rotation.Y > -135_deg && m_state.rotation.Y < 45_deg )
                {
                    moveX = loader::SectorSize - m_collisionRadius - inSectorX;
                }
                else
                {
                    moveZ = loader::SectorSize - m_collisionRadius - inSectorZ;
                }
            }
        }
    }
    else if( inSectorX < m_collisionRadius )
    {
        if( isPositionOutOfReach(
                m_state.position.position + core::TRCoordinates( -m_collisionRadius, bboxTop, 0 ),
                npcBoxFloor,
                m_state.creatureInfo->lot ) )
        {
            moveX = m_collisionRadius - inSectorX;
        }
    }
    else if( inSectorX > loader::SectorSize - m_collisionRadius )
    {
        if( isPositionOutOfReach(
                m_state.position.position + core::TRCoordinates( m_collisionRadius, bboxTop, 0 ),
                npcBoxFloor,
                m_state.creatureInfo->lot ) )
        {
            moveX = loader::SectorSize - m_collisionRadius - inSectorX;
        }
    }

    m_state.position.position.X += moveX;
    m_state.position.position.Z += moveX;

    if( moveX != 0 || moveZ != 0 )
    {
        npcSector = getLevel().findRealFloorSector(
                m_state.position.position + core::TRCoordinates( 0, bboxTop, 0 ),
                &npcRoom );
        auto effectiveCurveRoll = util::clamp( core::Angle( 8 * roll.toAU() ) - m_state.rotation.Z, -3_deg, +3_deg );
        m_state.rotation.Y += rotationToMoveTarget;
        m_state.rotation.Z += effectiveCurveRoll;
    }

    if( anyMovingEnabledItemInReach() )
    {
        m_state.position.position = initialPos;
        return true;
    }

    if( m_state.creatureInfo->lot.fly != 0 )
    {
        auto dy = util::clamp<int32_t>( m_state.creatureInfo->target.Y - m_state.position.position.Y,
                                        -m_state.creatureInfo->lot.fly,
                                        m_state.creatureInfo->lot.fly );
        const auto currentFloor = engine::HeightInfo::fromFloor( npcSector,
                                                                 m_state.position.position
                                                                 + core::TRCoordinates( 0, bboxTop, 0 ),
                                                                 getLevel().m_cameraController ).distance;
        if( dy + m_state.position.position.Y <= currentFloor )
        {
            const auto currentCeiling = HeightInfo::fromCeiling( npcSector,
                                                                 m_state.position.position
                                                                 + core::TRCoordinates( 0, bboxTop, 0 ),
                                                                 getLevel().m_cameraController ).distance;
            /** @fixme
            if( npc is CrocodileInWater )
            {
            bboxTop = 0;
            }
            */
            if( m_state.position.position.Y + bboxTop + dy < currentCeiling )
            {
                if( m_state.position.position.Y + bboxTop >= currentCeiling )
                {
                    dy = 0;
                }
                else
                {
                    m_state.position.position.X = m_state.position.position.X;
                    m_state.position.position.Z = m_state.position.position.Z;
                    dy = m_state.creatureInfo->lot.fly;
                }
            }
        }
        else if( currentFloor >= m_state.position.position.Y )
        {
            dy = 0;
            m_state.position.position.Y = currentFloor;
        }
        else
        {
            m_state.position.position.X = m_state.position.position.X;
            m_state.position.position.Z = m_state.position.position.Z;
            dy = -m_state.creatureInfo->lot.fly;
        }
        m_state.position.position.Y += dy;
        const auto sector = getLevel().findRealFloorSector(
                m_state.position.position + core::TRCoordinates( 0, bboxTop, 0 ),
                &npcRoom );
        const int h = HeightInfo::fromCeiling( sector,
                                               m_state.position.position + core::TRCoordinates( 0, bboxTop, 0 ),
                                               getLevel().m_cameraController ).distance;
        m_state.floor = h;

        core::Angle flyAngle = 0_au;
        if( m_state.speed != 0 )
        {
            flyAngle = core::Angle::fromAtan( m_state.speed, dy );
        }

        const core::Angle x = util::clamp( flyAngle, -1_deg, +1_deg );
        m_state.rotation.X = x;

        if( npcRoom != m_state.position.room )
        {
            setCurrentRoom( npcRoom );
        }
        return true;
    }

    if( m_state.position.position.Y - 1 >= m_state.floor )
    {
        m_state.position.position.Y = m_state.floor;
    }
    else
    {
        if( m_state.floor > 64 + m_state.position.position.Y - 1 )
        {
            m_state.position.position.Y += 64;
        }
    }

    m_state.rotation.X = 0_au;
    const auto currentSector = getLevel().findRealFloorSector(
            m_state.position.position,
            &npcRoom );
    const int h1 = HeightInfo::fromFloor( currentSector, m_state.position.position, getLevel().m_cameraController )
            .distance;
    m_state.floor = h1;
    if( npcRoom != m_state.position.room )
    {
        setCurrentRoom( npcRoom );
    }
    return true;
}

AIAgent::AIAgent(const gsl::not_null<level::Level*>& level,
                 const std::string& name,
                 const gsl::not_null<const loader::Room*>& room,
                 const loader::Item& item,
                 ItemNode::Characteristics characteristics,
                 const loader::SkeletalModelType& animatedModel,
                 int collisionRadius)
        : ModelItemNode( level, name, room, item, true, characteristics, animatedModel )
        , m_collisionRadius{collisionRadius}
{
    m_state.collidable = true;
    const core::Angle v = core::Angle( std::rand() % 65536 );
    m_state.rotation.Y += v;
}
}
}
