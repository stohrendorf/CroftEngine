#include "bear.h"

#include "engine/laranode.h"
#include "engine/particle.h"

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

    m_state.initCreatureInfo( getEngine() );

    core::Angle rotationToMoveTarget;

    static constexpr auto Walking = 0_as;
    static constexpr auto GettingDown = 1_as;
    static constexpr auto WalkingTall = 2_as;
    static constexpr auto Running = 3_as;
    static constexpr auto RoaringStanding = 4_as;
    static constexpr auto Growling = 5_as;
    static constexpr auto RunningAttack = 6_as;
    static constexpr auto Standing = 7_as;
    static constexpr auto Biting = 8_as;
    static constexpr auto Dying = 9_as;

    if( getHealth() > 0_hp )
    {
        const ai::AiInfo aiInfo{getEngine(), m_state};
        updateMood( getEngine(), m_state, aiInfo, true );

        rotationToMoveTarget = rotateTowardsTarget( m_state.creatureInfo->maximum_turn );
        if( m_state.is_hit )
            m_state.creatureInfo->flags = 1;

        switch( m_state.current_anim_state.get() )
        {
            case Walking.get():
                m_state.creatureInfo->maximum_turn = 2_deg;
                if( getEngine().getLara().m_state.health <= 0_hp && (m_state.touch_bits.to_ulong() & 0x2406cUL) != 0
                    && aiInfo.ahead )
                {
                    m_state.goal_anim_state = GettingDown;
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Bored )
                {
                    m_state.goal_anim_state = GettingDown;
                    if( m_state.creatureInfo->mood == ai::Mood::Escape )
                    {
                        m_state.required_anim_state = 0_as;
                    }
                }
                else if( util::rand15() < 80 )
                {
                    m_state.required_anim_state = Growling;
                    m_state.goal_anim_state = GettingDown;
                }
                break;
            case GettingDown.get():
                if( getEngine().getLara().m_state.health <= 0_hp )
                {
                    if( aiInfo.bite && aiInfo.distance < util::square( 768_len ) )
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
                    if( m_state.required_anim_state != 0_as )
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
            case WalkingTall.get():
                if( m_state.creatureInfo->flags != 0 )
                {
                    m_state.required_anim_state = 0_as;
                    m_state.goal_anim_state = RoaringStanding;
                }
                else if( aiInfo.ahead && (m_state.touch_bits.to_ulong() & 0x2406cUL) != 0 )
                {
                    m_state.goal_anim_state = RoaringStanding;
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Escape )
                {
                    m_state.required_anim_state = 0_as;
                    m_state.goal_anim_state = RoaringStanding;
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Bored || util::rand15() < 80 )
                {
                    m_state.required_anim_state = Growling;
                    m_state.goal_anim_state = RoaringStanding;
                }
                else if( aiInfo.distance > util::square( 2 * core::SectorSize ) || util::rand15() < 1536 )
                {
                    m_state.required_anim_state = GettingDown;
                    m_state.goal_anim_state = RoaringStanding;
                }
                break;
            case Running.get():
                m_state.creatureInfo->maximum_turn = 5_deg;
                if( (m_state.touch_bits.to_ulong() & 0x2406cUL) != 0 )
                {
                    getEngine().getLara().m_state.health -= 3_hp;
                    getEngine().getLara().m_state.is_hit = true;
                }
                if( m_state.creatureInfo->mood == ai::Mood::Bored || getEngine().getLara().m_state.health <= 0_hp )
                {
                    m_state.goal_anim_state = GettingDown;
                }
                else if( aiInfo.ahead && m_state.required_anim_state == 0_as )
                {
                    if( m_state.creatureInfo->flags == 0 && aiInfo.distance < util::square( 2048_len )
                        && util::rand15() < 768 )
                    {
                        m_state.required_anim_state = RoaringStanding;
                        m_state.goal_anim_state = GettingDown;
                    }
                    else if( aiInfo.distance < util::square( core::SectorSize ) )
                    {
                        m_state.goal_anim_state = RunningAttack;
                    }
                }
                break;
            case RoaringStanding.get():
                if( m_state.creatureInfo->flags != 0 )
                {
                    m_state.required_anim_state = 0_as;
                    m_state.goal_anim_state = GettingDown;
                }
                else if( m_state.required_anim_state != 0_as )
                {
                    m_state.goal_anim_state = m_state.required_anim_state;
                }
                else if( m_state.creatureInfo->mood == ai::Mood::Bored
                         || m_state.creatureInfo->mood == ai::Mood::Escape )
                {
                    m_state.goal_anim_state = GettingDown;
                }
                else if( aiInfo.bite && aiInfo.distance < util::square( 600_len ) )
                {
                    m_state.goal_anim_state = Standing;
                }
                else
                {
                    m_state.goal_anim_state = WalkingTall;
                }
                break;
            case RunningAttack.get():
                if( m_state.required_anim_state == 0_as && (m_state.touch_bits.to_ulong() & 0x2406cUL) )
                {
                    emitParticle( core::TRVec{0_len, 96_len, 335_len}, 14, &createBloodSplat );
                    getEngine().getLara().m_state.health -= 200_hp;
                    getEngine().getLara().m_state.is_hit = true;
                    m_state.required_anim_state = GettingDown;
                }
                break;
            case Standing.get():
                if( m_state.required_anim_state == 0_as && (m_state.touch_bits.to_ulong() & 0x2406cUL) )
                {
                    getEngine().getLara().m_state.health -= 400_hp;
                    getEngine().getLara().m_state.is_hit = true;
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
        switch( m_state.current_anim_state.get() )
        {
            case Walking.get():
            case Running.get():
                m_state.goal_anim_state = GettingDown;
                break;
            case GettingDown.get():
                m_state.creatureInfo->flags = 0;
                m_state.goal_anim_state = Dying;
                break;
            case WalkingTall.get():
                m_state.goal_anim_state = RoaringStanding;
                break;
            case RoaringStanding.get():
                m_state.creatureInfo->flags = 1;
                m_state.goal_anim_state = Dying;
                break;
            case Dying.get():
                if( m_state.creatureInfo->flags != 0 && (m_state.touch_bits.to_ulong() & 0x2406cUL) != 0 )
                {
                    getEngine().getLara().m_state.health -= 200_hp;
                    getEngine().getLara().m_state.is_hit = true;
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
