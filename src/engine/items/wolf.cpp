#include "wolf.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
void Wolf::update()
{
    if( m_state.triggerState == TriggerState::Invisible )
    {
        m_state.triggerState = TriggerState::Active;
    }

    m_state.initCreatureInfo( getLevel() );

    static constexpr const uint16_t Walking = 1;
    static constexpr const uint16_t Running = 2;
    static constexpr const uint16_t Jumping = 3;
    static constexpr const uint16_t Stalking = 5;
    static constexpr const uint16_t JumpAttack = 6;
    static constexpr const uint16_t Attacking = 7;
    static constexpr const uint16_t LyingDown = 8;
    static constexpr const uint16_t PrepareToStrike = 9;
    static constexpr const uint16_t RunningJump = 10;
    static constexpr const uint16_t Dying = 11;
    static constexpr const uint16_t Biting = 12;

    core::Angle pitch = 0_deg;
    core::Angle roll = 0_deg;
    core::Angle rotationToMoveTarget = 0_deg;
    if( getHealth() > 0 )
    {
        ai::AiInfo aiInfo{getLevel(), m_state};

        if( aiInfo.ahead )
        {
            pitch = aiInfo.angle;
        }

        updateMood( getLevel(), m_state, aiInfo, false );
        rotationToMoveTarget = rotateTowardsTarget( m_state.creatureInfo->maximum_turn );
        switch( m_state.current_anim_state )
        {
            case LyingDown:
                pitch = 0_deg;
                if( m_state.creatureInfo->mood != ai::Mood::Escape && aiInfo.enemy_zone != aiInfo.zone_number )
                {
                    if( util::rand15() < 32 )
                    {
                        m_state.required_anim_state = Running;
                        m_state.goal_anim_state = Walking;
                    }
                }
                else
                {
                    m_state.required_anim_state = PrepareToStrike;
                    m_state.goal_anim_state = Walking;
                }
                break;
            case Walking:
                if( m_state.required_anim_state != 0 )
                {
                    m_state.goal_anim_state = m_state.required_anim_state;
                    m_state.required_anim_state = 0;
                }
                else
                {
                    m_state.goal_anim_state = Running;
                }
                break;
            case Running:
                m_state.creatureInfo->maximum_turn = 2_deg;
                if( m_state.creatureInfo->mood != ai::Mood::Bored )
                {
                    m_state.goal_anim_state = Stalking;
                    m_state.required_anim_state = 0;
                }
                else if( util::rand15() < 32 )
                {
                    m_state.goal_anim_state = Walking;
                    m_state.required_anim_state = LyingDown;
                }
                break;
            case PrepareToStrike:
                if( m_state.required_anim_state != 0 )
                {
                    m_state.goal_anim_state = m_state.required_anim_state;
                    m_state.required_anim_state = 0;
                    break;
                }
                if( m_state.creatureInfo->mood == ai::Mood::Escape )
                {
                    m_state.goal_anim_state = Jumping;
                }
                else if( aiInfo.distance < util::square( 345 ) && aiInfo.bite )
                {
                    m_state.goal_anim_state = Biting;
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Stalk )
                {
                    m_state.goal_anim_state = Stalking;
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Bored )
                {
                    m_state.goal_anim_state = Jumping;
                }
                else
                {
                    m_state.goal_anim_state = Walking;
                }
                break;
            case Stalking:
                m_state.creatureInfo->maximum_turn = 2_deg;
                if( m_state.creatureInfo->mood == ai::Mood::Escape )
                {
                    m_state.goal_anim_state = Jumping;
                }
                else if( aiInfo.distance < util::square( 345 ) && aiInfo.bite )
                {
                    m_state.goal_anim_state = Biting;
                }
                else if( aiInfo.distance <= util::square( 3 * loader::SectorSize ) )
                {
                    if( m_state.creatureInfo->mood == ai::Mood::Attack )
                    {
                        if( !aiInfo.ahead
                            || aiInfo.distance > util::square( 3 * loader::SectorSize / 2 )
                            || (aiInfo.enemy_facing < 90_deg && aiInfo.enemy_facing > -90_deg) )
                        {
                            m_state.goal_anim_state = Jumping;
                        }
                    }
                    else if( util::rand15() >= 384 )
                    {
                        if( m_state.creatureInfo->mood == ai::Mood::Bored )
                        {
                            m_state.goal_anim_state = PrepareToStrike;
                        }
                    }
                    else
                    {
                        m_state.goal_anim_state = PrepareToStrike;
                        m_state.required_anim_state = Attacking;
                    }
                }
                else
                {
                    m_state.goal_anim_state = Jumping;
                }
                break;
            case Jumping:
                m_state.creatureInfo->maximum_turn = 5_deg;
                roll = rotationToMoveTarget;
                if( aiInfo.ahead && aiInfo.distance < util::square( 3 * loader::SectorSize / 2 ) )
                {
                    if( aiInfo.distance <= util::square( 3 * loader::SectorSize / 2 ) / 2
                        || (aiInfo.enemy_facing <= 90_deg && aiInfo.enemy_facing >= -90_deg) )
                    {
                        m_state.goal_anim_state = JumpAttack;
                        m_state.required_anim_state = 0;
                    }
                    else
                    {
                        m_state.goal_anim_state = PrepareToStrike;
                        m_state.required_anim_state = Stalking;
                    }
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Stalk
                         || aiInfo.distance >= util::square( 3 * loader::SectorSize ) )
                {
                    if( m_state.creatureInfo->mood == ai::Mood::Bored )
                    {
                        m_state.goal_anim_state = PrepareToStrike;
                    }
                }
                else
                {
                    m_state.goal_anim_state = PrepareToStrike;
                    m_state.required_anim_state = Stalking;
                }
                break;
            case JumpAttack:
                roll = rotationToMoveTarget;
                if( m_state.required_anim_state == 0 && (m_state.touch_bits & 0x774f) )
                {
                    emitParticle( core::TRVec{0, -14, 174}, 6, &engine::createBloodSplat );
                    getLevel().m_lara->m_state.is_hit = true;
                    getLevel().m_lara->m_state.health -= 50;
                    m_state.required_anim_state = Jumping;
                }
                m_state.goal_anim_state = Jumping;
                break;
            case Biting:
                if( m_state.required_anim_state == 0 && (m_state.touch_bits & 0x774f) && aiInfo.ahead )
                {
                    emitParticle( core::TRVec{0, -14, 174}, 6, &engine::createBloodSplat );
                    getLevel().m_lara->m_state.is_hit = true;
                    getLevel().m_lara->m_state.health -= 100;
                    m_state.required_anim_state = PrepareToStrike;
                }
                break;
            default:
                break;
        }
    }
    else if( m_state.current_anim_state != Dying )
    {
        const auto r = util::rand15( 3 );
        getSkeleton()->setAnimIdGlobal(
                m_state,
                to_not_null( &getLevel().m_animatedModels[m_state.object_number]->animation[20 + r] ),
                0 );
        BOOST_ASSERT( m_state.current_anim_state == Dying );
    }
    rotateCreatureTilt( roll );
    rotateCreatureHead( pitch );
    getSkeleton()->patchBone( 3, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix() );
    animateCreature( rotationToMoveTarget, roll );
}
}
}
