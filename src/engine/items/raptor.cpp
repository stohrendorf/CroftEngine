#include "raptor.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
void Raptor::update()
{
    if( m_state.triggerState == TriggerState::Invisible )
    {
        m_state.triggerState = TriggerState::Active;
    }

    m_state.initCreatureInfo( getLevel() );

    core::Angle animTilt = 0_deg;
    core::Angle animAngle = 0_deg;
    core::Angle animHead = 0_deg;
    if( m_state.health > 0 )
    {
        const ai::AiInfo aiInfo{getLevel(), m_state};
        if( aiInfo.ahead )
        {
            animHead = aiInfo.angle;
        }
        updateMood( getLevel(), m_state, aiInfo, true );
        animAngle = rotateTowardsTarget( m_state.creatureInfo->maximum_turn );
        switch( m_state.current_anim_state.get() )
        {
            case 1:
                if( m_state.required_anim_state != 0_as )
                {
                    m_state.goal_anim_state = m_state.required_anim_state;
                }
                else if( m_state.touch_bits.to_ulong() & 0xff7c00UL )
                {
                    m_state.goal_anim_state = 8_as;
                }
                else if( aiInfo.bite && aiInfo.distance < util::square( 680_len ) )
                {
                    m_state.goal_anim_state = 8_as;
                }
                else if( aiInfo.bite && aiInfo.distance < util::square( 1536_len ) )
                {
                    m_state.goal_anim_state = 4_as;
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Bored )
                {
                    m_state.goal_anim_state = 3_as;
                }
                else
                {
                    m_state.goal_anim_state = 2_as;
                }
                break;
            case 2:
                m_state.creatureInfo->maximum_turn = 1_deg;
                if( m_state.creatureInfo->mood != ai::Mood::Bored )
                {
                    m_state.goal_anim_state = 1_as;
                }
                else if( aiInfo.ahead )
                {
                    if( util::rand15() < 256 )
                    {
                        m_state.required_anim_state = 6_as;
                        m_state.goal_anim_state = 1_as;
                    }
                }
                break;
            case 3:
                m_state.creatureInfo->maximum_turn = 4_deg;
                animTilt = animAngle;
                if( m_state.touch_bits.to_ulong() & 0xff7c00UL )
                {
                    m_state.goal_anim_state = 1_as;
                }
                else if( aiInfo.bite && aiInfo.distance < util::square( 1536_len ) )
                {
                    if( m_state.goal_anim_state == 3_as )
                    {
                        if( util::rand15() >= 8192 )
                        {
                            m_state.goal_anim_state = 7_as;
                        }
                        else
                        {
                            m_state.goal_anim_state = 1_as;
                        }
                    }
                }
                else if( aiInfo.ahead && m_state.creatureInfo->mood != ai::Mood::Escape && util::rand15() < 256 )
                {
                    m_state.required_anim_state = 6_as;
                    m_state.goal_anim_state = 1_as;
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Bored )
                {
                    m_state.goal_anim_state = 1_as;
                }
                break;
            case 4:
                animTilt = animAngle;
                if( m_state.required_anim_state == 0_as )
                {
                    if( aiInfo.ahead )
                    {
                        if( m_state.touch_bits.to_ulong() & 0xff7c00UL )
                        {
                            emitParticle( core::TRVec{0_len, 66_len, 318_len}, 22, &createBloodSplat );
                            getLevel().m_lara->m_state.is_hit = true;
                            getLevel().m_lara->m_state.health -= 100;
                            m_state.required_anim_state = 1_as;
                        }
                    }
                }
                break;
            case 7:
                animTilt = animAngle;
                if( m_state.required_anim_state == 0_as && aiInfo.ahead )
                {
                    if( m_state.touch_bits.to_ulong() & 0xff7c00UL )
                    {
                        emitParticle( core::TRVec{0_len, 66_len, 318_len}, 22, &createBloodSplat );
                        getLevel().m_lara->m_state.is_hit = true;
                        getLevel().m_lara->m_state.health -= 100;
                        m_state.required_anim_state = 3_as;
                    }
                }
                break;
            case 8:
                animTilt = animAngle;
                if( m_state.required_anim_state == 0_as && (m_state.touch_bits.to_ulong() & 0xff7c00UL) )
                {
                    emitParticle( core::TRVec{0_len, 66_len, 318_len}, 22, &createBloodSplat );
                    getLevel().m_lara->m_state.is_hit = true;
                    getLevel().m_lara->m_state.health -= 100;
                    m_state.required_anim_state = 1_as;
                }
                break;
            default:
                break;
        }
    }
    else if( m_state.current_anim_state != 5_as )
    {
        m_state.anim = getLevel().findAnimatedModelForType( TR1ItemId::Raptor )->animations + 9
                       + util::rand15( 3 );
        m_state.current_anim_state = 5_as;
        m_state.frame_number = m_state.anim->firstFrame;
    }

    rotateCreatureTilt( animTilt );
    rotateCreatureHead( animHead );
    getSkeleton()->patchBone( 20, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix() );
    animateCreature( animAngle, animTilt );
}
}
}
