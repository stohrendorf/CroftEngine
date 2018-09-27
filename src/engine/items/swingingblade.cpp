#include "swingingblade.h"

#include "engine/heightinfo.h"
#include "engine/laranode.h"
#include "level/level.h"

namespace engine
{
namespace items
{
void SwingingBlade::update()
{
    if( !m_state.updateActivationTimeout() )
    {
        if( m_state.current_anim_state == 2 )
        {
            m_state.goal_anim_state = 0;
        }
    }
    else
    {
        if( m_state.current_anim_state == 0 )
        {
            m_state.goal_anim_state = 2;
        }
    }

    if( m_state.current_anim_state == 2 && m_state.touch_bits != 0 )
    {
        getLevel().m_lara->m_state.is_hit = true;
        getLevel().m_lara->m_state.health -= 100;

        const core::TRVec splatPos{
                getLevel().m_lara->m_state.position.position.X + util::rand15s() / 256,
                getLevel().m_lara->m_state.position.position.Y - util::rand15() / 44,
                getLevel().m_lara->m_state.position.position.Z + util::rand15s() / 256
        };
        auto fx = createBloodSplat( getLevel(),
                                    core::RoomBoundPosition{m_state.position.room, splatPos},
                                    getLevel().m_lara->m_state.speed,
                                    getLevel().m_lara->m_state.rotation.Y + util::rand15s( +22_deg ) );
        getLevel().m_particles.emplace_back( fx );
    }

    auto room = m_state.position.room;
    const auto sector = gsl::make_not_null(
            level::Level::findRealFloorSector( m_state.position.position, make_not_null( &room ) ) );
    setCurrentRoom( room );
    const int h = HeightInfo::fromFloor( sector, m_state.position.position, getLevel().m_itemNodes ).y;
    m_state.floor = h;

    ModelItemNode::update();
}

void SwingingBlade::collide(LaraNode& lara, CollisionInfo& collisionInfo)
{
    if( m_state.triggerState == TriggerState::Active )
    {
        if( isNear( lara, collisionInfo.collisionRadius ) )
        {
            testBoneCollision( lara );
        }
    }
    else if( m_state.triggerState != TriggerState::Invisible
             && isNear( lara, collisionInfo.collisionRadius )
             && testBoneCollision( lara ) )
    {
        if( (collisionInfo.policyFlags & CollisionInfo::EnableBaddiePush) != 0 )
        {
            enemyPush( lara, collisionInfo, false, true );
        }
    }
}
}
}
