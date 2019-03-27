#include "rat.h"

#include "engine/laranode.h"
#include "engine/particle.h"

namespace engine
{
namespace items
{
void Rat::update()
{
    if( m_state.triggerState == TriggerState::Invisible )
    {
        m_state.triggerState = TriggerState::Active;
    }

    m_state.initCreatureInfo( getEngine() );

    if( m_state.type == TR1ItemId::RatInWater )
    {
        if( m_state.health > 0_hp )
        {
            const ai::AiInfo aiInfo{getEngine(), m_state};
            core::Angle headRot = 0_deg;
            if( aiInfo.ahead )
            {
                headRot = aiInfo.angle;
            }
            updateMood( getEngine(), m_state, aiInfo, true );
            const auto turn = rotateTowardsTarget( 3_deg );

            if( m_state.current_anim_state == 1_as )
            {
                if( aiInfo.ahead )
                {
                    if( m_state.touch_bits.to_ulong() & 0x300018ful )
                    {
                        m_state.goal_anim_state = 2_as;
                    }
                }
            }
            else if( m_state.current_anim_state == 2_as )
            {
                if( m_state.required_anim_state == 0_as )
                {
                    if( aiInfo.ahead )
                    {
                        if( m_state.touch_bits.to_ulong() & 0x300018ful )
                        {
                            emitParticle( {0_len, -11_len, 108_len}, 3, &createBloodSplat );
                            getEngine().getLara().m_state.health -= 20_hp;
                            getEngine().getLara().m_state.is_hit = true;
                            m_state.required_anim_state = 1_as;
                        }
                    }
                }
                m_state.goal_anim_state = 0_as;
            }

            rotateCreatureHead( headRot );

            const auto waterHeight = getWaterSurfaceHeight();
            if( !waterHeight.is_initialized() )
            {
                m_state.type = TR1ItemId::RatOnLand;
                m_state.anim = &getEngine().findAnimatedModelForType( TR1ItemId::RatOnLand )->animations[0];
                m_state.frame_number = m_state.anim->firstFrame;
                m_state.goal_anim_state = m_state.anim->state_id;
                m_state.current_anim_state = m_state.anim->state_id;
            }
            const auto prevY = m_state.position.position.Y;
            m_state.position.position.Y = m_state.floor;
            animateCreature( turn, 0_deg );
            if( prevY != -core::HeightLimit )
            {
                const auto w = waterHeight.get_value_or( -core::HeightLimit );
                if( w - prevY < -32_len )
                {
                    m_state.position.position.Y = prevY - 32_len;
                }
                else if( w - prevY > 32_len )
                {
                    m_state.position.position.Y = prevY + 32_len;
                }
                else
                {
                    m_state.position.position.Y = w;
                }
            }
        }
        else
        {
            if( m_state.current_anim_state != 3_as )
            {
                m_state.anim = &getEngine().findAnimatedModelForType( TR1ItemId::RatInWater )->animations[2];
                m_state.frame_number = m_state.anim->firstFrame;
                m_state.current_anim_state = 3_as;
            }
            rotateCreatureHead( 0_deg );
            ModelItemNode::update();
            if( m_state.triggerState == TriggerState::Deactivated )
            {
                m_state.collidable = false;
                m_state.health = -16384_hp;
                m_state.triggerState = TriggerState::Active;
            }
            if( !getWaterSurfaceHeight().is_initialized() )
            {
                m_state.type = TR1ItemId::RatOnLand;
                m_state.anim = &getEngine().findAnimatedModelForType( TR1ItemId::RatOnLand )->animations[8];
                m_state.goal_anim_state = 5_as;
                m_state.frame_number = m_state.anim->firstFrame;
                m_state.current_anim_state = m_state.goal_anim_state;
                m_state.position.position.Y = m_state.floor;
            }
        }
    }
    else
    {
        BOOST_ASSERT( m_state.type == TR1ItemId::RatOnLand );
        core::Angle turn = 0_deg;
        core::Angle headRot = 0_deg;
        if( m_state.health > 0_hp )
        {
            const ai::AiInfo aiInfo{getEngine(), m_state};
            if( aiInfo.ahead )
            {
                headRot = aiInfo.angle;
            }
            updateMood( getEngine(), m_state, aiInfo, false );
            turn = rotateTowardsTarget( 6_deg );

            switch( m_state.current_anim_state.get() )
            {
                case 1:
                    if( m_state.required_anim_state != 0_as )
                    {
                        m_state.goal_anim_state = m_state.required_anim_state;
                    }
                    else if( aiInfo.bite && aiInfo.distance < util::square( 341_len ) )
                    {
                        m_state.goal_anim_state = 4_as;
                    }
                    else
                    {
                        m_state.goal_anim_state = 3_as;
                    }
                    break;
                case 2:
                    if( m_state.required_anim_state == 0_as && aiInfo.ahead
                        && m_state.touch_bits.to_ulong() & 0x300018ful )
                    {
                        emitParticle( {0_len, -11_len, 108_len}, 3, &createBloodSplat );
                        getEngine().getLara().m_state.health -= 20_hp;
                        getEngine().getLara().m_state.is_hit = true;
                        m_state.required_anim_state = 3_as;
                    }
                    break;
                case 3:
                    if( aiInfo.ahead && m_state.touch_bits.to_ulong() & 0x300018ful )
                    {
                        m_state.goal_anim_state = 1_as;
                        break;
                    }
                    if( aiInfo.bite && aiInfo.distance < util::square( 1536_len ) )
                    {
                        m_state.goal_anim_state = 2_as;
                    }
                    else if( aiInfo.ahead && util::rand15() < 256 )
                    {
                        m_state.required_anim_state = 6_as;
                        m_state.goal_anim_state = 1_as;
                    }
                    break;
                case 4:
                    if( m_state.required_anim_state == 0_as && aiInfo.ahead
                        && (m_state.touch_bits.to_ulong() & 0x300018ful) != 0 )
                    {
                        emitParticle( {0_len, -11_len, 108_len}, 3, &createBloodSplat );
                        getEngine().getLara().m_state.health -= 20_hp;
                        getEngine().getLara().m_state.is_hit = true;
                        m_state.required_anim_state = 1_as;
                    }
                    break;
                case 6:
                    if( m_state.creatureInfo->mood != ai::Mood::Bored || util::rand15() < 256 )
                    {
                        m_state.goal_anim_state = 1_as;
                    }
                    break;
                default:
                    break;
            }
        }
        else if( m_state.current_anim_state != 5_as )
        {
            m_state.anim = &getEngine().findAnimatedModelForType( TR1ItemId::RatOnLand )->animations[8];
            m_state.current_anim_state = 5_as;
            m_state.frame_number = m_state.anim->firstFrame;
        }
        rotateCreatureHead( headRot );
        if( const auto waterHeight = getWaterSurfaceHeight() )
        {
            m_state.type = TR1ItemId::RatInWater;
            m_state.anim = &getEngine().findAnimatedModelForType( TR1ItemId::RatInWater )->animations[0];
            m_state.frame_number = m_state.anim->firstFrame;
            m_state.goal_anim_state = m_state.anim->state_id;
            m_state.current_anim_state = m_state.anim->state_id;
            m_state.position.position.Y = waterHeight.get();
        }
        animateCreature( turn, 0_deg );
    }
}
}
}
