#include "gorilla.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
void Gorilla::update()
{
    if( m_state.triggerState == TriggerState::Invisible )
    {
        m_state.triggerState = TriggerState::Active;
    }

    m_state.initCreatureInfo( getLevel() );

    static const constexpr uint16_t FlgWantAttack = 1;
    static const constexpr uint16_t FlgTurnedRight = 2;
    static const constexpr uint16_t FlgTurnedLeft = 4;

    core::Angle headRot = 0_deg;
    core::Angle turn = 0_deg;

    if( getHealth() > 0 )
    {
        const ai::AiInfo aiInfo{getLevel(), m_state};
        if( aiInfo.ahead )
            headRot = aiInfo.angle;
        updateMood( getLevel(), m_state, aiInfo, false );

        turn = rotateTowardsTarget( m_state.creatureInfo->maximum_turn );
        if( m_state.is_hit || aiInfo.distance < util::square( 2 * loader::SectorSize ) )
        {
            m_state.creatureInfo->flags |= FlgWantAttack;
        }
        switch( m_state.current_anim_state )
        {
            case 1:
                // standing
                if( m_state.creatureInfo->flags & FlgTurnedRight )
                {
                    m_state.rotation.Y -= 90_deg;
                    m_state.creatureInfo->flags &= ~FlgTurnedRight;
                }
                else if( m_state.creatureInfo->flags & FlgTurnedLeft )
                {
                    m_state.rotation.Y += 90_deg;
                    m_state.creatureInfo->flags &= ~FlgTurnedLeft;
                }
                if( m_state.required_anim_state != 0 )
                {
                    m_state.goal_anim_state = m_state.required_anim_state;
                }
                else if( aiInfo.bite && aiInfo.distance < 184900 )
                {
                    m_state.goal_anim_state = 4;
                }
                else if( (m_state.creatureInfo->flags & FlgWantAttack)
                         || aiInfo.zone_number != aiInfo.enemy_zone
                         || !aiInfo.ahead )
                {
                    m_state.goal_anim_state = 3;
                }
                else
                {
                    const auto r = util::rand15( 1024 );
                    if( r < 160 )
                    {
                        m_state.goal_anim_state = 10;
                    }
                    else if( r < 320 )
                    {
                        m_state.goal_anim_state = 6;
                    }
                    else if( r < 480 )
                    {
                        m_state.goal_anim_state = 7;
                    }
                    else if( r < 752 )
                    {
                        m_state.goal_anim_state = 8;
                        m_state.creatureInfo->maximum_turn = 0_deg;
                    }
                    else
                    {
                        m_state.goal_anim_state = 9;
                        m_state.creatureInfo->maximum_turn = 0_deg;
                    }
                }
                break;
            case 3:
                // running
                m_state.creatureInfo->maximum_turn = 5_deg;
                if( m_state.creatureInfo->flags == 0 && aiInfo.angle > -45_deg && aiInfo.angle < 45_deg )
                {
                    m_state.goal_anim_state = 1;
                }
                else if( aiInfo.ahead && (m_state.touch_bits & 0xff00) != 0 )
                {
                    m_state.required_anim_state = 4;
                    m_state.goal_anim_state = 1;
                }
                else if( m_state.creatureInfo->mood != ai::Mood::Escape )
                {
                    const auto r = util::rand15();
                    if( r < 160 )
                    {
                        m_state.required_anim_state = 10;
                        m_state.goal_anim_state = 1;
                    }
                    else if( r < 320 )
                    {
                        m_state.required_anim_state = 6;
                        m_state.goal_anim_state = 1;
                    }
                    else if( r < 480 )
                    {
                        m_state.required_anim_state = 7;
                        m_state.goal_anim_state = 1;
                    }
                }
                break;
            case 4:
                // attacking
                if( m_state.required_anim_state == 0 )
                {
                    if( (m_state.touch_bits & 0xff00) != 0 )
                    {
                        emitParticle( {0, -19, 75}, 15, &createBloodSplat );
                        getLevel().m_lara->m_state.health -= 200;
                        getLevel().m_lara->m_state.is_hit = true;
                        m_state.required_anim_state = 1;
                    }
                }
                break;
            case 8:
                // turn left
                if( !(m_state.creatureInfo->flags & FlgTurnedLeft) )
                {
                    m_state.rotation.Y -= 90_deg;
                    m_state.creatureInfo->flags |= FlgTurnedLeft;
                }
                m_state.goal_anim_state = 1;
                break;
            case 9:
                // turn right
                if( !(m_state.creatureInfo->flags & FlgTurnedRight) )
                {
                    m_state.rotation.Y += 90_deg;
                    m_state.creatureInfo->flags |= FlgTurnedRight;
                }
                m_state.goal_anim_state = 1;
                break;
            default:
                break;
        }
    }
    else if( m_state.current_anim_state != 5 )
    {
        m_state.anim = &getLevel().findAnimatedModelForType( TR1ItemId::Gorilla )->animations[7 + util::rand15( 2 )];
        m_state.current_anim_state = 5;
        m_state.frame_number = m_state.anim->firstFrame;
    }
    rotateCreatureHead( headRot );
    if( m_state.current_anim_state == 11 )
    {
        // climbing
        getSkeleton()->patchBone( 14, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix() );
        animateCreature( turn, 0_deg );
    }
    else
    {
        if( m_state.creatureInfo->flags & FlgTurnedRight )
        {
            m_state.rotation.Y -= 90_deg;
            m_state.creatureInfo->flags &= ~FlgTurnedRight;
        }
        else if( m_state.creatureInfo->flags & FlgTurnedLeft )
        {
            m_state.rotation.Y += 90_deg;
            m_state.creatureInfo->flags &= ~FlgTurnedLeft;
        }
        const auto old = m_state.position.position;
        getSkeleton()->patchBone( 14, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix() );
        animateCreature( turn, 0_deg );
        if( old.Y - 384 < m_state.position.position.Y )
            return;

        const auto xSectorOld = old.X / loader::SectorSize;
        const auto zSectorOld = old.Z / loader::SectorSize;
        const auto xSectorNew = m_state.position.position.X / loader::SectorSize;
        const auto zSectorNew = m_state.position.position.Z / loader::SectorSize;
        if( zSectorOld == zSectorNew )
        {
            if( xSectorOld == xSectorNew )
            {
                return;
            }
            if( xSectorOld >= xSectorNew )
            {
                m_state.rotation.Y = -90_deg;
                m_state.position.position.X = xSectorOld * loader::SectorSize + 75;
            }
            else
            {
                m_state.rotation.Y = 90_deg;
                m_state.position.position.X = xSectorNew * loader::SectorSize - 75;
            }
        }
        else if( xSectorOld == xSectorNew )
        {
            if( zSectorOld >= zSectorNew )
            {
                m_state.rotation.Y = -180_deg;
                m_state.position.position.Z = zSectorOld * loader::SectorSize + 75;
            }
            else
            {
                m_state.rotation.Y = 0_deg;
                m_state.position.position.Z = zSectorNew * loader::SectorSize - 75;
            }
        }

        m_state.position.position.Y = old.Y;
        m_state.anim = &getLevel().findAnimatedModelForType( TR1ItemId::Gorilla )->animations[19];
        m_state.current_anim_state = 11;
        m_state.frame_number = m_state.anim->firstFrame;
    }
}
}
}
