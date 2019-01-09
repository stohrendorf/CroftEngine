#include "thorhammer.h"

#include "level/level.h"
#include "engine/laranode.h"

namespace engine
{
namespace items
{
ThorHammerHandle::ThorHammerHandle(const gsl::not_null<level::Level*>& level,
                                   const gsl::not_null<const loader::Room*>& room, const loader::Item& item,
                                   const loader::SkeletalModelType& animatedModel)
        : ModelItemNode{level, room, item, true, animatedModel}
{
    m_block = level->createItem<ThorHammerBlock>( TR1ItemId::ThorHammerBlock, room, core::Angle( item.rotation ),
                                                  item.position, 0 );
}

void ThorHammerHandle::update()
{
    switch( m_state.current_anim_state )
    {
        case 0:
            if( m_state.updateActivationTimeout() )
            {
                m_state.goal_anim_state = 1;
            }
            else
            {
                deactivate();
                m_state.triggerState = TriggerState::Inactive;
            }
            break;
        case 1:
            if( m_state.updateActivationTimeout() )
            {
                m_state.goal_anim_state = 2;
            }
            else
            {
                m_state.goal_anim_state = 0;
            }
            break;
        case 2:
            if( m_state.frame_number > m_state.anim->firstFrame + 30 )
            {
                auto posX = m_state.position.position.X;
                auto posZ = m_state.position.position.Z;
                if( m_state.rotation.Y == 0_deg )
                {
                    posZ += 3 * loader::SectorSize;
                }
                else if( m_state.rotation.Y == 90_deg )
                {
                    posX += 3 * loader::SectorSize;
                }
                else if( m_state.rotation.Y == 180_deg )
                {
                    posZ -= 3 * loader::SectorSize;
                }
                else if( m_state.rotation.Y == -90_deg )
                {
                    posX -= 3 * loader::SectorSize;
                }
                if( getLevel().m_lara->m_state.health >= 0 )
                {
                    if( posX - 520 < getLevel().m_lara->m_state.position.position.X
                        && posX + 520 > getLevel().m_lara->m_state.position.position.X
                        && posZ - 520 < getLevel().m_lara->m_state.position.position.Z
                        && posZ + 520 > getLevel().m_lara->m_state.position.position.Z )
                    {
                        getLevel().m_lara->m_state.health = -1;
                        getLevel().m_lara->m_state.anim = &getLevel()
                                .findAnimatedModelForType( engine::TR1ItemId::Lara )->animations[139];
                        getLevel().m_lara->m_state.frame_number = 3561;
                        getLevel().m_lara->m_state.current_anim_state = 46;
                        getLevel().m_lara->m_state.position.position.Y = m_state.position.position.Y;
                        getLevel().m_lara->setGoalAnimState( loader::LaraStateId::BoulderDeath );
                        getLevel().m_lara->m_state.falling = false;
                    }
                }
            }
            break;
        case 3:
        {
            const auto sector = level::Level::findRealFloorSector( m_state.position.position, m_state.position.room );
            const auto hi = HeightInfo::fromFloor( gsl::make_not_null( sector ), m_state.position.position,
                                                   getLevel().m_itemNodes );
            getLevel().m_lara->handleCommandSequence( hi.lastCommandSequenceOrDeath, true );

            const auto oldPosX = m_state.position.position.X;
            const auto oldPosZ = m_state.position.position.Z;
            if( m_state.rotation.Y == 0_deg )
            {
                m_state.position.position.Z += 3 * loader::SectorSize;
            }
            else if( m_state.rotation.Y == 90_deg )
            {
                m_state.position.position.X += 3 * loader::SectorSize;
            }
            else if( m_state.rotation.Y == 180_deg )
            {
                m_state.position.position.Z -= 3 * loader::SectorSize;
            }
            else if( m_state.rotation.Y == -90_deg )
            {
                m_state.position.position.X -= 3 * loader::SectorSize;
            }
            if( getLevel().m_lara->m_state.health >= 0 )
            {
                m_state.position.room->patchHeightsForBlock( *this, -2 * loader::SectorSize );
            }
            m_state.position.position.X = oldPosX;
            m_state.position.position.Z = oldPosZ;
            deactivate();
            m_state.triggerState = TriggerState::Deactivated;
            break;
        }
        default:
            break;
    }
    ModelItemNode::update();

    // sync anim
    const auto animIdx = std::distance(
            &getLevel().findAnimatedModelForType( engine::TR1ItemId::ThorHammerHandle )->animations[0], m_state.anim );
    m_block->m_state.anim = &getLevel().findAnimatedModelForType( engine::TR1ItemId::ThorHammerBlock )->animations[139];
    m_block->m_state.frame_number = m_state.frame_number - m_state.anim->firstFrame + m_block->m_state.anim->firstFrame;
    m_block->m_state.current_anim_state = m_state.current_anim_state;
}

void ThorHammerHandle::collide(LaraNode& node, CollisionInfo& info)
{
    if( (info.policyFlags & CollisionInfo::EnableBaddiePush) == 0 )
        return;

    if( !isNear( node, info.collisionRadius ) )
        return;

    enemyPush( node, info, false, true );
}

void ThorHammerBlock::collide(LaraNode& node, CollisionInfo& info)
{
    if( m_state.current_anim_state == 2 )
        return;

    if( (info.policyFlags & CollisionInfo::EnableBaddiePush) == 0 )
        return;

    if( !isNear( node, info.collisionRadius ) )
        return;

    enemyPush( node, info, false, true );
}
}
}
