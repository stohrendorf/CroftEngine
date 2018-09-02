#include "swordofdamocles.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
void SwordOfDamocles::update()
{
    if( m_state.current_anim_state == 0 )
    {
        m_state.goal_anim_state = 1;
        m_state.falling = true;
    }
    else if( m_state.current_anim_state == 1 && m_state.touch_bits )
    {
        getLevel().m_lara->m_state.is_hit = true;
        getLevel().m_lara->m_state.health -= 300;
    }

    ModelItemNode::update();

    if( m_state.triggerState == TriggerState::Deactivated )
    {
        deactivate();
    }
    else if( m_state.current_anim_state == 1 && m_state.position.position.Y >= m_state.floor )
    {
        m_state.goal_anim_state = 2;
        m_state.position.position.Y = m_state.floor;
        m_state.fallspeed = 0;
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
        if( (collisionInfo.policyFlags & CollisionInfo::EnableBaddiePush) != 0 )
        {
            enemyPush( lara, collisionInfo, false, true );
        }
    }
}
}
}
