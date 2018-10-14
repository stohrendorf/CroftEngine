#include "lion.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
void Lion::update()
{
    if( m_state.triggerState == TriggerState::Invisible )
    {
        m_state.triggerState = TriggerState::Active;
    }

    m_state.initCreatureInfo( getLevel() );

    core::Angle tiltRot = 0_deg;
    core::Angle angle = 0_deg;
    core::Angle headRot = 0_deg;

    if( m_state.health > 0 )
    {
        const ai::AiInfo aiInfo{getLevel(), m_state};
        if( aiInfo.ahead )
        {
            headRot = aiInfo.angle;
        }
        updateMood( getLevel(), m_state, aiInfo, true );
        angle = rotateTowardsTarget( m_state.creatureInfo->maximum_turn );
        switch( m_state.current_anim_state )
        {
            case 1:
                if( m_state.required_anim_state != 0 )
                {
                    m_state.goal_anim_state = m_state.required_anim_state;
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Bored )
                {
                    if( aiInfo.ahead && m_state.touch_bits & 0x380066 )
                    {
                        m_state.goal_anim_state = 7;
                    }
                    else if( aiInfo.ahead && aiInfo.distance < util::square( loader::SectorSize ) )
                    {
                        m_state.goal_anim_state = 4;
                    }
                    else
                    {
                        m_state.goal_anim_state = 3;
                    }
                }
                else
                {
                    m_state.goal_anim_state = 2;
                }
                break;
            case 2:
                m_state.creatureInfo->maximum_turn = 2_deg;
                if( m_state.creatureInfo->mood != ai::Mood::Bored )
                {
                    m_state.goal_anim_state = 1;
                }
                else if( util::rand15() < 128 )
                {
                    m_state.required_anim_state = 6;
                    m_state.goal_anim_state = 1;
                }
                break;
            case 3:
                tiltRot = angle;
                m_state.creatureInfo->maximum_turn = 5_deg;
                if( m_state.creatureInfo->mood == ai::Mood::Bored )
                {
                    m_state.goal_anim_state = 1;
                }
                else if( aiInfo.ahead && aiInfo.distance < util::square( loader::SectorSize ) )
                {
                    m_state.goal_anim_state = 1;
                }
                else if( m_state.touch_bits & 0x380066 && aiInfo.ahead )
                {
                    m_state.goal_anim_state = 1;
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Escape && util::rand15() < 128 )
                {
                    m_state.required_anim_state = 6;
                    m_state.goal_anim_state = 1;
                }
                break;
            case 4:
                if( m_state.required_anim_state == 0 && m_state.touch_bits & 0x380066 )
                {
                    getLevel().m_lara->m_state.health -= 150;
                    getLevel().m_lara->m_state.is_hit = true;
                    m_state.required_anim_state = 1;
                }
                break;
            case 7:
                if( m_state.required_anim_state == 0 && m_state.touch_bits & 0x380066 )
                {
                    emitParticle( {-2, -10, 132}, 21, &createBloodSplat );
                    getLevel().m_lara->m_state.health -= 250;
                    getLevel().m_lara->m_state.is_hit = true;
                    m_state.required_anim_state = 1;
                }
                break;
            default:
                // silence compiler
                break;
        }
    }
    else
    {
        if( m_state.current_anim_state != 5 )
        {
            if( m_state.type == TR1ItemId::Panther )
            {
                m_state.anim = &getLevel().findAnimatedModelForType( TR1ItemId::Panther )->animations[4 + util::rand15(
                        2 )];
            }
            else if( m_state.type == TR1ItemId::LionMale )
            {
                m_state.anim = &getLevel().findAnimatedModelForType( TR1ItemId::LionMale )->animations[7 + util::rand15(
                        2 )];
                m_state.current_anim_state = 5;
                m_state.frame_number = m_state.anim->firstFrame;
            }
            else
            {
                m_state.anim = &getLevel().findAnimatedModelForType( TR1ItemId::LionFemale )
                                          ->animations[7 + util::rand15( 2 )];
                m_state.current_anim_state = 5;
                m_state.frame_number = m_state.anim->firstFrame;
            }
        }
    }

    rotateCreatureTilt( tiltRot );
    rotateCreatureHead( headRot );
    getSkeleton()->patchBone( 20, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix() );
    animateCreature( angle, tiltRot );
}
}
}
