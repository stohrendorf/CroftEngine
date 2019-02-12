#include "trex.h"

#include "engine/laranode.h"
#include "engine/cameracontroller.h"

namespace engine
{
namespace items
{
void TRex::update()
{
    if( m_state.triggerState == TriggerState::Invisible )
    {
        m_state.triggerState = TriggerState::Active;
    }

    m_state.initCreatureInfo( getLevel() );

    core::Angle rotationToMoveTarget;

    core::Angle creatureHead = 0_deg;
    if( getHealth() > 0_hp )
    {
        const ai::AiInfo aiInfo{getLevel(), m_state};
        if( aiInfo.ahead )
        {
            creatureHead = aiInfo.angle;
        }
        updateMood( getLevel(), m_state, aiInfo, true );

        rotationToMoveTarget = rotateTowardsTarget( m_state.creatureInfo->maximum_turn );
        if( m_state.touch_bits.any() )
        {
            if( m_state.current_anim_state == 3_as )
            {
                getLevel().m_lara->m_state.health -= 10_hp;
            }
            else
            {
                getLevel().m_lara->m_state.health -= 1_hp;
            }
        }

        m_state.creatureInfo->flags = m_state.creatureInfo->mood != ai::Mood::Escape
                                      && !aiInfo.ahead
                                      && aiInfo.enemy_facing > -90_deg
                                      && aiInfo.enemy_facing < 90_deg;
        if( m_state.creatureInfo->flags == 0
            && aiInfo.distance > util::square( 1500_len )
            && aiInfo.distance < util::square( 4 * core::SectorSize )
            && aiInfo.bite )
        {
            m_state.creatureInfo->flags = 1;
        }

        switch( m_state.current_anim_state.get() )
        {
            case 1:
                if( m_state.required_anim_state != 0_as )
                {
                    m_state.goal_anim_state = m_state.required_anim_state;
                }
                else if( aiInfo.distance < util::square( 1500_len ) && aiInfo.bite )
                {
                    m_state.goal_anim_state = 7_as;
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Bored && m_state.creatureInfo->flags == 0 )
                {
                    m_state.goal_anim_state = 3_as;
                }
                else
                {
                    m_state.goal_anim_state = 2_as;
                }
                break;
            case 2:
                m_state.creatureInfo->maximum_turn = 2_deg;
                if( m_state.creatureInfo->mood != ai::Mood::Bored || m_state.creatureInfo->flags == 0 )
                {
                    m_state.goal_anim_state = 1_as;
                }
                else if( aiInfo.ahead && util::rand15() < 512 )
                {
                    m_state.required_anim_state = 6_as;
                    m_state.goal_anim_state = 1_as;
                }
                break;
            case 3:
                m_state.creatureInfo->maximum_turn = 4_deg;
                if( aiInfo.distance < util::square( 5 * core::SectorSize ) && aiInfo.bite )
                {
                    m_state.goal_anim_state = 1_as;
                }
                else if( m_state.creatureInfo->flags )
                {
                    m_state.goal_anim_state = 1_as;
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Escape && aiInfo.ahead && util::rand15() < 512 )
                {
                    m_state.required_anim_state = 6_as;
                    m_state.goal_anim_state = 1_as;
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Bored )
                {
                    m_state.goal_anim_state = 1_as;
                }
                break;
            case 7:
                if( m_state.touch_bits.to_ulong() & 0x3000UL )
                {
                    m_state.goal_anim_state = 8_as;

                    getLevel().m_lara->m_state.is_hit = true;
                    getLevel().m_lara->m_state.falling = false;

                    getLevel().m_lara->setCurrentRoom( m_state.position.room );
                    getLevel().m_lara->m_state.position.position = m_state.position.position;
                    getLevel().m_lara->m_state.rotation.X = 0_deg;
                    getLevel().m_lara->m_state.rotation.Y = m_state.rotation.Y;
                    getLevel().m_lara->m_state.rotation.Z = 0_deg;
                    getLevel().m_lara->m_state.anim = &getLevel()
                            .findAnimatedModelForType( engine::TR1ItemId::AlternativeLara )->animations[1];
                    getLevel().m_lara->m_state.frame_number = getLevel().m_lara->m_state.anim->firstFrame;
                    getLevel().m_lara->setCurrentAnimState( LaraStateId::BoulderDeath );
                    getLevel().m_lara->setGoalAnimState( LaraStateId::BoulderDeath );
                    getLevel().m_lara->setHandStatus( HandStatus::Grabbing );
                    getLevel().m_lara->gunType = LaraNode::WeaponId::None;
                    getLevel().m_cameraController->setModifier( CameraModifier::FollowCenter );
                    getLevel().m_cameraController->setRotationAroundCenter( -25_deg, 170_deg );
                    getLevel().m_lara->m_state.health = -1_hp;
                    getLevel().m_lara->setAir( -1_frame );
                    getLevel().useAlternativeLaraAppearance( true );
                }
                m_state.required_anim_state = 2_as;
                break;
            default:
                break;
        }
    }
    else if( m_state.current_anim_state == 1_as )
    {
        m_state.goal_anim_state = 5_as;
    }
    else
    {
        m_state.goal_anim_state = 1_as;
    }

    rotateCreatureHead( creatureHead );
    m_state.creatureInfo->neck_rotation = m_state.creatureInfo->head_rotation;
    getSkeleton()->patchBone( 11, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix() );
    getSkeleton()->patchBone( 12, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix() );
    animateCreature( rotationToMoveTarget, 0_deg );
    m_state.collidable = true;
}
}
}
