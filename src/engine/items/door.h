#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class Door final : public ModelItemNode
{
public:
    Door(const gsl::not_null<level::Level*>& level,
         const std::string& name,
         const gsl::not_null<const loader::Room*>& room,
         const loader::Item& item,
         const loader::SkeletalModelType& animatedModel)
            : ModelItemNode( level, name, room, item, true, animatedModel )
    {
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

        const auto wingsPosition = m_state.position.position + core::TRCoordinates{dx, 0, dz};

        m_extraDoorStuff.sector = const_cast<loader::Sector*>(m_state.position.room
                                                                     ->getSectorByAbsolutePosition( wingsPosition ));
        BOOST_ASSERT( m_extraDoorStuff.sector != nullptr );
        if( m_extraDoorStuff.sector->portalTarget == nullptr )
        {
            m_extraDoorStuff.box = const_cast<loader::Box*>(m_extraDoorStuff.sector->box);
        }
        else
        {
            m_extraDoorStuff.box = const_cast<loader::Box*>(m_extraDoorStuff.sector->portalTarget
                                                                            ->getSectorByAbsolutePosition(
                                                                                    wingsPosition )->box);
        }
        if( m_extraDoorStuff.box != nullptr && (m_extraDoorStuff.box->overlap_index & 0x8000u) == 0 )
        {
            m_extraDoorStuff.box = nullptr;
        }

        m_extraDoorStuff.sectorData = *m_extraDoorStuff.sector;
        if( m_state.position.room->alternateRoom == -1 )
        {
            m_extraDoorStuff.alternateSector = nullptr;
        }
        else
        {
            m_extraDoorStuff.alternateSector = const_cast<loader::Sector*>(getLevel().m_rooms[m_state.position.room
                                                                                                     ->alternateRoom]
                    .getSectorByAbsolutePosition( wingsPosition ));
            BOOST_ASSERT( m_extraDoorStuff.alternateSector != nullptr );
            if( m_extraDoorStuff.alternateSector->portalTarget == nullptr )
            {
                m_extraDoorStuff.alternateBox = const_cast<loader::Box*>(m_extraDoorStuff.alternateSector->box);
            }
            else
            {
                m_extraDoorStuff.alternateBox = const_cast<loader::Box*>(m_extraDoorStuff.alternateSector->portalTarget
                                                                                         ->getSectorByAbsolutePosition(
                                                                                                 wingsPosition )->box);
            }
            if( m_extraDoorStuff.alternateBox != nullptr
                && (m_extraDoorStuff.alternateBox->overlap_index & 0x8000u) == 0 )
            {
                m_extraDoorStuff.alternateBox = nullptr;
            }
            m_extraDoorStuff.alternateSectorData = *m_extraDoorStuff.alternateSector;
        }
        if( m_extraDoorStuff.sector != nullptr )
        {
            m_extraDoorStuff.sector->reset();
            if( m_extraDoorStuff.box != nullptr )
            {
                m_extraDoorStuff.box->overlap_index |= 0x4000u;
            }
        }
        if( m_extraDoorStuff.alternateSector != nullptr )
        {
            m_extraDoorStuff.alternateSector->reset();
            if( m_extraDoorStuff.alternateBox != nullptr )
            {
                m_extraDoorStuff.alternateBox->overlap_index |= 0x4000u;
            }
        }
        if( m_extraDoorStuff.sector->portalTarget == nullptr )
        {
            m_extraDoorStuff.targetSector = nullptr;
            m_extraDoorStuff.alternateTargetSector = nullptr;
            return;
        }

        m_extraDoorStuff.targetSector = const_cast<loader::Sector*>(m_extraDoorStuff.sector->portalTarget
                                                                                    ->getSectorByAbsolutePosition(
                                                                                            m_state.position
                                                                                                   .position ));
        if( m_extraDoorStuff.targetSector->portalTarget == nullptr )
        {
            m_extraDoorStuff.targetBox = const_cast<loader::Box*>(m_extraDoorStuff.targetSector->box);
        }
        else
        {
            m_extraDoorStuff.targetBox = const_cast<loader::Box*>(m_extraDoorStuff.targetSector->portalTarget
                                                                                  ->getSectorByAbsolutePosition(
                                                                                          m_state.position.position )
                                                                                  ->box);
        }
        if( (m_extraDoorStuff.targetBox->overlap_index & 0x8000) == 0 )
        {
            m_extraDoorStuff.targetBox = nullptr;
        }
        m_extraDoorStuff.targetSectorData = *m_extraDoorStuff.targetSector;
        if( m_extraDoorStuff.sector->portalTarget->alternateRoom == -1 )
        {
            m_extraDoorStuff.alternateTargetSector = nullptr;
        }
        else
        {
            m_extraDoorStuff.alternateTargetSector = const_cast<loader::Sector*>(getLevel().m_rooms[m_extraDoorStuff
                    .sector->portalTarget->alternateRoom].getSectorByAbsolutePosition( m_state.position.position ));
            if( m_extraDoorStuff.alternateTargetSector->portalTarget == nullptr )
            {
                m_extraDoorStuff.alternateTargetBox = const_cast<loader::Box*>(m_extraDoorStuff.alternateTargetSector
                                                                                               ->box);
            }
            else
            {
                m_extraDoorStuff.alternateTargetBox = const_cast<loader::Box*>(m_extraDoorStuff.alternateTargetSector
                                                                                               ->portalTarget
                                                                                               ->getSectorByAbsolutePosition(
                                                                                                       m_state.position
                                                                                                              .position )
                                                                                               ->box);
            }
            if( (m_extraDoorStuff.alternateTargetBox->overlap_index & 0x8000u) == 0 )
            {
                m_extraDoorStuff.alternateTargetBox = nullptr;
            }
            m_extraDoorStuff.alternateTargetSectorData = *m_extraDoorStuff.alternateTargetSector;
        }
        if( m_extraDoorStuff.targetSector )
        {
            m_extraDoorStuff.targetSector->reset();
            if( m_extraDoorStuff.targetBox != nullptr )
            {
                m_extraDoorStuff.targetBox->overlap_index |= 0x4000u;
            }
        }
        if( m_extraDoorStuff.alternateTargetSector )
        {
            m_extraDoorStuff.alternateTargetSector->reset();
            if( m_extraDoorStuff.alternateTargetBox != nullptr )
            {
                m_extraDoorStuff.alternateTargetBox->overlap_index |= 0x4000u;
            }
        }
    }

    void update() override
    {
        if( m_state.updateActivationTimeout() )
        {
            if( m_state.current_anim_state != 0 )
            {
                if( m_extraDoorStuff.sector )
                {
                    *m_extraDoorStuff.sector = m_extraDoorStuff.sectorData;
                    if( m_extraDoorStuff.box != nullptr )
                    {
                        m_extraDoorStuff.box->overlap_index &= ~0x4000u;
                    }
                }
                if( m_extraDoorStuff.targetSector )
                {
                    *m_extraDoorStuff.targetSector = m_extraDoorStuff.targetSectorData;
                    if( m_extraDoorStuff.targetBox != nullptr )
                    {
                        m_extraDoorStuff.targetBox->overlap_index &= ~0x4000u;
                    }
                }
                if( m_extraDoorStuff.alternateSector )
                {
                    *m_extraDoorStuff.alternateSector = m_extraDoorStuff.alternateSectorData;
                    if( m_extraDoorStuff.alternateBox != nullptr )
                    {
                        m_extraDoorStuff.alternateBox->overlap_index &= ~0x4000u;
                    }
                }
                if( m_extraDoorStuff.alternateTargetSector )
                {
                    *m_extraDoorStuff.alternateTargetSector = m_extraDoorStuff.alternateTargetSectorData;
                    if( m_extraDoorStuff.alternateTargetBox != nullptr )
                    {
                        m_extraDoorStuff.alternateTargetBox->overlap_index &= ~0x4000u;
                        ModelItemNode::update();
                        return;
                    }
                }
            }
            else
            {
                m_state.goal_anim_state = 1;
            }
        }
        else
        {
            if( m_state.current_anim_state == 1 )
            {
                m_state.goal_anim_state = 0;
                ModelItemNode::update();
                return;
            }

            if( m_extraDoorStuff.sector )
            {
                m_extraDoorStuff.sector->reset();
                if( m_extraDoorStuff.box != nullptr )
                {
                    m_extraDoorStuff.box->overlap_index |= 0x4000u;
                }
            }
            if( m_extraDoorStuff.targetSector )
            {
                m_extraDoorStuff.targetSector->reset();
                if( m_extraDoorStuff.targetBox != nullptr )
                {
                    m_extraDoorStuff.targetBox->overlap_index |= 0x4000u;
                }
            }
            if( m_extraDoorStuff.alternateSector )
            {
                m_extraDoorStuff.alternateSector->reset();
                if( m_extraDoorStuff.alternateBox != nullptr )
                {
                    m_extraDoorStuff.alternateBox->overlap_index |= 0x4000u;
                }
            }
            if( m_extraDoorStuff.alternateTargetSector )
            {
                m_extraDoorStuff.alternateTargetSector->reset();
                if( m_extraDoorStuff.alternateTargetBox != nullptr )
                {
                    m_extraDoorStuff.alternateTargetBox->overlap_index |= 0x4000u;
                }
            }
        }

        ModelItemNode::update();
    }

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override
    {
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
    }

private:
    struct ExtraDoorStuff
    {
        loader::Sector* sector{nullptr};
        loader::Sector sectorData;
        loader::Box* box{nullptr};
        loader::Sector* alternateSector{nullptr};
        loader::Sector alternateSectorData;
        loader::Box* alternateBox{nullptr};
        loader::Sector* targetSector{nullptr};
        loader::Sector targetSectorData;
        loader::Box* targetBox{nullptr};
        loader::Sector* alternateTargetSector{nullptr};
        loader::Sector alternateTargetSectorData;
        loader::Box* alternateTargetBox{nullptr};
    };

    ExtraDoorStuff m_extraDoorStuff;
};
}
}
