#include "keyhole.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
void KeyHole::collide(engine::LaraNode& lara, engine::CollisionInfo& /*collisionInfo*/)
{
    static const InteractionLimits limits{
            core::BoundingBox{{-200, 0, 312},
                              {200,  0, 512}},
            core::TRRotation{-10_deg, -30_deg, -10_deg},
            core::TRRotation{10_deg, 30_deg, 10_deg}
    };

    if( lara.getCurrentAnimState() != loader::LaraStateId::Stop )
        return;

    if( !getLevel().m_inputHandler->getInputState().action
        || lara.getHandStatus() != engine::HandStatus::None
        || lara.m_state.falling
        || !limits.canInteract( m_state, lara.m_state ) )
        return;

    if( m_state.triggerState == engine::items::TriggerState::Invisible )
    {
        lara.playSoundEffect( 2 );
        return;
    }

    bool hasKey = false;
    switch( m_state.object_number )
    {
        case engine::TR1ItemId::Keyhole1:
            hasKey = getLevel().takeInventoryItem( engine::TR1ItemId::Key1 );
            break;
        case engine::TR1ItemId::Keyhole2:
            hasKey = getLevel().takeInventoryItem( engine::TR1ItemId::Key2 );
            break;
        case engine::TR1ItemId::Keyhole3:
            hasKey = getLevel().takeInventoryItem( engine::TR1ItemId::Key3 );
            break;
        case engine::TR1ItemId::Keyhole4:
            hasKey = getLevel().takeInventoryItem( engine::TR1ItemId::Key4 );
            break;
        default:
            break;
    }
    if( !hasKey )
    {
        lara.playSoundEffect( 2 );
        return;
    }

    lara.alignForInteraction( core::TRVec{0, 0, 362}, m_state );

    do
    {
        lara.setGoalAnimState( LaraStateId::InsertKey );
        lara.updateImpl();
    } while( lara.getCurrentAnimState() != LaraStateId::InsertKey );

    lara.setGoalAnimState( loader::LaraStateId::Stop );
    lara.setHandStatus( engine::HandStatus::Grabbing );
    m_state.triggerState = engine::items::TriggerState::Active;
}
}
}
