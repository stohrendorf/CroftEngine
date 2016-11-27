#include "block.h"

#include "engine/laranode.h"
#include "level/level.h"

namespace engine
{
    namespace items
    {
        void Item_Block::onInteract(LaraNode& lara)
        {
            if( !getLevel().m_inputHandler->getInputState().action || (m_flags2_02_toggledOn && !m_flags2_04_ready)
                || isFalling() || !util::fuzzyEqual( lara.getPosition().Y, getPosition().Y, 1.0f ) )
                return;

            static const InteractionLimits limits{
                    gameplay::BoundingBox{{-300, 0, -692},
                                          {200,  0, -512}},
                    {-10_deg, -30_deg, -10_deg},
                    {+10_deg, +30_deg, +10_deg}
            };

            auto axis = core::axisFromAngle( lara.getRotation().Y, 45_deg );
            Expects( axis.is_initialized() );

            if( lara.getCurrentAnimState() == loader::LaraStateId::Stop )
            {
                if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Null
                    || lara.getHandStatus() != 0 )
                    return;

                setYRotation( core::alignRotation( *axis ) );

                if( !limits.canInteract( *this, lara ) )
                    return;

                switch( *axis )
                {
                    case core::Axis::PosZ:
                    {
                        auto pos = lara.getPosition();
                        pos.Z = std::floor( pos.Z / loader::SectorSize ) * loader::SectorSize + 924;
                        lara.setPosition( pos );
                        break;
                    }
                    case core::Axis::PosX:
                    {
                        auto pos = lara.getPosition();
                        pos.X = std::floor( pos.X / loader::SectorSize ) * loader::SectorSize + 924;
                        lara.setPosition( pos );
                        break;
                    }
                    case core::Axis::NegZ:
                    {
                        auto pos = lara.getPosition();
                        pos.Z = std::floor( pos.Z / loader::SectorSize ) * loader::SectorSize + 100;
                        lara.setPosition( pos );
                        break;
                    }
                    case core::Axis::NegX:
                    {
                        auto pos = lara.getPosition();
                        pos.X = std::floor( pos.X / loader::SectorSize ) * loader::SectorSize + 100;
                        lara.setPosition( pos );
                        break;
                    }
                    default:break;
                }

                lara.setYRotation( getRotation().Y );
                lara.setTargetState( loader::LaraStateId::PushableGrab );
                lara.advanceFrame();
                if( lara.getCurrentAnimState() == loader::LaraStateId::PushableGrab )
                    lara.setHandStatus( 1 );
                return;
            }

            if( lara.getCurrentAnimState() != loader::LaraStateId::PushableGrab
                || core::toFrame( lara.getCurrentTime() ) != 2091 || !limits.canInteract( *this, lara ) )
                return;

            if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
            {
                if( !canPushBlock( loader::SectorSize, *axis ) )
                    return;

                setTargetState( 2 );
                lara.setTargetState( loader::LaraStateId::PushablePush );
            }
            else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward )
            {
                if( !canPullBlock( loader::SectorSize, *axis ) )
                    return;

                setTargetState( 3 );
                lara.setTargetState( loader::LaraStateId::PushablePull );
            }
            else
            {
                return;
            }

            activate();
            loader::Room::patchHeightsForBlock( *this, loader::SectorSize );
            m_flags2_02_toggledOn = true;
            m_flags2_04_ready = false;
        }


        void Item_Block::onFrameChanged(FrameChangeType frameChangeType)
        {
            if( (m_itemFlags & Oneshot) != 0 )
            {
                loader::Room::patchHeightsForBlock( *this, loader::SectorSize );
                m_isActive = false;
                m_itemFlags |= Locked;
                return;
            }

            ItemNode::onFrameChanged( frameChangeType );

            auto pos = getRoomBoundPosition();
            auto sector = getLevel().findFloorSectorWithClampedPosition( pos );
            auto height = HeightInfo::fromFloor( sector, pos.position.toInexact(), getLevel().m_cameraController )
                    .distance;
            if( height > pos.position.Y )
            {
                setFalling( true );
            }
            else if( isFalling() )
            {
                pos.position.Y = height;
                setPosition( pos.position );
                setFalling( false );
                m_flags2_02_toggledOn = false;
                m_flags2_04_ready = true;
                //! @todo Shake camera
                playSoundEffect( 70 );
            }

            setCurrentRoom( pos.room );

            if( m_flags2_02_toggledOn || !m_flags2_04_ready )
                return;

            m_flags2_02_toggledOn = false;
            m_flags2_04_ready = false;
            deactivate();
            loader::Room::patchHeightsForBlock( *this, -loader::SectorSize );
            pos = getRoomBoundPosition();
            sector = getLevel().findFloorSectorWithClampedPosition( pos );
            HeightInfo hi = HeightInfo::fromFloor( sector, pos.position.toInexact(), getLevel().m_cameraController );
            getLevel().m_lara->handleTriggers( hi.lastTriggerOrKill, true );
        }


        bool Item_Block::isOnFloor(int height) const
        {
            auto sector = getLevel().findFloorSectorWithClampedPosition( getPosition().toInexact(), getCurrentRoom() );
            return sector->floorHeight == -127
                   || util::fuzzyEqual( gsl::narrow_cast<float>( sector->floorHeight * loader::QuarterSectorSize ),
                                        getPosition().Y - height, 1.0f );
        }


        bool Item_Block::canPushBlock(int height, core::Axis axis) const
        {
            if( !isOnFloor( height ) )
                return false;

            auto pos = getPosition();
            switch( axis )
            {
                case core::Axis::PosZ: pos.Z += loader::SectorSize;
                    break;
                case core::Axis::PosX: pos.X += loader::SectorSize;
                    break;
                case core::Axis::NegZ: pos.Z -= loader::SectorSize;
                    break;
                case core::Axis::NegX: pos.X -= loader::SectorSize;
                    break;
                default: break;
            }

            CollisionInfo tmp;
            tmp.orientationAxis = axis;
            tmp.collisionRadius = 500;
            if( tmp.checkStaticMeshCollisions( pos, 1000, getLevel() ) )
                return false;

            auto targetSector = getLevel().findFloorSectorWithClampedPosition( pos.toInexact(), getCurrentRoom() );
            if( !util::fuzzyEqual( gsl::narrow_cast<float>( targetSector->floorHeight * loader::QuarterSectorSize ),
                                   pos.Y, 1.0f ) )
                return false;

            pos.Y -= height;
            return pos.Y
                   >= getLevel().findFloorSectorWithClampedPosition( pos.toInexact(), getCurrentRoom() )->ceilingHeight
                      * loader::QuarterSectorSize;
        }


        bool Item_Block::canPullBlock(int height, core::Axis axis) const
        {
            if( !isOnFloor( height ) )
                return false;

            auto pos = getPosition();
            switch( axis )
            {
                case core::Axis::PosZ: pos.Z -= loader::SectorSize;
                    break;
                case core::Axis::PosX: pos.X -= loader::SectorSize;
                    break;
                case core::Axis::NegZ: pos.Z += loader::SectorSize;
                    break;
                case core::Axis::NegX: pos.X += loader::SectorSize;
                    break;
                default: break;
            }

            auto room = getCurrentRoom();
            auto sector = getLevel().findFloorSectorWithClampedPosition( pos.toInexact(), &room );

            CollisionInfo tmp;
            tmp.orientationAxis = axis;
            tmp.collisionRadius = 500;
            if( tmp.checkStaticMeshCollisions( pos, 1000, getLevel() ) )
                return false;

            if( !util::fuzzyEqual( gsl::narrow_cast<float>( sector->floorHeight * loader::QuarterSectorSize ), pos.Y,
                                   1.0f ) )
                return false;

            auto topPos = pos;
            topPos.Y -= height;
            auto topSector = getLevel().findFloorSectorWithClampedPosition( topPos.toInexact(), getCurrentRoom() );
            if( topPos.Y < topSector->ceilingHeight * loader::QuarterSectorSize )
                return false;

            auto laraPos = pos;
            switch( axis )
            {
                case core::Axis::PosZ: laraPos.Z -= loader::SectorSize;
                    break;
                case core::Axis::PosX: laraPos.X -= loader::SectorSize;
                    break;
                case core::Axis::NegZ: laraPos.Z += loader::SectorSize;
                    break;
                case core::Axis::NegX: laraPos.X += loader::SectorSize;
                    break;
                default: break;
            }

            sector = getLevel().findFloorSectorWithClampedPosition( laraPos.toInexact(), &room );
            if( !util::fuzzyEqual( gsl::narrow_cast<float>( sector->floorHeight * loader::QuarterSectorSize ), pos.Y,
                                   1.0f ) )
                return false;

            laraPos.Y -= core::ScalpHeight;
            sector = getLevel().findFloorSectorWithClampedPosition( laraPos.toInexact(), &room );
            if( laraPos.Y < sector->ceilingHeight * loader::QuarterSectorSize )
                return false;

            laraPos = getLevel().m_lara->getPosition();
            switch( axis )
            {
                case core::Axis::PosZ:laraPos.Z -= loader::SectorSize;
                    tmp.orientationAxis = core::Axis::NegZ;
                    break;
                case core::Axis::PosX:laraPos.X -= loader::SectorSize;
                    tmp.orientationAxis = core::Axis::NegX;
                    break;
                case core::Axis::NegZ:laraPos.Z += loader::SectorSize;
                    tmp.orientationAxis = core::Axis::PosZ;
                    break;
                case core::Axis::NegX:laraPos.X += loader::SectorSize;
                    tmp.orientationAxis = core::Axis::PosX;
                    break;
                default: break;
            }
            tmp.collisionRadius = 100;

            return !tmp.checkStaticMeshCollisions( laraPos, core::ScalpHeight, getLevel() );
        }
    }
}
