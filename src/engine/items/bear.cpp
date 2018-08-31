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

    static constexpr uint16_t Walking = 0;
    static constexpr uint16_t GettingDown = 1;
    static constexpr uint16_t WalkingTall = 2;
    static constexpr uint16_t Running = 3;
    static constexpr uint16_t RoaringStanding = 4;
    static constexpr uint16_t Growling = 5;
    static constexpr uint16_t RunningAttack = 6;
    static constexpr uint16_t Standing = 7;
    static constexpr uint16_t Biting = 8;
    static constexpr uint16_t Dying = 9;

    if( getHealth() > 0 )
    {
        ai::AiInfo aiInfo{getLevel(), m_state};
        ai::updateMood( getLevel(), m_state, aiInfo, true );

        rotationToMoveTarget = rotateTowardsTarget( m_state.creatureInfo->maximum_turn );
        if( m_state.is_hit )
            m_state.creatureInfo->flags = 1;

        switch( m_state.current_anim_state )
        {
            case Walking:
                m_state.creatureInfo->maximum_turn = 2_deg;
                if( getLevel().m_lara->m_state.health <= 0 && (m_state.touch_bits & 0x2406c) != 0 && aiInfo.ahead )
                {
                    m_state.goal_anim_state = GettingDown;
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Bored )
                {
                    m_state.goal_anim_state = GettingDown;
                    if( m_state.creatureInfo->mood == ai::Mood::Escape )
                    {
                        m_state.required_anim_state = 0;
                    }
                }
                else if( util::rand15() < 80 )
                {
                    m_state.required_anim_state = Growling;
                    m_state.goal_anim_state = GettingDown;
                }
                break;
            case GettingDown:
                if( getLevel().m_lara->m_state.health <= 0 )
                {
                    if( aiInfo.bite && aiInfo.distance < util::square( 768 ) )
                    {
                        m_state.goal_anim_state = Biting;
                    }
                    else
                    {
                        m_state.goal_anim_state = Walking;
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
                        m_state.goal_anim_state = Running;
                    }
                    else
                    {
                        m_state.goal_anim_state = Walking;
                    }
                }
                break;
            case WalkingTall:
                if( m_state.creatureInfo->flags != 0 )
                {
                    m_state.required_anim_state = 0;
                    m_state.goal_anim_state = RoaringStanding;
                }
                else if( aiInfo.ahead && (m_state.touch_bits & 0x2406c) != 0 )
                {
                    m_state.goal_anim_state = RoaringStanding;
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Escape )
                {
                    m_state.required_anim_state = 0;
                    m_state.goal_anim_state = RoaringStanding;
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Bored && util::rand15() >= 80 )
                {
                    if( aiInfo.distance > util::square( 2048 ) || util::rand15() < 1536 )
                    {
                        m_state.required_anim_state = GettingDown;
                        m_state.goal_anim_state = RoaringStanding;
                    }
                }
                else
                {
                    m_state.required_anim_state = Growling;
                    m_state.goal_anim_state = RoaringStanding;
                }
                break;
            case Running:
                m_state.creatureInfo->maximum_turn = 5_deg;
                if( (m_state.touch_bits & 0x2406c) != 0 )
                {
                    getLevel().m_lara->m_state.health -= 3;
                    getLevel().m_lara->m_state.is_hit = true;
                }
                if( m_state.creatureInfo->mood == ai::Mood::Bored || getLevel().m_lara->m_state.health <= 0 )
                {
                    m_state.goal_anim_state = GettingDown;
                }
                else if( aiInfo.ahead && m_state.required_anim_state == 0 )
                {
                    if( m_state.creatureInfo->flags == 0 && aiInfo.distance < util::square( 2048 )
                        && util::rand15() < 768 )
                    {
                        m_state.required_anim_state = RoaringStanding;
                        m_state.goal_anim_state = GettingDown;
                    }
                    else if( aiInfo.distance < util::square( 1024 ) )
                    {
                        m_state.goal_anim_state = RunningAttack;
                    }
                }
                break;
            case RoaringStanding:
                if( m_state.creatureInfo->flags != 0 )
                {
                    m_state.required_anim_state = 0;
                    m_state.goal_anim_state = GettingDown;
                }
                else if( m_state.required_anim_state != 0 )
                {
                    m_state.goal_anim_state = m_state.required_anim_state;
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Bored
                         || m_state.creatureInfo->mood == ai::Mood::Escape )
                {
                    m_state.goal_anim_state = GettingDown;
                }
                else if( aiInfo.bite && aiInfo.distance < util::square( 600 ) )
                {
                    m_state.goal_anim_state = Standing;
                }
                else
                {
                    m_state.goal_anim_state = WalkingTall;
                }
                break;
            case RunningAttack:
                if( m_state.required_anim_state == 0 && (m_state.touch_bits & 0x2406c) )
                {
                    emitParticle( core::TRCoordinates{0, 96, 335}, 14, &engine::createBloodSplat );
                    getLevel().m_lara->m_state.health -= 200;
                    getLevel().m_lara->m_state.is_hit = true;
                    m_state.required_anim_state = GettingDown;
                }
                break;
            case Standing:
                if( m_state.required_anim_state == 0 && (m_state.touch_bits & 0x2406c) )
                {
                    getLevel().m_lara->m_state.health -= 400;
                    getLevel().m_lara->m_state.is_hit = true;
                    m_state.required_anim_state = RoaringStanding;
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
            case Walking:
            case Running:
                m_state.goal_anim_state = GettingDown;
                break;
            case GettingDown:
                m_state.creatureInfo->flags = 0;
                m_state.goal_anim_state = Dying;
                break;
            case WalkingTall:
                m_state.goal_anim_state = RoaringStanding;
                break;
            case RoaringStanding:
                m_state.creatureInfo->flags = 1;
                m_state.goal_anim_state = Dying;
                break;
            case Dying:
                if( m_state.creatureInfo->flags != 0 && (m_state.touch_bits & 0x2406c) != 0 )
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
