#include "block.h"

#include "engine/laranode.h"
#include "level/level.h"
#include "core/boundingbox.h"


namespace engine
{
    namespace items
    {
        void Block::onInteract(LaraNode& lara)
        {
            if( !getLevel().m_inputHandler->getInputState().action || m_triggerState == TriggerState::Enabled
                || isFalling() || lara.getPosition().Y != getPosition().Y )
                return;

            static const InteractionLimits limits{
                core::BoundingBox{{-300, 0, -692}, {200, 0, -512}},
                {-10_deg, -30_deg, -10_deg},
                {+10_deg, +30_deg, +10_deg}
            };

            auto axis = core::axisFromAngle(lara.getRotation().Y, 45_deg);
            Expects( axis.is_initialized() );

            if( lara.getCurrentAnimState() == loader::LaraStateId::Stop )
            {
                if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Null
                    || lara.getHandStatus() != 0 )
                    return;

                setYRotation(core::alignRotation(*axis));

                if( !limits.canInteract(*this, lara) )
                    return;

                switch( *axis )
                {
                    case core::Axis::PosZ:
                    {
                        auto pos = lara.getPosition();
                        pos.Z = (pos.Z / loader::SectorSize) * loader::SectorSize + 924;
                        lara.setPosition(pos);
                        break;
                    }
                    case core::Axis::PosX:
                    {
                        auto pos = lara.getPosition();
                        pos.X = (pos.X / loader::SectorSize) * loader::SectorSize + 924;
                        lara.setPosition(pos);
                        break;
                    }
                    case core::Axis::NegZ:
                    {
                        auto pos = lara.getPosition();
                        pos.Z = (pos.Z / loader::SectorSize) * loader::SectorSize + 100;
                        lara.setPosition(pos);
                        break;
                    }
                    case core::Axis::NegX:
                    {
                        auto pos = lara.getPosition();
                        pos.X = (pos.X / loader::SectorSize) * loader::SectorSize + 100;
                        lara.setPosition(pos);
                        break;
                    }
                    default:
                        break;
                }

                lara.setYRotation(getRotation().Y);
                lara.setTargetState(loader::LaraStateId::PushableGrab);
                lara.updateImpl();
                if( lara.getCurrentAnimState() == loader::LaraStateId::PushableGrab )
                    lara.setHandStatus(1);
                return;
            }

            if( lara.getCurrentAnimState() != loader::LaraStateId::PushableGrab
                || lara.getNode()->getCurrentFrame() != 2091 || !limits.canInteract(*this, lara) )
                return;

            if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
            {
                if( !canPushBlock(loader::SectorSize, *axis) )
                    return;

                getNode()->setTargetState(2);
                lara.setTargetState(loader::LaraStateId::PushablePush);
            }
            else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward )
            {
                if( !canPullBlock(loader::SectorSize, *axis) )
                    return;

                getNode()->setTargetState(3);
                lara.setTargetState(loader::LaraStateId::PushablePull);
            }
            else
            {
                return;
            }

            activate();
            loader::Room::patchHeightsForBlock(*this, loader::SectorSize);
            m_triggerState = TriggerState::Enabled;

            ModelItemNode::update();
            getLevel().m_lara->updateImpl();
        }


        void Block::update()
        {
            if(m_activationState.isOneshot())
            {
                loader::Room::patchHeightsForBlock(*this, loader::SectorSize);
                m_isActive = false;
                m_activationState.setLocked(true);
                return;
            }

            ModelItemNode::update();

            auto pos = getRoomBoundPosition();
            auto sector = getLevel().findRealFloorSector(pos);
            auto height = HeightInfo::fromFloor(sector, pos.position, getLevel().m_cameraController)
                .distance;
            if( height > pos.position.Y )
            {
                setFalling(true);
            }
            else if( isFalling() )
            {
                pos.position.Y = height;
                setPosition(pos.position);
                setFalling(false);
                m_triggerState = TriggerState::Activated;
                //! @todo Shake camera
                playSoundEffect(70);
            }

            setCurrentRoom(pos.room);

            if( m_triggerState != TriggerState::Activated )
                return;

            m_triggerState = TriggerState::Disabled;
            deactivate();
            loader::Room::patchHeightsForBlock(*this, -loader::SectorSize);
            pos = getRoomBoundPosition();
            sector = getLevel().findRealFloorSector(pos);
            HeightInfo hi = HeightInfo::fromFloor(sector, pos.position, getLevel().m_cameraController);
            getLevel().m_lara->handleCommandSequence(hi.lastCommandSequenceOrDeath, true);
        }


        bool Block::isOnFloor(int height) const
        {
            auto sector = getLevel().findRealFloorSector(getPosition(), getCurrentRoom());
            return sector->floorHeight == -127
                || sector->floorHeight * loader::QuarterSectorSize == getPosition().Y - height;
        }


        bool Block::canPushBlock(int height, core::Axis axis) const
        {
            if( !isOnFloor(height) )
                return false;

            auto pos = getPosition();
            switch( axis )
            {
                case core::Axis::PosZ:
                    pos.Z += loader::SectorSize;
                    break;
                case core::Axis::PosX:
                    pos.X += loader::SectorSize;
                    break;
                case core::Axis::NegZ:
                    pos.Z -= loader::SectorSize;
                    break;
                case core::Axis::NegX:
                    pos.X -= loader::SectorSize;
                    break;
                default:
                    break;
            }

            CollisionInfo tmp;
            tmp.facingAxis = axis;
            tmp.collisionRadius = 500;
            if( tmp.checkStaticMeshCollisions(pos, 1000, getLevel()) )
                return false;

            auto targetSector = getLevel().findRealFloorSector(pos, getCurrentRoom());
            if( targetSector->floorHeight * loader::QuarterSectorSize != pos.Y )
                return false;

            pos.Y -= height;
            return pos.Y
                >= getLevel().findRealFloorSector(pos, getCurrentRoom())->ceilingHeight
                * loader::QuarterSectorSize;
        }


        bool Block::canPullBlock(int height, core::Axis axis) const
        {
            if( !isOnFloor(height) )
                return false;

            auto pos = getPosition();
            switch( axis )
            {
                case core::Axis::PosZ:
                    pos.Z -= loader::SectorSize;
                    break;
                case core::Axis::PosX:
                    pos.X -= loader::SectorSize;
                    break;
                case core::Axis::NegZ:
                    pos.Z += loader::SectorSize;
                    break;
                case core::Axis::NegX:
                    pos.X += loader::SectorSize;
                    break;
                default:
                    break;
            }

            auto room = getCurrentRoom();
            auto sector = getLevel().findRealFloorSector(pos, &room);

            CollisionInfo tmp;
            tmp.facingAxis = axis;
            tmp.collisionRadius = 500;
            if( tmp.checkStaticMeshCollisions(pos, 1000, getLevel()) )
                return false;

            if( sector->floorHeight * loader::QuarterSectorSize != pos.Y )
                return false;

            auto topPos = pos;
            topPos.Y -= height;
            auto topSector = getLevel().findRealFloorSector(topPos, getCurrentRoom());
            if( topPos.Y < topSector->ceilingHeight * loader::QuarterSectorSize )
                return false;

            auto laraPos = pos;
            switch( axis )
            {
                case core::Axis::PosZ:
                    laraPos.Z -= loader::SectorSize;
                    break;
                case core::Axis::PosX:
                    laraPos.X -= loader::SectorSize;
                    break;
                case core::Axis::NegZ:
                    laraPos.Z += loader::SectorSize;
                    break;
                case core::Axis::NegX:
                    laraPos.X += loader::SectorSize;
                    break;
                default:
                    break;
            }

            sector = getLevel().findRealFloorSector(laraPos, &room);
            if( sector->floorHeight * loader::QuarterSectorSize != pos.Y )
                return false;

            laraPos.Y -= core::ScalpHeight;
            sector = getLevel().findRealFloorSector(laraPos, &room);
            if( laraPos.Y < sector->ceilingHeight * loader::QuarterSectorSize )
                return false;

            laraPos = getLevel().m_lara->getPosition();
            switch( axis )
            {
                case core::Axis::PosZ:
                    laraPos.Z -= loader::SectorSize;
                    tmp.facingAxis = core::Axis::NegZ;
                    break;
                case core::Axis::PosX:
                    laraPos.X -= loader::SectorSize;
                    tmp.facingAxis = core::Axis::NegX;
                    break;
                case core::Axis::NegZ:
                    laraPos.Z += loader::SectorSize;
                    tmp.facingAxis = core::Axis::PosZ;
                    break;
                case core::Axis::NegX:
                    laraPos.X += loader::SectorSize;
                    tmp.facingAxis = core::Axis::PosX;
                    break;
                default:
                    break;
            }
            tmp.collisionRadius = core::DefaultCollisionRadius;

            return !tmp.checkStaticMeshCollisions(laraPos, core::ScalpHeight, getLevel());
        }
    }
}
