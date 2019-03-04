#include "door.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
// #define NO_DOOR_BLOCK

Door::Door(const gsl::not_null<engine::Engine*>& engine, const gsl::not_null<const loader::file::Room*>& room,
           const loader::file::Item& item, const loader::file::SkeletalModelType& animatedModel)
        : ModelItemNode{engine, room, item, true, animatedModel}
{
#ifndef NO_DOOR_BLOCK
    core::Length dx = 0_len, dz = 0_len;
    if( m_state.rotation.Y == 0_au )
    {
        dz = -core::SectorSize;
    }
    else if( m_state.rotation.Y == -180_deg )
    {
        dz = core::SectorSize;
    }
    else if( m_state.rotation.Y == 90_deg )
    {
        dx = -core::SectorSize;
    }
    else
    {
        dx = core::SectorSize;
    }

    const auto wingsPosition = m_state.position.position + core::TRVec{dx, 0_len, dz};

    m_sector = const_cast<loader::file::Sector*>(m_state.position.room->getSectorByAbsolutePosition( wingsPosition ));
    BOOST_ASSERT( m_sector != nullptr );
    if( m_sector->portalTarget == nullptr )
    {
        m_box = const_cast<loader::file::Box*>(m_sector->box);
    }
    else
    {
        m_box = const_cast<loader::file::Box*>(m_sector->portalTarget->getSectorByAbsolutePosition( wingsPosition )->box);
    }
    if( m_box != nullptr && !m_box->isBlockable() )
    {
        m_box = nullptr;
    }

    m_sectorData = *m_sector;
    if( m_state.position.room->alternateRoom.get() < 0 )
    {
        m_alternateSector = nullptr;
    }
    else
    {
        m_alternateSector = const_cast<loader::file::Sector*>(getEngine().getRooms()
                                                                  .at( m_state.position.room->alternateRoom.get() )
                                                                  .getSectorByAbsolutePosition( wingsPosition ));
        BOOST_ASSERT( m_alternateSector != nullptr );
        if( m_alternateSector->portalTarget == nullptr )
        {
            m_alternateBox = const_cast<loader::file::Box*>(m_alternateSector->box);
        }
        else
        {
            m_alternateBox = const_cast<loader::file::Box*>(m_alternateSector->portalTarget->getSectorByAbsolutePosition(
                    wingsPosition )->box);
        }
        if( m_alternateBox != nullptr && !m_alternateBox->isBlockable() )
        {
            m_alternateBox = nullptr;
        }
        m_alternateSectorData = *m_alternateSector;
    }
    if( m_sector != nullptr )
    {
        m_sector->reset();
        if( m_box != nullptr )
        {
            m_box->block();
        }
    }
    if( m_alternateSector != nullptr )
    {
        m_alternateSector->reset();
        if( m_alternateBox != nullptr )
        {
            m_alternateBox->block();
        }
    }
    if( m_sector->portalTarget == nullptr )
    {
        m_targetSector = nullptr;
        m_alternateTargetSector = nullptr;
        return;
    }

    m_targetSector = const_cast<loader::file::Sector*>(m_sector->portalTarget->getSectorByAbsolutePosition(
            m_state.position.position ));
    if( m_targetSector->portalTarget == nullptr )
    {
        m_targetBox = const_cast<loader::file::Box*>(m_targetSector->box);
    }
    else
    {
        m_targetBox = const_cast<loader::file::Box*>(m_targetSector->portalTarget->getSectorByAbsolutePosition(
                m_state.position.position )->box);
    }
    if( !m_targetBox->isBlockable() )
    {
        m_targetBox = nullptr;
    }
    m_targetSectorData = *m_targetSector;
    if( m_sector->portalTarget->alternateRoom.get() < 0 )
    {
        m_alternateTargetSector = nullptr;
    }
    else
    {
        m_alternateTargetSector = const_cast<loader::file::Sector*>(getEngine().getRooms()
                                                                        .at( m_sector->portalTarget->alternateRoom
                                                                                     .get() )
                                                                        .getSectorByAbsolutePosition(
                                                                                m_state.position.position ));
        if( m_alternateTargetSector->portalTarget == nullptr )
        {
            m_alternateTargetBox = const_cast<loader::file::Box*>(m_alternateTargetSector->box);
        }
        else
        {
            m_alternateTargetBox = const_cast<loader::file::Box*>(m_alternateTargetSector->portalTarget
                                                                                   ->getSectorByAbsolutePosition(
                                                                                           m_state.position
                                                                                                  .position )->box);
        }
        if( !m_alternateTargetBox->isBlockable() )
        {
            m_alternateTargetBox = nullptr;
        }
        m_alternateTargetSectorData = *m_alternateTargetSector;
    }
    if( m_targetSector )
    {
        m_targetSector->reset();
        if( m_targetBox != nullptr )
        {
            m_targetBox->block();
        }
    }
    if( m_alternateTargetSector != nullptr )
    {
        m_alternateTargetSector->reset();
        if( m_alternateTargetBox != nullptr )
        {
            m_alternateTargetBox->block();
        }
    }
#endif
}

void Door::update()
{
    if( m_state.updateActivationTimeout() )
    {
        if( m_state.current_anim_state != 0_as )
        {
#ifndef NO_DOOR_BLOCK
            if( m_sector )
            {
                *m_sector = m_sectorData;
                if( m_box != nullptr )
                {
                    m_box->unblock();
                }
            }
            if( m_targetSector )
            {
                *m_targetSector = m_targetSectorData;
                if( m_targetBox != nullptr )
                {
                    m_targetBox->unblock();
                }
            }
            if( m_alternateSector )
            {
                *m_alternateSector = m_alternateSectorData;
                if( m_alternateBox != nullptr )
                {
                    m_alternateBox->unblock();
                }
            }
            if( m_alternateTargetSector )
            {
                *m_alternateTargetSector = m_alternateTargetSectorData;
                if( m_alternateTargetBox != nullptr )
                {
                    m_alternateTargetBox->unblock();
                }
            }
#endif
        }
        else
        {
            m_state.goal_anim_state = 1_as;
        }
    }
    else if( m_state.current_anim_state == 1_as )
    {
        m_state.goal_anim_state = 0_as;
    }
    else
    {
#ifndef NO_DOOR_BLOCK
        if( m_sector )
        {
            m_sector->reset();
            if( m_box != nullptr )
            {
                m_box->block();
            }
        }
        if( m_targetSector )
        {
            m_targetSector->reset();
            if( m_targetBox != nullptr )
            {
                m_targetBox->block();
            }
        }
        if( m_alternateSector )
        {
            m_alternateSector->reset();
            if( m_alternateBox != nullptr )
            {
                m_alternateBox->block();
            }
        }
        if( m_alternateTargetSector )
        {
            m_alternateTargetSector->reset();
            if( m_alternateTargetBox != nullptr )
            {
                m_alternateTargetBox->block();
            }
        }
#endif
    }

    ModelItemNode::update();
}

void Door::collide(LaraNode& lara, CollisionInfo& collisionInfo)
{
#ifndef NO_DOOR_BLOCK
    if( !isNear( lara, collisionInfo.collisionRadius ) )
        return;

    if( !testBoneCollision( lara ) )
        return;

    if( !collisionInfo.policyFlags.is_set( CollisionInfo::PolicyFlags::EnableBaddiePush ) )
        return;

    if( m_state.current_anim_state == m_state.goal_anim_state )
    {
        enemyPush( lara, collisionInfo, false, true );
    }
    else
    {
        const auto enableSpaz = collisionInfo.policyFlags.is_set( CollisionInfo::PolicyFlags::EnableSpaz );
        enemyPush( lara, collisionInfo, enableSpaz, true );
    }
#endif
}
}
}