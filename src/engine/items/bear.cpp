#include "bear.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
void Bear::update()
{
    if( m_state.triggerState == TriggerState::Invisible )
    {
        m_state.triggerState = TriggerState::Active;
    }

    m_state.initCreatureInfo( getLevel() );

    core::Angle rotationToMoveTarget = 0_deg;
    if( getHealth() > 0 )
    {
        ai::AiInfo aiInfo{getLevel(), m_state};
        ai::updateMood( getLevel(), m_state, aiInfo, true );

        rotationToMoveTarget = rotateTowardsTarget( m_state.creatureInfo->maximum_turn );
        if( m_state.is_hit )
            m_state.creatureInfo->flags = 1;

        switch( m_state.current_anim_state )
        {
            case 0: // walking
                m_state.creatureInfo->maximum_turn = 2_deg;
                if( getHealth() <= 0 && (m_state.touch_bits & 0x2406c) != 0 && aiInfo.ahead )
                {
                    m_state.goal_anim_state = 1;
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Escape )
                {
                    m_state.required_anim_state = 0;
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Bored )
                {
                    m_state.goal_anim_state = 1;
                }
                else if( util::rand15() < 80 )
                {
                    m_state.required_anim_state = 5;
                    m_state.goal_anim_state = 1;
                }
                break;
            case 1: // getting down again
                if( getHealth() <= 0 )
                {
                    if( aiInfo.bite && aiInfo.distance < 0x90000 )
                    {
                        m_state.goal_anim_state = 8;
                    }
                    else
                    {
                        m_state.goal_anim_state = 0;
                    }
                }
                else
                {
                    if( m_state.required_anim_state != 0 )
                    {
                        m_state.goal_anim_state = m_state.required_anim_state;
                    }
                    else if( m_state.creatureInfo->mood != ai::Mood::Bored )
                    {
                        m_state.goal_anim_state = 3;
                    }
                    else
                    {
                        m_state.goal_anim_state = 0;
                    }
                }
                break;
            case 2: // walking tall
                if( m_state.creatureInfo->flags != 0 )
                {
                    m_state.required_anim_state = 0;
                    m_state.goal_anim_state = 4;
                }
                else if( aiInfo.ahead && (m_state.touch_bits & 0x2406c) != 0 )
                {
                    m_state.goal_anim_state = 4;
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Escape )
                {
                    m_state.goal_anim_state = 4;
                    m_state.required_anim_state = 0;
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Bored && util::rand15() >= 80 )
                {
                    if( aiInfo.distance > 0x400000 || util::rand15() < 1536 )
                    {
                        m_state.required_anim_state = 1;
                        m_state.goal_anim_state = 4;
                    }
                }
                else
                {
                    m_state.required_anim_state = 5;
                    m_state.goal_anim_state = 4;
                }
                break;
            case 3: // running
                m_state.creatureInfo->maximum_turn = 5_deg;
                if( (m_state.touch_bits & 0x2406c) != 0 )
                {
                    getLevel().m_lara->m_state.health -= 3;
                    getLevel().m_lara->m_state.is_hit = true;
                }
                if( m_state.creatureInfo->mood == ai::Mood::Bored || getHealth() <= 0 )
                {
                    m_state.goal_anim_state = 1;
                }
                else if( aiInfo.ahead && m_state.required_anim_state == 0 )
                {
                    if( m_state.creatureInfo->flags == 0 && aiInfo.distance < 0x400000 && util::rand15() < 768 )
                    {
                        m_state.required_anim_state = 4;
                        m_state.goal_anim_state = 1;
                    }
                    else if( aiInfo.distance < 0x100000 )
                    {
                        m_state.goal_anim_state = 6;
                    }
                }
                break;
            case 4: // roaring standing
                if( m_state.creatureInfo->flags != 0 )
                {
                    m_state.required_anim_state = 0;
                    m_state.goal_anim_state = 1;
                }
                else
                {
                    if( m_state.required_anim_state != 0 )
                    {
                        m_state.goal_anim_state = m_state.required_anim_state;
                    }
                    else if( m_state.creatureInfo->mood == ai::Mood::Bored
                             || m_state.creatureInfo->mood == ai::Mood::Escape )
                    {
                        m_state.goal_anim_state = 1;
                    }
                    else if( aiInfo.bite && aiInfo.distance < 360000 )
                    {
                        m_state.goal_anim_state = 7;
                    }
                    else
                    {
                        m_state.goal_anim_state = 2;
                    }
                }
                break;
            case 6: // running attack
                if( m_state.required_anim_state == 0 && (m_state.touch_bits & 0x2406C) )
                {
                    emitParticle( core::TRCoordinates{0, 96, 335}, 14, &engine::createBloodSplat );
                    getLevel().m_lara->m_state.health -= 200;
                    getLevel().m_lara->m_state.is_hit = true;
                    m_state.required_anim_state = 1;
                }
                break;
            case 7: // standing
                if( m_state.required_anim_state == 0 && (m_state.touch_bits & 0x2406C) )
                {
                    getLevel().m_lara->m_state.health -= 400;
                    getLevel().m_lara->m_state.is_hit = true;
                    m_state.required_anim_state = 4;
                }
                break;
            default:
                break;
        }
        rotateCreatureHead( aiInfo.angle );
    }
    else
    {
        rotationToMoveTarget = rotateTowardsTarget( 1_deg );
        switch( m_state.current_anim_state )
        {
            case 0:
            case 3:
                m_state.goal_anim_state = 1;
                break;
            case 1:
                m_state.creatureInfo->flags = 0;
                m_state.goal_anim_state = 9;
                break;
            case 2:
                m_state.goal_anim_state = 4;
                break;
            case 4:
                m_state.creatureInfo->flags = 1;
                m_state.goal_anim_state = 9;
                break;
            case 9:
                if( m_state.creatureInfo->flags != 0 && (m_state.touch_bits & 0x2406C) != 0 )
                {
                    getLevel().m_lara->m_state.health -= 200;
                    getLevel().m_lara->m_state.is_hit = true;
                    m_state.creatureInfo->flags = 0;
                }
                break;
            default:
                break;
        }
        rotateCreatureHead( 0_deg );
    }
    getSkeleton()->patchBone( 14, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix() );
    animateCreature( rotationToMoveTarget, 0_deg );
}
}
}
