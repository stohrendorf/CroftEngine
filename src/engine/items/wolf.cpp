#include "wolf.h"

#include "engine/laranode.h"
#include "engine/particle.h"

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

    m_state.initCreatureInfo( getEngine() );

    static constexpr const auto Walking = 1_as;
    static constexpr const auto Running = 2_as;
    static constexpr const auto Jumping = 3_as;
    static constexpr const auto Stalking = 5_as;
    static constexpr const auto JumpAttack = 6_as;
    static constexpr const auto Attacking = 7_as;
    static constexpr const auto LyingDown = 8_as;
    static constexpr const auto PrepareToStrike = 9_as;
    // static constexpr const uint16_t RunningJump = 10;
    static constexpr const auto Dying = 11_as;
    static constexpr const auto Biting = 12_as;

    core::Angle pitch = 0_deg;
    core::Angle roll = 0_deg;
    core::Angle rotationToMoveTarget = 0_deg;
    if( getHealth() > 0_hp )
    {
        const ai::AiInfo aiInfo{getEngine(), m_state};

        if( aiInfo.ahead )
        {
            pitch = aiInfo.angle;
        }

        updateMood( getEngine(), m_state, aiInfo, false );
        rotationToMoveTarget = rotateTowardsTarget( m_state.creatureInfo->maximum_turn );
        switch( m_state.current_anim_state.get() )
        {
            case LyingDown.get():
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
            case Walking.get():
                if( m_state.required_anim_state != 0_as )
                {
                    m_state.goal_anim_state = m_state.required_anim_state;
                    m_state.required_anim_state = 0_as;
                }
                else
                {
                    m_state.goal_anim_state = Running;
                }
                break;
            case Running.get():
                m_state.creatureInfo->maximum_turn = 2_deg;
                if( m_state.creatureInfo->mood != ai::Mood::Bored )
                {
                    m_state.goal_anim_state = Stalking;
                    m_state.required_anim_state = 0_as;
                }
                else if( util::rand15() < 32 )
                {
                    m_state.goal_anim_state = Walking;
                    m_state.required_anim_state = LyingDown;
                }
                break;
            case PrepareToStrike.get():
                if( m_state.required_anim_state != 0_as )
                {
                    m_state.goal_anim_state = m_state.required_anim_state;
                    m_state.required_anim_state = 0_as;
                    break;
                }
                if( m_state.creatureInfo->mood == ai::Mood::Escape )
                {
                    m_state.goal_anim_state = Jumping;
                }
                else if( aiInfo.distance < util::square( 345_len ) && aiInfo.bite )
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
            case Stalking.get():
                m_state.creatureInfo->maximum_turn = 2_deg;
                if( m_state.creatureInfo->mood == ai::Mood::Escape )
                {
                    m_state.goal_anim_state = Jumping;
                }
                else if( aiInfo.distance < util::square( 345_len ) && aiInfo.bite )
                {
                    m_state.goal_anim_state = Biting;
                }
                else if( aiInfo.distance <= util::square( 3 * core::SectorSize ) )
                {
                    if( m_state.creatureInfo->mood == ai::Mood::Attack )
                    {
                        if( !aiInfo.ahead
                            || aiInfo.distance > util::square( 3 * core::SectorSize / 2 )
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
            case Jumping.get():
                m_state.creatureInfo->maximum_turn = 5_deg;
                roll = rotationToMoveTarget;
                if( aiInfo.ahead && aiInfo.distance < util::square( 3 * core::SectorSize / 2 ) )
                {
                    if( aiInfo.distance <= util::square( 3 * core::SectorSize / 2 ) / 2
                        || (aiInfo.enemy_facing <= 90_deg && aiInfo.enemy_facing >= -90_deg) )
                    {
                        m_state.goal_anim_state = JumpAttack;
                        m_state.required_anim_state = 0_as;
                    }
                    else
                    {
                        m_state.goal_anim_state = PrepareToStrike;
                        m_state.required_anim_state = Stalking;
                    }
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Stalk
                         || aiInfo.distance >= util::square( 3 * core::SectorSize ) )
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
            case JumpAttack.get():
                roll = rotationToMoveTarget;
                if( m_state.required_anim_state == 0_as && (m_state.touch_bits.to_ulong() & 0x774fUL) )
                {
                    emitParticle( core::TRVec{0_len, -14_len, 174_len}, 6, &createBloodSplat );
                    getEngine().m_lara->m_state.is_hit = true;
                    getEngine().m_lara->m_state.health -= 50_hp;
                    m_state.required_anim_state = Jumping;
                }
                m_state.goal_anim_state = Jumping;
                break;
            case Biting.get():
                if( m_state.required_anim_state == 0_as && (m_state.touch_bits.to_ulong() & 0x774fUL) && aiInfo.ahead )
                {
                    emitParticle( core::TRVec{0_len, -14_len, 174_len}, 6, &createBloodSplat );
                    getEngine().m_lara->m_state.is_hit = true;
                    getEngine().m_lara->m_state.health -= 100_hp;
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
        getSkeleton()->setAnimation( m_state, &getEngine().findAnimatedModelForType(m_state.type)->animations[20 + r], 0_frame );
        BOOST_ASSERT( m_state.current_anim_state == Dying );
    }
    rotateCreatureTilt( roll );
    rotateCreatureHead( pitch );
    getSkeleton()->patchBone( 3, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix() );
    animateCreature( rotationToMoveTarget, roll );
}
}
}
