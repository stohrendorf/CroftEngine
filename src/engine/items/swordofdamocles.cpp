#include "swordofdamocles.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
void SwordOfDamocles::update()
{
    if( m_state.current_anim_state == 0_as )
    {
        m_state.goal_anim_state = 1_as;
        m_state.falling = true;
    }
    else if( m_state.current_anim_state == 1_as && m_state.touch_bits != 0 )
    {
        getEngine().getLara().m_state.is_hit = true;
        getEngine().getLara().m_state.health -= 300_hp;
    }

    ModelItemNode::update();

    if( m_state.triggerState == TriggerState::Deactivated )
    {
        deactivate();
    }
    else if( m_state.current_anim_state == 1_as && m_state.position.position.Y >= m_state.floor )
    {
        m_state.goal_anim_state = 2_as;
        m_state.position.position.Y = m_state.floor;
        m_state.fallspeed = 0_spd;
        m_state.falling = false;
    }
}

void SwordOfDamocles::collide(LaraNode& lara, CollisionInfo& collisionInfo)
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
        if( collisionInfo.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush) )
        {
            enemyPush( lara, collisionInfo, false, true );
        }
    }
}
}
}
