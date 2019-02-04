#include "crocodile.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
void Crocodile::update()
{
    if( m_state.triggerState == TriggerState::Invisible )
    {
        m_state.triggerState = TriggerState::Active;
    }

    m_state.initCreatureInfo( getLevel() );

    if( m_state.type == TR1ItemId::CrocodileInWater )
    {
        core::Angle headRot = 0_deg;
        if( m_state.health > 0_hp )
        {
            const ai::AiInfo aiInfo{getLevel(), m_state};
            if( aiInfo.ahead )
            {
                headRot = aiInfo.angle;
            }
            updateMood( getLevel(), m_state, aiInfo, true );
            rotateTowardsTarget( 3_deg );
            if( m_state.current_anim_state == 1_as )
            {
                if( aiInfo.bite )
                {
                    if( m_state.touch_bits.any() )
                    {
                        m_state.goal_anim_state = 2_as;
                    }
                }
            }
            else if( m_state.current_anim_state == 2_as )
            {
                if( m_state.frame_number == m_state.anim->firstFrame )
                {
                    m_state.required_anim_state = 0_as;
                }
                if( aiInfo.bite && m_state.touch_bits != 0 )
                {
                    if( m_state.required_anim_state == 0_as )
                    {
                        emitParticle( {5_len, -21_len, 467_len}, 9, &createBloodSplat );
                        getLevel().m_lara->m_state.health -= 100_hp;
                        getLevel().m_lara->m_state.is_hit = true;
                        m_state.required_anim_state = 1_as;
                    }
                }
                else
                {
                    m_state.goal_anim_state = 1_as;
                }
            }
            rotateCreatureHead( headRot );
            if( auto waterSurfaceHeight = getWaterSurfaceHeight() )
            {
                *waterSurfaceHeight += core::QuarterSectorSize;
                if( *waterSurfaceHeight > m_state.position.position.Y )
                {
                    m_state.position.position.Y = *waterSurfaceHeight;
                }
            }
            else
            {
                m_state.anim = &getLevel().findAnimatedModelForType( TR1ItemId::CrocodileOnLand )->animations[0];
                m_state.type = TR1ItemId::CrocodileOnLand;
                m_state.frame_number = m_state.anim->firstFrame;
                m_state.rotation.X = 0_deg;
                m_state.goal_anim_state = m_state.anim->state_id;
                m_state.current_anim_state = m_state.anim->state_id;
                m_state.position.position.Y = m_state.floor;
                m_state.creatureInfo->lot.step = 256_len;
                m_state.creatureInfo->lot.drop = -256_len;
                m_state.creatureInfo->lot.fly = 0_len;
            }
            getSkeleton()
                    ->patchBone( 8, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix() );
            animateCreature( 0_deg, 0_deg );
        }
        else
        {
            if( m_state.current_anim_state != 3_as )
            {
                m_state.anim = &getLevel().findAnimatedModelForType( TR1ItemId::CrocodileInWater )->animations[4];
                m_state.current_anim_state = 3_as;
                m_state.health = -16384_hp;
                m_state.frame_number = m_state.anim->firstFrame;
            }
            if( const auto waterSurfaceHeight = getWaterSurfaceHeight() )
            {
                if( *waterSurfaceHeight + 32_len < m_state.position.position.Y )
                {
                    m_state.position.position.Y = m_state.position.position.Y - 32_len;
                }
                else if( *waterSurfaceHeight > m_state.position.position.Y )
                {
                    m_state.position.position.Y = *waterSurfaceHeight;
                    m_state.creatureInfo.reset();
                }
            }
            else
            {
                m_state.anim = &getLevel().findAnimatedModelForType( TR1ItemId::CrocodileOnLand )->animations[11];
                m_state.type = TR1ItemId::CrocodileOnLand;
                m_state.goal_anim_state = 7_as;
                m_state.frame_number = m_state.anim->firstFrame;
                m_state.current_anim_state = m_state.goal_anim_state;
                auto room = m_state.position.room;
                auto sector = level::Level::findRealFloorSector( m_state.position.position, &room );
                m_state.position.position.Y = HeightInfo::fromFloor( sector,
                                                                     m_state.position.position,
                                                                     getLevel().m_itemNodes ).y;
                m_state.rotation.X = 0_deg;
            }
            ModelItemNode::update();
            auto room = m_state.position.room;
            auto sector = level::Level::findRealFloorSector( m_state.position.position, &room );
            m_state.floor = HeightInfo::fromFloor( sector, m_state.position.position, getLevel().m_itemNodes ).y;
            setCurrentRoom( room );
        }
    }
    else
    {
        BOOST_ASSERT( m_state.type == TR1ItemId::CrocodileOnLand );
        core::Angle turnRot = 0_deg;
        core::Angle headRot = 0_deg;
        if( m_state.health > 0_hp )
        {
            const ai::AiInfo aiInfo{getLevel(), m_state};
            if( aiInfo.ahead )
            {
                headRot = aiInfo.angle;
            }
            updateMood( getLevel(), m_state, aiInfo, true );
            if( m_state.current_anim_state == 4_as )
            {
                m_state.rotation.Y += 6_deg;
            }
            else
            {
                turnRot = rotateTowardsTarget( 3_deg );
            }
            switch( m_state.current_anim_state.get() )
            {
                case 1:
                    if( aiInfo.bite && aiInfo.distance < util::square( 435_len ) )
                    {
                        m_state.goal_anim_state = 5_as;
                        break;
                    }
                    switch( m_state.creatureInfo->mood )
                    {
                        case ai::Mood::Escape:
                            m_state.goal_anim_state = 2_as;
                            break;
                        case ai::Mood::Attack:
                            if( (aiInfo.angle >= -90_deg && aiInfo.angle <= 90_deg)
                                || aiInfo.distance <= util::square( 3 * core::SectorSize ) )
                            {
                                m_state.goal_anim_state = 2_as;
                            }
                            else
                            {
                                m_state.goal_anim_state = 4_as;
                            }
                            break;
                        case ai::Mood::Stalk:
                            m_state.goal_anim_state = 3_as;
                            break;
                        default:
                            // silence compiler
                            break;
                    }
                    break;
                case 2:
                    if( aiInfo.ahead && (m_state.touch_bits.to_ulong() & 0x3fcUL) )
                    {
                        m_state.goal_anim_state = 1_as;
                    }
                    else
                    {
                        if( m_state.creatureInfo->mood == ai::Mood::Stalk )
                        {
                            m_state.goal_anim_state = 3_as;
                        }
                        else if( m_state.creatureInfo->mood != ai::Mood::Bored )
                        {
                            if( m_state.creatureInfo->mood == ai::Mood::Attack
                                && aiInfo.distance > util::square( 3 * core::SectorSize )
                                && (aiInfo.angle < -90_deg || aiInfo.angle > 90_deg) )
                            {
                                m_state.goal_anim_state = 1_as;
                            }
                        }
                        else
                        {
                            m_state.goal_anim_state = 1_as;
                        }
                    }
                    break;
                case 3:
                    if( aiInfo.ahead && (m_state.touch_bits.to_ulong() & 0x03fcUL) )
                    {
                        m_state.goal_anim_state = 1_as;
                    }
                    else
                    {
                        if( m_state.creatureInfo->mood == ai::Mood::Attack
                            || m_state.creatureInfo->mood == ai::Mood::Escape )
                        {
                            m_state.goal_anim_state = 2_as;
                        }
                        else if( m_state.creatureInfo->mood == ai::Mood::Bored )
                        {
                            m_state.goal_anim_state = 1_as;
                        }
                    }
                    break;
                case 4:
                    if( aiInfo.angle > -90_deg && aiInfo.angle < 90_deg )
                    {
                        m_state.goal_anim_state = 3_as;
                    }
                    break;
                case 5:
                    if( m_state.required_anim_state == 0_as )
                    {
                        emitParticle( {5_len, -21_len, 467_len}, 9, &createBloodSplat );
                        getLevel().m_lara->m_state.health -= 100_hp;
                        getLevel().m_lara->m_state.is_hit = true;
                        m_state.required_anim_state = 1_as;
                    }
                    break;
                default:
                    break;
            }
        }
        else
        {
            if( m_state.current_anim_state != 7_as )
            {
                m_state.anim = &getLevel().findAnimatedModelForType( TR1ItemId::CrocodileOnLand )->animations[11];
                m_state.current_anim_state = 7_as;
                m_state.frame_number = m_state.anim->firstFrame;
            }
        }
        if( m_state.creatureInfo != nullptr )
        {
            rotateCreatureHead( headRot );
        }
        if( m_state.position.room->isWaterRoom() )
        {
            m_state.anim = &getLevel().findAnimatedModelForType( TR1ItemId::CrocodileInWater )->animations[0];
            m_state.type = TR1ItemId::CrocodileInWater;
            m_state.frame_number = m_state.anim->firstFrame;
            m_state.goal_anim_state = m_state.anim->state_id;
            m_state.current_anim_state = m_state.anim->state_id;
            if( m_state.creatureInfo != nullptr )
            {
                m_state.creatureInfo->lot.step = 20480_len;
                m_state.creatureInfo->lot.drop = -20480_len;
                m_state.creatureInfo->lot.fly = 16_len;
            }
        }
        if( m_state.creatureInfo != nullptr )
        {
            getSkeleton()
                    ->patchBone( 8, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix() );
            animateCreature( turnRot, 0_deg );
        }
        else
        {
            ModelItemNode::update();
        }
    }
}
}
}
