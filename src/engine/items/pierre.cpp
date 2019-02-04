#include "pierre.h"

#include "engine/laranode.h"
#include "pickupitem.h"

#include "engine/cameracontroller.h"

namespace engine
{
namespace items
{
void Pierre::update()
{
    if( getLevel().m_pierre == nullptr )
    {
        getLevel().m_pierre = this;
    }
    else if( this != getLevel().m_pierre )
    {
        if( m_state.activationState.isOneshot() )
        {
            if( getLevel().m_pierre != nullptr )
                getLevel().m_pierre->kill();

            getLevel().m_pierre = nullptr;
        }
        else
        {
            kill();
        }
    }

    if( m_state.triggerState == TriggerState::Invisible )
    {
        m_state.triggerState = TriggerState::Active;
    }

    m_state.initCreatureInfo( getLevel() );

    core::Angle tiltRot = 0_deg;
    core::Angle creatureTurn = 0_deg;
    core::Angle headRot = 0_deg;
    if ( m_state.health <= 40 && !m_state.activationState.isOneshot() )
    {
        m_state.health = 40;
        ++m_state.creatureInfo->flags;
    }
    if( m_state.health > 0 )
    {
        ai::AiInfo aiInfo{getLevel(), m_state};
        if( aiInfo.ahead )
        {
            headRot = aiInfo.angle;
        }
        if( m_state.creatureInfo->flags != 0 )
        {
            aiInfo.enemy_zone = -1;
            m_state.is_hit = true;
        }

        updateMood( getLevel(), m_state, aiInfo, false );

        creatureTurn = rotateTowardsTarget( m_state.creatureInfo->maximum_turn );
        switch( m_state.current_anim_state.get() )
        {
            case 1:
                if( m_state.required_anim_state != 0_as )
                {
                    m_state.goal_anim_state = m_state.required_anim_state;
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Bored )
                {
                    if( util::rand15() >= 96 )
                    {
                        m_state.goal_anim_state = 2_as;
                    }
                    else
                    {
                        m_state.goal_anim_state = 6_as;
                    }
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Escape )
                {
                    m_state.goal_anim_state = 3_as;
                }
                else
                {
                    m_state.goal_anim_state = 2_as;
                }
                break;
            case 2:
                m_state.creatureInfo->maximum_turn = 3_deg;
                if( m_state.creatureInfo->mood == ai::Mood::Bored && util::rand15() < 96 )
                {
                    m_state.required_anim_state = 6_as;
                    m_state.goal_anim_state = 1_as;
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Escape )
                {
                    m_state.required_anim_state = 3_as;
                    m_state.goal_anim_state = 1_as;
                }
                else if( canShootAtLara( aiInfo ) )
                {
                    m_state.required_anim_state = 4_as;
                    m_state.goal_anim_state = 1_as;
                }
                else if( !aiInfo.ahead || aiInfo.distance > util::square( 3 * core::SectorSize ) )
                {
                    m_state.required_anim_state = 3_as;
                    m_state.goal_anim_state = 1_as;
                }
                break;
            case 3:
                m_state.creatureInfo->maximum_turn = 6_deg;
                tiltRot = creatureTurn / 2;
                if( m_state.creatureInfo->mood != ai::Mood::Bored || util::rand15() >= 96 )
                {
                    if( canShootAtLara( aiInfo ) )
                    {
                        m_state.required_anim_state = 4_as;
                        m_state.goal_anim_state = 1_as;
                    }
                    else if( aiInfo.ahead && aiInfo.distance < util::square( 3 * core::SectorSize ) )
                    {
                        m_state.required_anim_state = 2_as;
                        m_state.goal_anim_state = 1_as;
                    }
                }
                else
                {
                    m_state.required_anim_state = 6_as;
                    m_state.goal_anim_state = 1_as;
                }
                break;
            case 4:
                if( m_state.required_anim_state != 0_as )
                {
                    m_state.goal_anim_state = m_state.required_anim_state;
                }
                else if( canShootAtLara( aiInfo ) )
                {
                    m_state.goal_anim_state = 7_as;
                }
                else
                {
                    m_state.goal_anim_state = 1_as;
                }
                break;
            case 6:
                if( m_state.creatureInfo->mood != ai::Mood::Bored )
                {
                    m_state.goal_anim_state = 1_as;
                }
                else if( util::rand15() < 96 )
                {
                    m_state.required_anim_state = 2_as;
                    m_state.goal_anim_state = 1_as;
                }
                break;
            case 7:
                if( m_state.required_anim_state == 0_as )
                {
                    if( tryShootAtLara( *this, aiInfo.distance, {60_len, 200_len, 0_len}, 11, headRot ) )
                    {
                        getLevel().m_lara->m_state.health -= 25;
                        getLevel().m_lara->m_state.is_hit = true;
                    }
                    if( tryShootAtLara( *this, aiInfo.distance, {-57_len, 200_len, 0_len}, 14, headRot ) )
                    {
                        getLevel().m_lara->m_state.health -= 25;
                        getLevel().m_lara->m_state.is_hit = true;
                    }
                    m_state.required_anim_state = 4_as;
                }
                if( m_state.creatureInfo->mood == ai::Mood::Escape && util::rand15() > 8192 )
                {
                    m_state.required_anim_state = 1_as;
                }
                break;
            default:
                break;
        }
    }
    else if( m_state.current_anim_state != 5_as ) // injured/dying
    {
        m_state.anim = &getLevel().findAnimatedModelForType( TR1ItemId::Pierre )->animations[12];
        m_state.frame_number = m_state.anim->firstFrame;
        m_state.current_anim_state = 5_as;
        getLevel().createPickup( TR1ItemId::MagnumsSprite, m_state.position.room, m_state.position.position );
        getLevel().createPickup( TR1ItemId::ScionPiece2, m_state.position.room, m_state.position.position );
        getLevel().createPickup( TR1ItemId::Key1Sprite, m_state.position.room, m_state.position.position );
    }
    rotateCreatureTilt( tiltRot );
    rotateCreatureHead( headRot );
    animateCreature( creatureTurn, 0_deg );
    getSkeleton()->patchBone( 7, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix() );
    if( m_state.creatureInfo->flags != 0 )
    {
        auto camPos = m_state.position;
        camPos.position.Y -= core::SectorSize;
        auto target = getLevel().m_cameraController->getTRPosition();
        if( engine::CameraController::clampPosition( target, camPos, getLevel() ) )
        {
            m_state.creatureInfo->flags = 1;
        }
        else if( m_state.creatureInfo->flags > 10 )
        {
            m_state.health = -16384;
            m_state.creatureInfo = nullptr;
            kill();
            getLevel().m_pierre = nullptr;
        }
    }
    if( getWaterSurfaceHeight().is_initialized() )
    {
        m_state.health = -16384;
        m_state.creatureInfo = nullptr;
        kill();
        getLevel().m_pierre = nullptr;
    }
}

Pierre::Pierre(const gsl::not_null<level::Level*>& level,
               const gsl::not_null<const loader::Room*>& room,
               const loader::Item& item,
               const loader::SkeletalModelType& animatedModel)
        : AIAgent{level, room, item, animatedModel}
{
}
}
}
