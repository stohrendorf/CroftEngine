#include "barricade.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
void Barricade::update()
{
    if( m_state.updateActivationTimeout() )
    {
        m_state.goal_anim_state = 1_as;
    }
    else
    {
        m_state.goal_anim_state = 0_as;
    }

    ModelItemNode::update();
    auto room = m_state.position.room;
    loader::file::findRealFloorSector( m_state.position.position, &room );
    if( room != m_state.position.room )
    {
        setCurrentRoom( room );
    }
}

void Barricade::collide(LaraNode& lara, CollisionInfo& collisionInfo)
{
    if( !isNear( lara, collisionInfo.collisionRadius ) )
        return;

    if( !testBoneCollision( lara ) )
        return;

    if( !collisionInfo.policyFlags.is_set( CollisionInfo::PolicyFlags::EnableBaddiePush ) )
        return;

    enemyPush( lara, collisionInfo, false, true );
}
}
}
