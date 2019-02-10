#include "boulder.h"

#include "level/level.h"
#include "engine/laranode.h"

void engine::items::RollingBall::update()
{
    if( m_state.triggerState == TriggerState::Active )
    {
        if( m_state.position.position.Y >= m_state.floor )
        {
            if( m_state.current_anim_state == 0_as )
            {
                m_state.goal_anim_state = 1_as;
            }
        }
        else
        {
            if( !m_state.falling )
            {
                m_state.fallspeed = -10_spd;
                m_state.falling = true;
            }
        }
        const auto oldPos = m_state.position.position;
        ModelItemNode::update();

        auto room = m_state.position.room;
        auto sector = level::Level::findRealFloorSector( m_state.position.position, &room );
        setCurrentRoom( room );
        const auto hi = HeightInfo::fromFloor( sector, m_state.position.position, getLevel().m_itemNodes );
        m_state.floor = hi.y;
        getLevel().m_lara->handleCommandSequence( hi.lastCommandSequenceOrDeath, true );
        if( m_state.floor - core::QuarterSectorSize <= m_state.position.position.Y )
        {
            m_state.fallspeed = 0_spd;
            m_state.falling = false;
            m_state.position.position.Y = m_state.floor;
        }

        // let's see if we hit a wall, and if that's the case, stop.
        const auto testPos = m_state.position.position
                             + core::TRVec( (m_state.rotation.Y.sin() * core::SectorSize.retype_as<float>()).retype_as<core::Length>() / 2,
                                            0_len,
                                            (m_state.rotation.Y.cos() * core::SectorSize.retype_as<float>()).retype_as<core::Length>() / 2 );
        sector = level::Level::findRealFloorSector( testPos, room );
        if( HeightInfo::fromFloor( sector, testPos, getLevel().m_itemNodes ).y < m_state.position.position.Y )
        {
            m_state.fallspeed = 0_spd;
            m_state.touch_bits.reset();
            m_state.speed = 0_spd;
            m_state.triggerState = TriggerState::Deactivated;
            m_state.position.position.X = oldPos.X;
            m_state.position.position.Y = m_state.floor;
            m_state.position.position.Z = oldPos.Z;
        }
    }
    else if( m_state.triggerState == TriggerState::Deactivated && !m_state.updateActivationTimeout() )
    {
        m_state.triggerState = TriggerState::Deactivated;
        m_state.position.position = m_position.position;
        setCurrentRoom( m_position.room );
        getSkeleton()->setAnimation( m_state, getLevel().m_animatedModels[m_state.type]->animations, 0_frame );
        m_state.goal_anim_state = m_state.current_anim_state;
        m_state.required_anim_state = 0_as;
        deactivate();
    }
}

void engine::items::RollingBall::collide(LaraNode& lara, CollisionInfo& collisionInfo)
{
    if( m_state.triggerState != TriggerState::Active )
    {
        if( m_state.triggerState != TriggerState::Invisible )
        {
            if( !isNear( lara, collisionInfo.collisionRadius ) )
                return;

            if( !testBoneCollision( lara ) )
                return;

            if( !collisionInfo.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush) )
                return;

            enemyPush( lara, collisionInfo, false, true );
        }
        return;
    }

    if( !isNear( lara, collisionInfo.collisionRadius ) )
        return;

    if( !testBoneCollision( lara ) )
        return;

    if( !lara.m_state.falling )
    {
        lara.m_state.is_hit = true;
        if( lara.m_state.health <= 0_hp )
            return;

        lara.m_state.health = -1_hp;
        lara.setCurrentRoom( m_state.position.room );
        lara.setAnimation( loader::AnimationId::SQUASH_BOULDER, 3561_frame );
        getLevel().m_cameraController->setModifier( CameraModifier::FollowCenter );
        getLevel().m_cameraController->setEyeRotation( -25_deg, 170_deg );
        lara.m_state.rotation.X = 0_deg;
        lara.m_state.rotation.Y = m_state.rotation.Y;
        lara.m_state.rotation.Z = 0_deg;
        lara.setGoalAnimState( loader::LaraStateId::BoulderDeath );
        for( int i = 0; i < 15; ++i )
        {
            const auto x = util::rand15s( 128_len, core::Length::type() ) + lara.m_state.position.position.X;
            const auto y = lara.m_state.position.position.Y - util::rand15s( 512_len, core::Length::type() );
            const auto z = util::rand15s( 128_len, core::Length::type() ) + lara.m_state.position.position.Z;
            auto fx = createBloodSplat(
                    getLevel(),
                    core::RoomBoundPosition{m_state.position.room, core::TRVec{x, y, z}},
                    2 * m_state.speed,
                    core::Angle( gsl::narrow_cast<int16_t>( util::rand15s( 4096 ) ) ) + m_state.rotation.Y
            );
            getLevel().m_particles.emplace_back( fx );
        }
        return;
    }

    if( collisionInfo.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush) )
    {
        enemyPush(
                lara,
                collisionInfo,
                collisionInfo.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableSpaz),
                true );
    }
    lara.m_state.health -= 100_hp;
    const auto x = lara.m_state.position.position.X - m_state.position.position.X;
    const auto y = lara.m_state.position.position.Y - 350_len
                   - (m_state.position.position.Y - 2 * core::QuarterSectorSize);
    const auto z = lara.m_state.position.position.Z - m_state.position.position.Z;
    const auto xyz = std::max( 2 * core::QuarterSectorSize,
            sqrt( util::square( x ) + util::square( y ) + util::square( z ) ) );

    auto fx = createBloodSplat(
            getLevel(),
            core::RoomBoundPosition{
                    m_state.position.room,
                    core::TRVec{
                            x * core::SectorSize / 2 / xyz + m_state.position.position.X,
                            y * core::SectorSize / 2 / xyz + m_state.position.position.Y
                            - 2 * core::QuarterSectorSize,
                            z * core::SectorSize / 2 / xyz + m_state.position.position.Z
                    }
            },
            m_state.speed,
            m_state.rotation.Y
    );
    getLevel().m_particles.emplace_back( fx );
}
