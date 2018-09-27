#pragma once

#include "itemnode.h"

// #define NO_DOOR_BLOCK

namespace engine
{
namespace items
{
class Door final : public ModelItemNode
{
public:
    Door(const gsl::not_null<level::Level*>& level,
         const gsl::not_null<const loader::Room*>& room,
         const loader::Item& item,
         const loader::SkeletalModelType& animatedModel)
            : ModelItemNode{level, room, item, true, animatedModel}
    {
#ifndef NO_DOOR_BLOCK
        int dx = 0, dz = 0;
        if( m_state.rotation.Y == 0_au )
        {
            dz = -loader::SectorSize;
        }
        else if( m_state.rotation.Y == -180_deg )
        {
            dz = loader::SectorSize;
        }
        else if( m_state.rotation.Y == 90_deg )
        {
            dx = -loader::SectorSize;
        }
        else
        {
            dx = loader::SectorSize;
        }

        const auto wingsPosition = m_state.position.position + core::TRVec{dx, 0, dz};

        m_sector = const_cast<loader::Sector*>(m_state.position.room->getSectorByAbsolutePosition( wingsPosition ));
        BOOST_ASSERT( m_sector != nullptr );
        if( m_sector->portalTarget == nullptr )
        {
            m_box = const_cast<loader::Box*>(m_sector->box);
        }
        else
        {
            m_box = const_cast<loader::Box*>(m_sector->portalTarget->getSectorByAbsolutePosition( wingsPosition )->box);
        }
        if( m_box != nullptr && (m_box->overlap_index & 0x8000u) == 0 )
        {
            m_box = nullptr;
        }

        m_sectorData = *m_sector;
        if( m_state.position.room->alternateRoom == -1 )
        {
            m_alternateSector = nullptr;
        }
        else
        {
            m_alternateSector = const_cast<loader::Sector*>(getLevel().m_rooms[m_state.position.room->alternateRoom]
                    .getSectorByAbsolutePosition( wingsPosition ));
            BOOST_ASSERT( m_alternateSector != nullptr );
            if( m_alternateSector->portalTarget == nullptr )
            {
                m_alternateBox = const_cast<loader::Box*>(m_alternateSector->box);
            }
            else
            {
                m_alternateBox = const_cast<loader::Box*>(m_alternateSector->portalTarget->getSectorByAbsolutePosition(
                        wingsPosition )->box);
            }
            if( m_alternateBox != nullptr
                && (m_alternateBox->overlap_index & 0x8000u) == 0 )
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
                m_box->overlap_index |= 0x4000u;
            }
        }
        if( m_alternateSector != nullptr )
        {
            m_alternateSector->reset();
            if( m_alternateBox != nullptr )
            {
                m_alternateBox->overlap_index |= 0x4000u;
            }
        }
        if( m_sector->portalTarget == nullptr )
        {
            m_targetSector = nullptr;
            m_alternateTargetSector = nullptr;
            return;
        }

        m_targetSector = const_cast<loader::Sector*>(m_sector->portalTarget->getSectorByAbsolutePosition(
                m_state.position.position ));
        if( m_targetSector->portalTarget == nullptr )
        {
            m_targetBox = const_cast<loader::Box*>(m_targetSector->box);
        }
        else
        {
            m_targetBox = const_cast<loader::Box*>(m_targetSector->portalTarget->getSectorByAbsolutePosition(
                    m_state.position.position )->box);
        }
        if( (m_targetBox->overlap_index & 0x8000) == 0 )
        {
            m_targetBox = nullptr;
        }
        m_targetSectorData = *m_targetSector;
        if( m_sector->portalTarget->alternateRoom == -1 )
        {
            m_alternateTargetSector = nullptr;
        }
        else
        {
            m_alternateTargetSector = const_cast<loader::Sector*>(getLevel().m_rooms[m_sector->portalTarget
                                                                                             ->alternateRoom]
                    .getSectorByAbsolutePosition( m_state.position.position ));
            if( m_alternateTargetSector->portalTarget == nullptr )
            {
                m_alternateTargetBox = const_cast<loader::Box*>(m_alternateTargetSector->box);
            }
            else
            {
                m_alternateTargetBox = const_cast<loader::Box*>(m_alternateTargetSector->portalTarget
                                                                                       ->getSectorByAbsolutePosition(
                                                                                               m_state.position
                                                                                                      .position )->box);
            }
            if( (m_alternateTargetBox->overlap_index & 0x8000u) == 0 )
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
                m_targetBox->overlap_index |= 0x4000u;
            }
        }
        if( m_alternateTargetSector != nullptr )
        {
            m_alternateTargetSector->reset();
            if( m_alternateTargetBox != nullptr )
            {
                m_alternateTargetBox->overlap_index |= 0x4000u;
            }
        }
#endif
    }

    void update() override
    {
        if( m_state.updateActivationTimeout() )
        {
            if( m_state.current_anim_state != 0 )
            {
#ifndef NO_DOOR_BLOCK
                if( m_sector )
                {
                    *m_sector = m_sectorData;
                    if( m_box != nullptr )
                    {
                        m_box->overlap_index &= ~0x4000u;
                    }
                }
                if( m_targetSector )
                {
                    *m_targetSector = m_targetSectorData;
                    if( m_targetBox != nullptr )
                    {
                        m_targetBox->overlap_index &= ~0x4000u;
                    }
                }
                if( m_alternateSector )
                {
                    *m_alternateSector = m_alternateSectorData;
                    if( m_alternateBox != nullptr )
                    {
                        m_alternateBox->overlap_index &= ~0x4000u;
                    }
                }
                if( m_alternateTargetSector )
                {
                    *m_alternateTargetSector = m_alternateTargetSectorData;
                    if( m_alternateTargetBox != nullptr )
                    {
                        m_alternateTargetBox->overlap_index &= ~0x4000u;
                    }
                }
#endif
            }
            else
            {
                m_state.goal_anim_state = 1;
            }
        }
        else if( m_state.current_anim_state == 1 )
        {
            m_state.goal_anim_state = 0;
        }
        else
        {
#ifndef NO_DOOR_BLOCK
            if( m_sector )
            {
                m_sector->reset();
                if( m_box != nullptr )
                {
                    m_box->overlap_index |= 0x4000u;
                }
            }
            if( m_targetSector )
            {
                m_targetSector->reset();
                if( m_targetBox != nullptr )
                {
                    m_targetBox->overlap_index |= 0x4000u;
                }
            }
            if( m_alternateSector )
            {
                m_alternateSector->reset();
                if( m_alternateBox != nullptr )
                {
                    m_alternateBox->overlap_index |= 0x4000u;
                }
            }
            if( m_alternateTargetSector )
            {
                m_alternateTargetSector->reset();
                if( m_alternateTargetBox != nullptr )
                {
                    m_alternateTargetBox->overlap_index |= 0x4000u;
                }
            }
#endif
        }

        ModelItemNode::update();
    }

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override
    {
#ifndef NO_DOOR_BLOCK
        if( !isNear( lara, collisionInfo.collisionRadius ) )
            return;

        if( !testBoneCollision( lara ) )
            return;

        if( !(collisionInfo.policyFlags & CollisionInfo::EnableBaddiePush) )
            return;

        if( m_state.current_anim_state == m_state.goal_anim_state )
        {
            enemyPush( lara, collisionInfo, false, true );
        }
        else
        {
            const auto enableSpaz = (collisionInfo.policyFlags & CollisionInfo::EnableSpaz) != 0;
            enemyPush( lara, collisionInfo, enableSpaz, true );
        }
#endif
    }

private:
#ifndef NO_DOOR_BLOCK
    loader::Sector* m_sector{nullptr};
    loader::Sector m_sectorData;
    loader::Box* m_box{nullptr};
    loader::Sector* m_alternateSector{nullptr};
    loader::Sector m_alternateSectorData;
    loader::Box* m_alternateBox{nullptr};
    loader::Sector* m_targetSector{nullptr};
    loader::Sector m_targetSectorData;
    loader::Box* m_targetBox{nullptr};
    loader::Sector* m_alternateTargetSector{nullptr};
    loader::Sector m_alternateTargetSectorData;
    loader::Box* m_alternateTargetBox{nullptr};
#endif
};
}
}
