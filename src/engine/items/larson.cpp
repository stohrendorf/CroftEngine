#include "larson.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
void Larson::update()
{
    if( m_state.triggerState == TriggerState::Invisible )
    {
        m_state.triggerState = TriggerState::Active;
    }

    m_state.initCreatureInfo( getLevel() );

    core::Angle tiltRot = 0_deg;
    core::Angle creatureTurn = 0_deg;
    core::Angle headRot = 0_deg;
    if( m_state.health > 0 )
    {
        const ai::AiInfo aiInfo{getLevel(), m_state};
        if( aiInfo.ahead )
        {
            headRot = aiInfo.angle;
        }

        updateMood( getLevel(), m_state, aiInfo, false );

        creatureTurn = rotateTowardsTarget( m_state.creatureInfo->maximum_turn );
        switch( m_state.current_anim_state.get() )
        {
            case 1:                                   // standing holding gun
                if( m_state.required_anim_state != 0_as )
                {
                    m_state.goal_anim_state = m_state.required_anim_state;
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Bored )
                {
                    if( util::rand15() >= 96 )
                    {
                        m_state.goal_anim_state = 2_as; // walking
                    }
                    else
                    {
                        m_state.goal_anim_state = 6_as; // standing
                    }
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Escape )
                {
                    m_state.goal_anim_state = 3_as;      // running
                }
                else
                {
                    m_state.goal_anim_state = 2_as;      // walking
                }
                break;
            case 2:                                   // walking
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
            case 3:                                   // running
                m_state.creatureInfo->maximum_turn = 6_deg;
                tiltRot = creatureTurn / 2;
                if( m_state.creatureInfo->mood == ai::Mood::Bored && util::rand15() < 96 )
                {
                    m_state.required_anim_state = 6_as;
                    m_state.goal_anim_state = 1_as;
                }
                else if( canShootAtLara( aiInfo ) )
                {
                    m_state.required_anim_state = 4_as;
                    m_state.goal_anim_state = 1_as;
                }
                else if( aiInfo.ahead && aiInfo.distance < util::square( 3 * core::SectorSize ) )
                {
                    m_state.required_anim_state = 2_as;
                    m_state.goal_anim_state = 1_as;
                }
                break;
            case 4:                                   // aiming
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
            case 6:                                   // standing
                if( m_state.creatureInfo->mood != ai::Mood::Bored )
                {
                    m_state.goal_anim_state = 1_as;          // standing/holding gun
                }
                else if( util::rand15() < 96 )
                {
                    m_state.required_anim_state = 2_as;      // walking
                    m_state.goal_anim_state = 1_as;
                }
                break;
            case 7:                                   // firing
                if( m_state.required_anim_state == 0_as )
                {
                    if( tryShootAtLara( *this, aiInfo.distance, core::TRVec{-60_len, 170_len, 0_len}, 14, headRot ) )
                    {
                        getLevel().m_lara->m_state.health -= 50;
                        getLevel().m_lara->m_state.is_hit = true;
                    }
                    m_state.required_anim_state = 4_as;
                }
                if( m_state.creatureInfo->mood == ai::Mood::Escape )
                {
                    m_state.required_anim_state = 1_as;
                }
                break;
            default:
                break;
        }
    }
    else if( m_state.current_anim_state != 5_as )   // injured/dying
    {
        m_state.anim = &getLevel().findAnimatedModelForType( engine::TR1ItemId::Larson )->animations[15];
        m_state.frame_number = m_state.anim->firstFrame;
        m_state.current_anim_state = 5_as;
    }
    rotateCreatureTilt( tiltRot );
    rotateCreatureHead( headRot );
    getSkeleton()->patchBone( 7, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix() );
    animateCreature( creatureTurn, 0_deg );
}

Larson::Larson(const gsl::not_null<level::Level*>& level,
               const gsl::not_null<const loader::Room*>& room,
               const loader::Item& item,
               const loader::SkeletalModelType& animatedModel)
        : AIAgent{level, room, item, animatedModel}
{
}
}
}
