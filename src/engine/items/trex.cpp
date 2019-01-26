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
    if( getHealth() > 0 )
    {
        const ai::AiInfo aiInfo{getLevel(), m_state};
        if( aiInfo.ahead )
        {
            creatureHead = aiInfo.angle;
        }
        updateMood( getLevel(), m_state, aiInfo, true );

        rotationToMoveTarget = rotateTowardsTarget( m_state.creatureInfo->maximum_turn );
        if( m_state.touch_bits )
        {
            if( m_state.current_anim_state == 3 )
            {
                getLevel().m_lara->m_state.health -= 10;
            }
            else
            {
                getLevel().m_lara->m_state.health -= 1;
            }
        }

        m_state.creatureInfo->flags = m_state.creatureInfo->mood != ai::Mood::Escape
                                      && !aiInfo.ahead
                                      && aiInfo.enemy_facing > -90_deg
                                      && aiInfo.enemy_facing < 90_deg;
        if( m_state.creatureInfo->flags == 0
            && aiInfo.distance > util::square( 1500 )
            && aiInfo.distance < util::square( 4 * loader::SectorSize )
            && aiInfo.bite )
        {
            m_state.creatureInfo->flags = 1;
        }

        switch( m_state.current_anim_state )
        {
            case 1:
                if( m_state.required_anim_state != 0 )
                {
                    m_state.goal_anim_state = m_state.required_anim_state;
                }
                else if( aiInfo.distance < util::square( 1500 ) && aiInfo.bite )
                {
                    m_state.goal_anim_state = 7;
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Bored && !m_state.creatureInfo->flags )
                {
                    m_state.goal_anim_state = 3;
                }
                else
                {
                    m_state.goal_anim_state = 2;
                }
                break;
            case 2:
                m_state.creatureInfo->maximum_turn = 2_deg;
                if( m_state.creatureInfo->mood != ai::Mood::Bored || !m_state.creatureInfo->flags )
                {
                    m_state.goal_anim_state = 1;
                }
                else if( aiInfo.ahead && util::rand15() < 512 )
                {
                    m_state.required_anim_state = 6;
                    m_state.goal_anim_state = 1;
                }
                break;
            case 3:
                m_state.creatureInfo->maximum_turn = 4_deg;
                if( aiInfo.distance < util::square( 5 * loader::SectorSize ) && aiInfo.bite )
                {
                    m_state.goal_anim_state = 1;
                }
                else if( m_state.creatureInfo->flags )
                {
                    m_state.goal_anim_state = 1;
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Escape && aiInfo.ahead && util::rand15() < 512 )
                {
                    m_state.required_anim_state = 6;
                    m_state.goal_anim_state = 1;
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Bored )
                {
                    m_state.goal_anim_state = 1;
                }
                break;
            case 7:
                if( m_state.touch_bits & 0x3000 )
                {
                    m_state.goal_anim_state = 8;

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
                    getLevel().m_lara->setCurrentAnimState( loader::LaraStateId::BoulderDeath );
                    getLevel().m_lara->setGoalAnimState( loader::LaraStateId::BoulderDeath );
                    getLevel().m_lara->setHandStatus( HandStatus::Grabbing );
                    getLevel().m_lara->gunType = LaraNode::WeaponId::None;
                    getLevel().m_cameraController->setModifier( CameraModifier::FollowCenter );
                    getLevel().m_cameraController->setRotationAroundCenter( -25_deg, 170_deg );
                    getLevel().m_lara->m_state.health = -1;
                    getLevel().m_lara->setAir( -1 );
                    getLevel().useAlternativeLaraAppearance( true );
                }
                m_state.required_anim_state = 2;
                break;
            default:
                break;
        }
    }
    else if( m_state.current_anim_state == 1 )
    {
        m_state.goal_anim_state = 5;
    }
    else
    {
        m_state.goal_anim_state = 1;
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
