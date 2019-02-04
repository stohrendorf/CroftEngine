#include "bat.h"

#include "level/level.h"
#include "engine/laranode.h"
#include "engine/ai/ai.h"

namespace engine
{
namespace items
{
void Bat::update()
{
    if( m_state.triggerState == TriggerState::Invisible )
    {
        m_state.triggerState = TriggerState::Active;
    }

    m_state.initCreatureInfo( getLevel() );

    static constexpr const uint16_t StartingToFly = 1;
    static constexpr const uint16_t FlyingStraight = 2;
    static constexpr const uint16_t Biting = 3;
    static constexpr const uint16_t Circling = 4;
    static constexpr const uint16_t Dying = 5;

    core::Angle rotationToMoveTarget = 0_deg;
    if( getHealth() > 0_hp )
    {
        const ai::AiInfo aiInfo{getLevel(), m_state};
        updateMood( getLevel(), m_state, aiInfo, false );

        rotationToMoveTarget = rotateTowardsTarget( 20_deg );
        switch( m_state.current_anim_state.get() )
        {
            case StartingToFly:
                m_state.goal_anim_state = FlyingStraight;
                break;
            case FlyingStraight:
                if( m_state.touch_bits != 0 )
                {
                    m_state.goal_anim_state = Biting;
                }
                break;
            case Biting:
                if( m_state.touch_bits != 0 )
                {
                    emitParticle( core::TRVec{0_len, 16_len, 45_len}, 4, &createBloodSplat );
                    getLevel().m_lara->m_state.is_hit = true;
                    getLevel().m_lara->m_state.health -= 2_hp;
                }
                else
                {
                    m_state.goal_anim_state = FlyingStraight;
                    m_state.creatureInfo->mood = ai::Mood::Bored;
                }
                break;
            default:
                break;
        }
    }
    else
    {
        if( m_state.position.position.Y >= m_state.floor )
        {
            m_state.goal_anim_state = Dying;
            m_state.position.position.Y = m_state.floor;
            m_state.falling = false;
        }
        else
        {
            m_state.goal_anim_state = Circling;
            m_state.speed = 0_len;
            m_state.falling = true;
        }
    }
    animateCreature( rotationToMoveTarget, 0_deg );
}
}
}
