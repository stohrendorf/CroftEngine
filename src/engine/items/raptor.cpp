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
        ai::AiInfo aiInfo{getLevel(), m_state};
        if( aiInfo.ahead )
        {
            animHead = aiInfo.angle;
        }
        ai::updateMood( getLevel(), m_state, aiInfo, true );
        animAngle = rotateTowardsTarget( m_state.creatureInfo->maximum_turn );
        switch( m_state.current_anim_state )
        {
            case 1:
                if( m_state.required_anim_state != 0 )
                {
                    m_state.goal_anim_state = m_state.required_anim_state;
                }
                else if( m_state.touch_bits & 0xff7c00 )
                {
                    m_state.goal_anim_state = 8;
                }
                else if( aiInfo.distance < 462400 && aiInfo.bite )
                {
                    m_state.goal_anim_state = 8;
                }
                else if( aiInfo.bite && aiInfo.distance < 2359296 )
                {
                    m_state.goal_anim_state = 4;
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Bored )
                {
                    m_state.goal_anim_state = 3;
                }
                else
                {
                    m_state.goal_anim_state = 2;
                }
                break;
            case 2:
                m_state.creatureInfo->maximum_turn = 1_deg;
                if( m_state.creatureInfo->mood != ai::Mood::Bored )
                {
                    m_state.goal_anim_state = 1;
                }
                else if( aiInfo.ahead )
                {
                    if( util::rand15() < 256 )
                    {
                        m_state.required_anim_state = 6;
                        m_state.goal_anim_state = 1;
                    }
                }
                break;
            case 3:
                m_state.creatureInfo->maximum_turn = 4_deg;
                animTilt = animAngle;
                if( m_state.touch_bits & 0xff7c00 )
                {
                    m_state.goal_anim_state = 1;
                }
                else if( aiInfo.bite && aiInfo.distance < 2359296 )
                {
                    if( m_state.goal_anim_state == 3 )
                    {
                        if( util::rand15() >= 8192 )
                        {
                            m_state.goal_anim_state = 7;
                        }
                        else
                        {
                            m_state.goal_anim_state = 1;
                        }
                    }
                }
                else if( aiInfo.ahead && m_state.creatureInfo->mood != ai::Mood::Escape && util::rand15() < 256 )
                {
                    m_state.required_anim_state = 6;
                    m_state.goal_anim_state = 1;
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Bored )
                {
                    m_state.goal_anim_state = 1;
                }
                break;
            case 4:
                animTilt = animAngle;
                if( m_state.required_anim_state == 0 )
                {
                    if( aiInfo.ahead )
                    {
                        if( m_state.touch_bits & 0xff7c00 )
                        {
                            emitParticle(core::TRCoordinates{0, 66, 318}, 22, &createBloodSplat);
                            getLevel().m_lara->m_state.is_hit = true;
                            getLevel().m_lara->m_state.health -= 100;
                            m_state.required_anim_state = 1;
                        }
                    }
                }
                break;
            case 7:
                animTilt = animAngle;
                if( m_state.required_anim_state == 0 && aiInfo.ahead )
                {
                    if( m_state.touch_bits & 0xff7c00 )
                    {
                        emitParticle(core::TRCoordinates{0, 66, 318}, 22, &createBloodSplat);
                        getLevel().m_lara->m_state.is_hit = true;
                        getLevel().m_lara->m_state.health -= 100;
                        m_state.required_anim_state = 3;
                    }
                }
                break;
            case 8:
                animTilt = animAngle;
                if( m_state.required_anim_state == 0 && m_state.touch_bits & 0xff7c00 )
                {
                    emitParticle(core::TRCoordinates{0, 66, 318}, 22, &createBloodSplat);
                    getLevel().m_lara->m_state.is_hit = true;
                    getLevel().m_lara->m_state.health -= 100;
                    m_state.required_anim_state = 1;
                }
                break;
            default:
                break;
        }
    }
    else if( m_state.current_anim_state != 5 )
    {
        m_state.anim = getLevel().findAnimatedModelForType( engine::TR1ItemId::Raptor )->animation + 9
                       + util::rand15( 3 );
        m_state.current_anim_state = 5;
        m_state.frame_number = m_state.anim->firstFrame;
    }

    rotateCreatureTilt( animTilt );
    rotateCreatureHead( animHead );
    getSkeleton()->patchBone( 20, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix() );
    animateCreature( animAngle, animTilt );
}
}
}
