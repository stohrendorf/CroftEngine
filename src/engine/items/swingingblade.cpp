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
        if( m_state.current_anim_state == 2_as )
        {
            m_state.goal_anim_state = 0_as;
        }
    }
    else
    {
        if( m_state.current_anim_state == 0_as )
        {
            m_state.goal_anim_state = 2_as;
        }
    }

    if( m_state.current_anim_state == 2_as && m_state.touch_bits.any() )
    {
        getLevel().m_lara->m_state.is_hit = true;
        getLevel().m_lara->m_state.health -= 100_hp;

        const core::TRVec splatPos{
                getLevel().m_lara->m_state.position.position.X + util::rand15s( 128_len, core::Length::type() ),
                getLevel().m_lara->m_state.position.position.Y - util::rand15( 745_len, core::Length::type() ),
                getLevel().m_lara->m_state.position.position.Z + util::rand15s( 128_len, core::Length::type() )
        };
        auto fx = createBloodSplat( getLevel(),
                                    core::RoomBoundPosition{m_state.position.room, splatPos},
                                    getLevel().m_lara->m_state.speed,
                                    getLevel().m_lara->m_state.rotation.Y + util::rand15s( +22_deg ) );
        getLevel().m_particles.emplace_back( fx );
    }

    auto room = m_state.position.room;
    const auto sector = level::Level::findRealFloorSector( m_state.position.position, &room );
    setCurrentRoom( room );
    m_state.floor = HeightInfo::fromFloor( sector, m_state.position.position, getLevel().m_itemNodes ).y;

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
        if( collisionInfo.policyFlags.is_set( CollisionInfo::PolicyFlags::EnableBaddiePush ) )
        {
            enemyPush( lara, collisionInfo, false, true );
        }
    }
}
}
}
