#include "aiagent.h"

#include "engine/laranode.h"
#include "engine/particle.h"
#include "engine/script/reflection.h"

#include <boost/range/adaptors.hpp>

namespace engine
{
namespace items
{
core::Angle AIAgent::rotateTowardsTarget(core::Angle maxRotationSpeed)
{
    if( m_state.speed == 0_spd || maxRotationSpeed == 0_au )
    {
        return 0_au;
    }

    const auto dx = m_state.creatureInfo->target.X - m_state.position.position.X;
    const auto dz = m_state.creatureInfo->target.Z - m_state.position.position.Z;
    auto turnAngle = core::Angle::fromAtan( dx, dz ) - m_state.rotation.Y;
    if( turnAngle < -90_deg || turnAngle > 90_deg )
    {
        // the target is behind the current item, so we need a U-turn
        const auto relativeSpeed = core::Speed{m_state.speed.get() * (+90_deg).toAU() / maxRotationSpeed.toAU()};
        if( util::square( dx ) + util::square( dz ) < util::square( relativeSpeed * 1_frame ) )
        {
            maxRotationSpeed /= 2;
        }
    }

    turnAngle = util::clamp( turnAngle, -maxRotationSpeed, maxRotationSpeed );

    m_state.rotation.Y += turnAngle;
    return turnAngle;
}

bool AIAgent::isPositionOutOfReach(const core::TRVec& testPosition,
                                   const core::Length currentBoxFloor,
                                   const core::Length nextBoxFloor,
                                   const ai::LotInfo& lotInfo) const
{
    const auto sectorBox = loader::file::findRealFloorSector( testPosition, m_state.position.room )->box;
    if( sectorBox == nullptr )
        return true;

    if( lotInfo.block_mask & sectorBox->overlap_index )
        return true;

    const auto stepHeight = currentBoxFloor - sectorBox->floor;

    if( stepHeight > lotInfo.step || stepHeight < lotInfo.drop )
        return true;

    if( stepHeight < -lotInfo.step && sectorBox->floor > nextBoxFloor )
        return true;

    return lotInfo.fly != 0_len && testPosition.Y > lotInfo.fly + sectorBox->floor;
}

bool AIAgent::anyMovingEnabledItemInReach() const
{
    for( const auto& item : getEngine().getItemNodes() | boost::adaptors::map_values )
    {
        if( !item->m_isActive || item.get().get() == this || item.get().get() == &getEngine().getLara() )
            continue;

        if( item->m_state.triggerState == TriggerState::Active
            && item->m_state.speed != 0_spd
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

    const auto boxFloor = m_state.box->floor;
    const auto zoneRef = loader::file::Box::getZoneRef( getEngine().roomsAreSwapped(), creatureInfo->lot.fly,
                                                        creatureInfo->lot.step );
    ModelItemNode::update();
    if( m_state.triggerState == TriggerState::Deactivated )
    {
        m_state.health = -16384_hp;
        m_state.collidable = false;
        m_state.creatureInfo.reset();
        deactivate();
        return false;
    }

    const auto bbox = getSkeleton()->getBoundingBox( m_state );
    const auto bboxMinY = m_state.position.position.Y + bbox.minY;

    auto room = m_state.position.room;
    auto sector = loader::file::findRealFloorSector( m_state.position.position + core::TRVec{0_len, bbox.minY, 0_len},
                                                     &room );
    Expects( sector->box != nullptr );
    auto currentFloor = sector->box->floor;

    core::Length nextFloor = 0_len;
    if( lotInfo.nodes.at( sector->box ).exit_box == nullptr )
    {
        nextFloor = currentFloor;
    }
    else
    {
        nextFloor = lotInfo.nodes.at( sector->box ).exit_box->floor;
    }

    if( sector->box == nullptr
        || m_state.box->*zoneRef != sector->box->*zoneRef
        || boxFloor - currentFloor > lotInfo.step
        || boxFloor - currentFloor < lotInfo.drop )
    {
        const auto newSectorX = m_state.position.position.X / core::SectorSize;
        const auto newSectorZ = m_state.position.position.Z / core::SectorSize;

        const auto oldSectorX = oldPosition.X / core::SectorSize;
        const auto oldSectorZ = oldPosition.Z / core::SectorSize;

        static const auto shoveMin = [](const core::Length& l) {
            return l - (l % core::SectorSize);
        };

        static const auto shoveMax = [](const core::Length& l) {
            return shoveMin( l ) + core::SectorSize - 1_len;
        };

        if( newSectorX < oldSectorX )
            m_state.position.position.X = shoveMin( oldPosition.X );
        else if( newSectorX > oldSectorX )
            m_state.position.position.X = shoveMax( oldPosition.X );

        if( newSectorZ < oldSectorZ )
            m_state.position.position.Z = shoveMin( oldPosition.Z );
        else if( newSectorZ > oldSectorZ )
            m_state.position.position.Z = shoveMax( oldPosition.Z );

        sector = loader::file::findRealFloorSector(
                core::TRVec{m_state.position.position.X, bboxMinY, m_state.position.position.Z}, &room );

        currentFloor = sector->box->floor;

        if( lotInfo.nodes.at( sector->box ).exit_box == nullptr )
        {
            nextFloor = sector->box->floor;
        }
        else
        {
            nextFloor = lotInfo.nodes.at( sector->box ).exit_box->floor;
        }
    }

    const auto basePosX = m_state.position.position.X;
    const auto basePosZ = m_state.position.position.Z;

    const auto inSectorX = basePosX % core::SectorSize;
    const auto inSectorZ = basePosZ % core::SectorSize;

    auto objectInfo = getEngine().getScriptEngine()["getObjectInfo"].call<script::ObjectInfo>( m_state.type.get() );
    const core::Length radius{static_cast<core::Length::type>(objectInfo.radius)};

    core::Length moveX = 0_len;
    core::Length moveZ = 0_len;

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
            else if( moveZ == 0_len
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
        else if( core::SectorSize - radius < inSectorX )
        {
            if( isPositionOutOfReach( core::TRVec{radius + basePosX, bboxMinY, basePosZ}, currentFloor,
                                      nextFloor, lotInfo ) )
            {
                moveX = core::SectorSize - radius - inSectorX;
            }
            else if( moveZ == 0_len
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
                    moveX = core::SectorSize - radius - inSectorX;
                }
            }
        }
    }
    else if( core::SectorSize - radius < inSectorZ )
    {
        if( isPositionOutOfReach( core::TRVec{basePosX, bboxMinY, basePosZ + radius}, currentFloor,
                                  nextFloor,
                                  lotInfo ) )
        {
            moveZ = core::SectorSize - radius - inSectorZ;
        }

        if( radius > inSectorX )
        {
            if( isPositionOutOfReach( core::TRVec{basePosX - radius, bboxMinY, basePosZ}, currentFloor,
                                      nextFloor, lotInfo ) )
            {
                moveX = radius - inSectorX;
            }
            else if( moveZ == 0_len
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
                    moveZ = core::SectorSize - radius - inSectorZ;
                }
            }
        }
        else if( core::SectorSize - radius < inSectorX )
        {
            if( isPositionOutOfReach( core::TRVec{radius + basePosX, bboxMinY, basePosZ}, currentFloor,
                                      nextFloor, lotInfo ) )
            {
                moveX = core::SectorSize - radius - inSectorX;
            }
            else if( moveZ == 0_len
                     && isPositionOutOfReach(
                             core::TRVec{radius + basePosX, bboxMinY, basePosZ + radius},
                             currentFloor, nextFloor, lotInfo ) )
            {
                if( m_state.rotation.Y < 45_deg && m_state.rotation.Y > -135_deg )
                {
                    moveX = core::SectorSize - radius - inSectorX;
                }
                else
                {
                    moveZ = core::SectorSize - radius - inSectorZ;
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
    else if( inSectorX > core::SectorSize - radius )
    {
        if( isPositionOutOfReach( core::TRVec{basePosX + radius, bboxMinY, basePosZ}, currentFloor,
                                  nextFloor, lotInfo ) )
        {
            moveX = core::SectorSize - radius - inSectorX;
        }
    }

    m_state.position.position.X += moveX;
    m_state.position.position.Z += moveZ;

    if( moveX != 0_len || moveZ != 0_len )
    {
        sector = loader::file::findRealFloorSector(
                core::TRVec{m_state.position.position.X, bboxMinY, m_state.position.position.Z}, &room );

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

    if( lotInfo.fly != 0_len )
    {
        auto moveY = util::clamp( creatureInfo->target.Y - m_state.position.position.Y,
                                  -lotInfo.fly,
                                  lotInfo.fly );

        currentFloor = HeightInfo::fromFloor( sector,
                                              core::TRVec{
                                                      m_state.position.position.X,
                                                      bboxMinY,
                                                      m_state.position.position.Z
                                              },
                                              getEngine().getItemNodes() ).y;

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
                moveY = 0_len;
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
                                                          getEngine().getItemNodes() ).y;

            const auto y = m_state.type == TR1ItemId::CrocodileInWater ? 0_len : bbox.minY;

            if( m_state.position.position.Y + y + moveY < ceiling )
            {
                if( m_state.position.position.Y + y < ceiling )
                {
                    m_state.position.position.X = oldPosition.X;
                    m_state.position.position.Z = oldPosition.Z;
                    moveY = lotInfo.fly;
                }
                else
                    moveY = 0_len;
            }
        }

        m_state.position.position.Y += moveY;
        sector = loader::file::findRealFloorSector(
                core::TRVec{m_state.position.position.X, bboxMinY, m_state.position.position.Z}, &room );
        m_state.floor = HeightInfo::fromFloor( sector,
                                               core::TRVec{
                                                       m_state.position.position.X,
                                                       bboxMinY,
                                                       m_state.position.position.Z
                                               },
                                               getEngine().getItemNodes() ).y;

        core::Angle yaw{0};
        if( m_state.speed != 0_spd )
            yaw = core::Angle::fromAtan( -moveY, m_state.speed * 1_frame );

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
    else if( m_state.floor - m_state.position.position.Y > 64_len )
    {
        m_state.position.position.Y += 64_len;
    }
    else if( m_state.position.position.Y < m_state.floor )
    {
        m_state.position.position.Y = m_state.floor;
    }

    m_state.rotation.X = 0_au;

    sector = loader::file::findRealFloorSector( m_state.position.position, &room );
    m_state.floor = HeightInfo::fromFloor( sector, m_state.position.position, getEngine().getItemNodes() ).y;

    setCurrentRoom( room );

    return true;
}

AIAgent::AIAgent(const gsl::not_null<Engine*>& engine,
                 const gsl::not_null<const loader::file::Room*>& room,
                 const loader::file::Item& item,
                 const loader::file::SkeletalModelType& animatedModel)
        : ModelItemNode{engine, room, item, true, animatedModel}
        , m_collisionRadius{static_cast<core::Length::type>(engine->getScriptEngine()["getObjectInfo"]
                .call<script::ObjectInfo>( m_state.type.get() ).radius)}
{
    m_state.collidable = true;
    const core::Angle v = core::Angle( util::rand15() * 2 );
    m_state.rotation.Y += v;
    m_state.health = core::Health{static_cast<core::Health::type>(engine->getScriptEngine()["getObjectInfo"]
            .call<script::ObjectInfo>( m_state.type.get() ).hit_points)};
}

void AIAgent::collide(LaraNode& lara, CollisionInfo& collisionInfo)
{
    if( !isNear( lara, collisionInfo.collisionRadius ) )
        return;

    if( !testBoneCollision( lara ) )
        return;

    if( !collisionInfo.policyFlags.is_set( CollisionInfo::PolicyFlags::EnableBaddiePush ) )
        return;

    const bool enableSpaz = m_state.health > 0_hp
                            && collisionInfo.policyFlags.is_set( CollisionInfo::PolicyFlags::EnableSpaz );
    enemyPush( lara, collisionInfo, enableSpaz, false );
}

bool AIAgent::canShootAtLara(const ai::AiInfo& aiInfo) const
{
    if( !aiInfo.ahead || aiInfo.distance >= util::square( 7 * core::SectorSize ) )
    {
        return false;
    }

    auto start = m_state.position;
    auto end = getEngine().getLara().m_state.position;
    end.position.Y -= 768_len;
    return CameraController::clampPosition( start, end, getEngine() );
}

namespace
{
gsl::not_null<std::shared_ptr<Particle>> createGunFlare(Engine& engine,
                                                        const core::RoomBoundPosition& pos,
                                                        core::Speed speed,
                                                        core::Angle angle)
{
    auto particle = std::make_shared<GunflareParticle>( pos, engine, angle );
    setParent( particle, pos.room->node );
    return particle;
}
}

bool AIAgent::tryShootAtLara(items::ModelItemNode& item,
                             core::Area distance,
                             const core::TRVec& bonePos,
                             size_t boneIndex,
                             core::Angle angle)
{
    bool isHit = false;
    if( distance <= util::square( 7 * core::SectorSize ) )
    {
        if( util::rand15() < (util::square( 7 * core::SectorSize ) - distance) / util::square( 40_len ) - 8192 )
        {
            isHit = true;

            getEngine().getLara().emitParticle( core::TRVec{},
                                                util::rand15( getEngine().getLara().getNode()->getChildren().size() ),
                                                &createBloodSplat );

            if( !getEngine().getLara().isInWater() )
                getEngine().getLara().playSoundEffect( TR1SoundId::BulletHitsLara );
        }
    }

    if( !isHit )
    {
        auto pos = getEngine().getLara().m_state.position;
        pos.position.X += util::rand15s( core::SectorSize / 2, core::Length::type() );
        pos.position.Y = getEngine().getLara().m_state.floor;
        pos.position.Z += util::rand15s( core::SectorSize / 2, core::Length::type() );
        getEngine().getLara().playShotMissed( pos );
    }

    auto p = item.emitParticle( bonePos, boneIndex, &createGunFlare );
    p->angle.Y += angle;

    return isHit;
}
}
}
