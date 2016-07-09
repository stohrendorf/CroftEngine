#include "itemcontroller.h"

#include "animationcontroller.h"
#include "laracontroller.h"

namespace engine
{
    uint16_t ItemController::getCurrentAnimState() const
    {
        return m_dispatcher->getCurrentAnimState();
    }

    bool ItemController::handleTRTransitions()
    {
        return m_dispatcher->handleTRTransitions();
    }

    void ItemController::handleAnimationEnd()
    {
        m_dispatcher->handleAnimationEnd();
    }


    void ItemController::applyRotation()
    {
        //! @todo This is horribly inefficient code, but it properly converts ZXY angles to XYZ angles.
        irr::core::quaternion q;
        q.makeIdentity();
        q *= irr::core::quaternion().fromAngleAxis(getRotation().Y.toRad(), {0,1,0});
        q *= irr::core::quaternion().fromAngleAxis(getRotation().X.toRad(), {-1,0,0});
        q *= irr::core::quaternion().fromAngleAxis(getRotation().Z.toRad(), {0,0,-1});

        irr::core::vector3df euler;
        q.toEuler(euler);
        m_sceneNode->setRotation(euler * 180 / irr::core::PI);
    }

    void ItemController::setTargetState(uint16_t st)
    {
        Expects(m_dispatcher != nullptr);
        m_dispatcher->setTargetState(st);
    }

    uint16_t ItemController::getTargetState() const
    {
        Expects(m_dispatcher != nullptr);
        return m_dispatcher->getTargetState();
    }

    void ItemController::playAnimation(uint16_t anim, const boost::optional<irr::u32>& firstFrame)
    {
        Expects(m_dispatcher != nullptr);
        m_dispatcher->playLocalAnimation(anim, firstFrame);
    }

    void ItemController::nextFrame()
    {
        Expects(m_dispatcher != nullptr);
        m_dispatcher->advanceFrame();
    }

    irr::u32 ItemController::getCurrentFrame() const
    {
        Expects(m_dispatcher != nullptr);
        return m_dispatcher->getCurrentFrame();
    }

    irr::u32 ItemController::getAnimEndFrame() const
    {
        Expects(m_dispatcher != nullptr);
        return m_dispatcher->getAnimEndFrame();
    }

    ItemController::ItemController(const gsl::not_null<level::Level*>& level, const std::shared_ptr<engine::MeshAnimationController>& dispatcher, const gsl::not_null<irr::scene::ISceneNode*>& sceneNode, const std::string & name, const gsl::not_null<const loader::Room*>& room, gsl::not_null<loader::Item*> item, bool hasProcessAnimCommandsOverride, uint8_t characteristics)
        : m_position(room, core::ExactTRCoordinates(item->position))
        , m_rotation(0_deg, core::Angle{ item->rotation }, 0_deg)
        , m_level(level)
        , m_sceneNode(sceneNode)
        , m_dispatcher(dispatcher)
        , m_name(name)
        , m_itemFlags(item->flags)
        , m_hasProcessAnimCommandsOverride(hasProcessAnimCommandsOverride)
        , m_characteristics(characteristics)
    {
        setCurrentRoom(room);

        if(m_itemFlags & Oneshot)
            m_sceneNode->setVisible(false);

        if((m_itemFlags & Oneshot) != 0)
        {
            m_itemFlags &= ~Oneshot;
            m_flags2_02_toggledOn = true;
            m_flags2_04_ready = true;
        }

        if((m_itemFlags & ActivationMask) == ActivationMask)
        {
            m_itemFlags &= ~ActivationMask;
            m_itemFlags |= InvertedActivation;
            activate();
            m_flags2_02_toggledOn = true;
            m_flags2_04_ready = false;
        }
    }

    irr::core::aabbox3di ItemController::getBoundingBox() const
    {
        if(m_dispatcher == nullptr)
        {
            BOOST_LOG_TRIVIAL(warning) << "Trying to get bounding box from non-animated item: " << getName();
            return irr::core::aabbox3di(0, 0, 0, 0, 0, 0);
        }

        Expects(m_dispatcher != nullptr);
        return m_dispatcher->getBoundingBox();
    }

    void ItemController::setCurrentRoom(const loader::Room* newRoom)
    {
        if( newRoom == m_position.room )
            return;

        BOOST_LOG_TRIVIAL(debug) << "Room switch of " << m_name << " to " << newRoom->node->getName();
        if( newRoom == nullptr )
        {
            BOOST_LOG_TRIVIAL(fatal) << "No room to switch to. Matching rooms by position:";
            for( size_t i = 0; i < m_level->m_rooms.size(); ++i )
            {
                const loader::Room& room = m_level->m_rooms[i];
                if( room.node->getTransformedBoundingBox().isPointInside(m_sceneNode->getAbsolutePosition()) )
                {
                    BOOST_LOG_TRIVIAL(fatal) << "  - " << i;
                }
            }
            return;
        }

        m_sceneNode->setParent(newRoom->node);

        m_position.room = newRoom;
        for( irr::u32 i = 0; i < m_sceneNode->getMaterialCount(); ++i )
        {
            irr::video::SMaterial& material = m_sceneNode->getMaterial(i);
            const auto col = m_position.room->lightColor.toSColor(1 - m_position.room->darkness / 8191.0f);
            material.DiffuseColor = col;
        }
    }

    uint16_t ItemController::getCurrentAnimationId() const
    {
        Expects(m_dispatcher != nullptr);
        return m_dispatcher->getCurrentAnimationId();
    }

    float ItemController::calculateAnimFloorSpeed() const
    {
        Expects(m_dispatcher != nullptr);
        return m_dispatcher->calculateFloorSpeed();
    }

    int ItemController::getAnimAccelleration() const
    {
        Expects(m_dispatcher != nullptr);
        return m_dispatcher->getAccelleration();
    }

    void ItemController::processAnimCommands(bool advanceFrame)
    {
        m_flags2_10 = false;

        if(advanceFrame)
            nextFrame();

        bool newFrame = advanceFrame;

        if(handleTRTransitions() || m_lastAnimFrame != getCurrentFrame())
        {
            m_lastAnimFrame = getCurrentFrame();
            newFrame = true;
        }

        const bool isAnimEnd = getCurrentFrame() >= getAnimEndFrame();

        const loader::Animation& animation = getLevel().m_animations[getCurrentAnimationId()];
        if(animation.animCommandCount > 0)
        {
            BOOST_ASSERT(animation.animCommandIndex < getLevel().m_animCommands.size());
            const auto* cmd = &getLevel().m_animCommands[animation.animCommandIndex];
            for(uint16_t i = 0; i < animation.animCommandCount; ++i)
            {
                BOOST_ASSERT(cmd < &getLevel().m_animCommands.back());
                const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
                ++cmd;
                switch(opcode)
                {
                    case AnimCommandOpcode::SetPosition:
                        if(isAnimEnd && newFrame)
                        {
                            moveLocal(
                                cmd[0],
                                cmd[1],
                                cmd[2]
                            );
                        }
                        cmd += 3;
                        break;
                    case AnimCommandOpcode::SetVelocity:
                        if(isAnimEnd && newFrame)
                        {
                            m_fallSpeed = cmd[0];
                            m_falling = true;
                            m_horizontalSpeed = cmd[1];
                        }
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::EmptyHands:
                        break;
                    case AnimCommandOpcode::PlaySound:
                        if(getCurrentFrame() == cmd[0])
                        {
                            //! @todo playsound(cmd[1])
                        }
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::PlayEffect:
                        if(getCurrentFrame() == cmd[0])
                        {
                            BOOST_LOG_TRIVIAL(debug) << "Anim effect: " << int(cmd[1]);
                            if(cmd[1] == 0 && newFrame)
                                addYRotation(180_deg);
                            else if(cmd[1] == 12)
                                getLevel().m_lara->setHandStatus(0);
                            //! @todo Execute anim effect cmd[1]
                        }
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::Kill:
                        if(isAnimEnd && newFrame)
                        {
                            m_flags2_02_toggledOn = false;
                            m_flags2_04_ready = true;
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        if(isAnimEnd)
        {
            handleAnimationEnd();
        }

        if(m_falling)
        {
            m_horizontalSpeed.add(getAnimAccelleration(), getCurrentDeltaTime());
            if(getFallSpeed() >= 128)
                m_fallSpeed.add(1, getCurrentDeltaTime());
            else
                m_fallSpeed.add(6, getCurrentDeltaTime());
        }
        else
        {
            m_horizontalSpeed = calculateAnimFloorSpeed();
        }

        move(
            getRotation().Y.sin() * m_horizontalSpeed.getScaled(getCurrentDeltaTime()),
            m_falling ? m_fallSpeed.getScaled(getCurrentDeltaTime()) : 0,
            getRotation().Y.cos() * m_horizontalSpeed.getScaled(getCurrentDeltaTime())
        );

        applyPosition();
    }

    void ItemController::activate()
    {
        if(!m_hasProcessAnimCommandsOverride)
        {
            m_flags2_02_toggledOn = false;
            m_flags2_04_ready = false;
            return;
        }

        if(m_isActive)
            BOOST_LOG_TRIVIAL(warning) << "Item controller " << m_name << " already active";
        else
            BOOST_LOG_TRIVIAL(trace) << "Activating item controller " << m_name;

        m_isActive = true;
    }

    void ItemController::deactivate()
    {
        if(!m_isActive)
            BOOST_LOG_TRIVIAL(warning) << "Item controller " << m_name << " already inactive";
        else
            BOOST_LOG_TRIVIAL(trace) << "Deactivating item controller " << m_name;

        m_isActive = false;
    }

    void ItemController_55_Switch::onInteract(LaraController& lara, LaraState& /*state*/)
    {
        if(!getLevel().m_inputHandler->getInputState().action)
            return;

        if(lara.getHandStatus() != 0)
            return;

        if(lara.isFalling())
            return;

        if(m_flags2_04_ready || m_flags2_02_toggledOn)
            return;

        if(lara.getCurrentState() != loader::LaraStateId::Stop)
            return;

        static const InteractionLimits limits{
            irr::core::aabbox3di{{-200, 0, 312}, {200, 0, 512}},
            {-10_deg,-30_deg,-10_deg},
            {+10_deg,+30_deg,+10_deg}
        };

        if(!limits.canInteract(*this, lara))
            return;

        lara.setYRotation(getRotation().Y);

        if(getCurrentAnimState() == 1)
        {
            BOOST_LOG_TRIVIAL(debug) << "Switch " << getName() << ": pull down";
            lara.setTargetState(loader::LaraStateId::SwitchDown);
            do
            {
                lara.processLaraAnimCommands(true);
            } while(lara.getCurrentAnimState() != loader::LaraStateId::SwitchDown);
            lara.setTargetState(loader::LaraStateId::Stop);
            setTargetState(0);
            lara.setHandStatus(1);
        }
        else
        {
            if(getCurrentAnimState() != 0)
                return;

            BOOST_LOG_TRIVIAL(debug) << "Switch " << getName() << ": pull up";
            lara.setTargetState(loader::LaraStateId::SwitchUp);
            do
            {
                lara.processLaraAnimCommands(true);
            } while(lara.getCurrentAnimState() != loader::LaraStateId::SwitchUp);
            lara.setTargetState(loader::LaraStateId::Stop);
            setTargetState(1);
            lara.setHandStatus(1);
        }

        m_flags2_04_ready = false;
        m_flags2_02_toggledOn = true;

        activate();
        ItemController::processAnimCommands();
    }

    bool InteractionLimits::canInteract(const ItemController& item, const LaraController& lara) const
    {
        const auto angle = lara.getRotation() - item.getRotation();
        if(   angle.X < minAngle.X || angle.X > maxAngle.X
           || angle.Y < minAngle.Y || angle.Y > maxAngle.Y
           || angle.Z < minAngle.Z || angle.Z > maxAngle.Z)
        {
            return false;
        }

        irr::core::quaternion q;
        q.makeIdentity();
        q *= irr::core::quaternion().fromAngleAxis(item.getRotation().Y.toRad(), { 0,1,0 });
        q *= irr::core::quaternion().fromAngleAxis(item.getRotation().X.toRad(), { -1,0,0 });
        q *= irr::core::quaternion().fromAngleAxis(item.getRotation().Z.toRad(), { 0,0,-1 });

        irr::core::matrix4 m = q.getMatrix();

        const auto dist = lara.getPosition() - item.getPosition();
        const auto dx = m(0, 0) * dist.X + m(0, 1) * dist.Y + m(0, 2) * dist.Z;
        const auto dy = m(1, 0) * dist.X + m(1, 1) * dist.Y + m(1, 2) * dist.Z;
        const auto dz = m(2, 0) * dist.X + m(2, 1) * dist.Y + m(2, 2) * dist.Z;

        return distance.isPointInside(irr::core::vector3di(dx, dy, dz));
    }

    void ItemController_Door::onInteract(LaraController& /*lara*/, LaraState& /*state*/)
    {

    }

    void ItemController_35_CollapsibleFloor::processAnimCommands(bool advanceFrame)
    {
        if(getCurrentAnimState() == 0) // stationary
        {
            if(!irr::core::equals(getPosition().Y - 512, getLevel().m_lara->getPosition().Y, 1.0f))
            {
                m_flags2_02_toggledOn = false;
                m_flags2_04_ready = false;
                deactivate();
                return;
            }
            setTargetState(1);
        }
        else if(getCurrentAnimState() == 1) // shaking
        {
            setTargetState(2);
        }
        else if(getCurrentAnimState() == 2 && getTargetState() != 3) // falling, not going to settle
        {
            setFalling(true);
        }

        ItemController::processAnimCommands(advanceFrame);

        if(m_flags2_04_ready && !m_flags2_02_toggledOn)
        {
            deactivate();
            return;
        }

        auto room = getCurrentRoom();
        auto sector = getLevel().findFloorSectorWithClampedPosition(getPosition().toInexact(), &room);
        setCurrentRoom(room);

        HeightInfo h = HeightInfo::fromFloor(sector, getPosition().toInexact(), getLevel().m_cameraController);
        setFloorHeight(h.distance);
        if(getCurrentAnimState() != 2 || getPosition().Y < h.distance)
            return;

        // settle
        setTargetState(3);
        setFallSpeed(core::makeInterpolatedValue(0.0f));
        auto pos = getPosition();
        pos.Y = getFloorHeight();
        setPosition(pos);
        setFalling(false);
    }

    void ItemController_Block::onInteract(LaraController& lara, LaraState & state)
    {
        if(!getLevel().m_inputHandler->getInputState().action || (m_flags2_02_toggledOn && !m_flags2_04_ready) || isFalling() || !irr::core::equals(lara.getPosition().Y, getPosition().Y, 1.0f))
            return;

        static const InteractionLimits limits{
            irr::core::aabbox3di{ { -300, 0, -692 },{ 200, 0, -512 } },
            { -10_deg,-30_deg,-10_deg },
            { +10_deg,+30_deg,+10_deg }
        };

        auto axis = core::axisFromAngle(lara.getRotation().Y, 45_deg);
        Expects(axis.is_initialized());

        if(lara.getCurrentAnimState() == loader::LaraStateId::Stop)
        {
            if(getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Null || lara.getHandStatus() != 0)
                return;

            setYRotation(core::alignRotation(*axis));

            if(!limits.canInteract(*this, lara))
                return;

            switch(*axis)
            {
                case core::Axis::PosZ:
                {
                    auto pos = lara.getPosition();
                    pos.Z = std::floor(pos.Z / loader::SectorSize)*loader::SectorSize + 924;
                    lara.setPosition(pos);
                    break;
                }
                case core::Axis::PosX:
                {
                    auto pos = lara.getPosition();
                    pos.X = std::floor(pos.X / loader::SectorSize)*loader::SectorSize + 924;
                    lara.setPosition(pos);
                    break;
                }
                case core::Axis::NegZ:
                {
                    auto pos = lara.getPosition();
                    pos.Z = std::floor(pos.Z / loader::SectorSize)*loader::SectorSize + 100;
                    lara.setPosition(pos);
                    break;
                }
                case core::Axis::NegX:
                {
                    auto pos = lara.getPosition();
                    pos.X = std::floor(pos.X / loader::SectorSize)*loader::SectorSize + 100;
                    lara.setPosition(pos);
                    break;
                }
                default:
                    break;
            }

            lara.setYRotation(getRotation().Y);
            lara.setTargetState(loader::LaraStateId::PushableGrab);
            lara.processLaraAnimCommands(true);
            if(lara.getCurrentAnimState() == loader::LaraStateId::PushableGrab)
                lara.setHandStatus(1);
            return;
        }

        if(lara.getCurrentAnimState() != loader::LaraStateId::PushableGrab || lara.getCurrentFrame() != 2091 || !limits.canInteract(*this, lara))
            return;

        if(getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward)
        {
            if(!canPushBlock(loader::SectorSize, *axis))
                return;

            setTargetState(2);
            lara.setTargetState(loader::LaraStateId::PushablePush);
        }
        else if(getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward)
        {
            if(!canPullBlock(loader::SectorSize, *axis))
                return;

            setTargetState(3);
            lara.setTargetState(loader::LaraStateId::PushablePull);
        }
        else
        {
            return;
        }

        activate();
        loader::Room::patchHeightsForBlock(*this, loader::SectorSize);
        m_flags2_02_toggledOn = true;
        m_flags2_04_ready = false;
        ItemController::processAnimCommands();
        lara.processLaraAnimCommands();
    }

    void ItemController_Block::processAnimCommands(bool advanceFrame)
    {
        if((m_itemFlags & Oneshot) != 0)
        {
            loader::Room::patchHeightsForBlock(*this, loader::SectorSize);
            m_isActive = false;
            m_itemFlags |= Locked;
            return;
        }

        ItemController::processAnimCommands(advanceFrame);

        auto pos = getRoomBoundPosition();
        auto sector = getLevel().findFloorSectorWithClampedPosition(pos);
        auto height = HeightInfo::fromFloor(sector, pos.position.toInexact(), getLevel().m_cameraController).distance;
        if(height > pos.position.Y)
        {
            setFalling(true);
        }
        else if(isFalling())
        {
            pos.position.Y = height;
            setPosition(pos.position);
            setFalling(false);
            m_flags2_02_toggledOn = false;
            m_flags2_04_ready = true;
            //! @todo Shake camera
            //! @todo play sound 70
        }

        setCurrentRoom(pos.room);

        if(m_flags2_02_toggledOn || !m_flags2_04_ready)
            return;

        m_flags2_02_toggledOn = false;
        m_flags2_04_ready = false;
        deactivate();
        loader::Room::patchHeightsForBlock(*this, -loader::SectorSize);
        pos = getRoomBoundPosition();
        sector = getLevel().findFloorSectorWithClampedPosition(pos);
        HeightInfo hi = HeightInfo::fromFloor(sector, pos.position.toInexact(), getLevel().m_cameraController);
        getLevel().m_lara->handleTriggers(hi.lastTriggerOrKill, true);
    }

    bool ItemController_Block::isOnFloor(int height) const
    {
        auto sector = getLevel().findFloorSectorWithClampedPosition(getPosition().toInexact(), getCurrentRoom());
        return sector->floorHeight == -127 || irr::core::equals(gsl::narrow_cast<float>(sector->floorHeight*loader::QuarterSectorSize), getPosition().Y - height, 1.0f);
    }

    bool ItemController_Block::canPushBlock(int height, core::Axis axis) const
    {
        if(!isOnFloor(height))
            return false;

        auto pos = getPosition();
        switch(axis)
        {
            case core::Axis::PosZ: pos.Z += loader::SectorSize; break;
            case core::Axis::PosX: pos.X += loader::SectorSize; break;
            case core::Axis::NegZ: pos.Z -= loader::SectorSize; break;
            case core::Axis::NegX: pos.X -= loader::SectorSize; break;
            default: break;
        }

        LaraState tmp;
        tmp.orientationAxis = axis;
        tmp.collisionRadius = 500;
        if(tmp.checkStaticMeshCollisions(pos, 1000, getLevel()))
            return false;

        auto targetSector = getLevel().findFloorSectorWithClampedPosition(pos.toInexact(), getCurrentRoom());
        if(!irr::core::equals(gsl::narrow_cast<float>(targetSector->floorHeight*loader::QuarterSectorSize), pos.Y, 1.0f))
            return false;

        pos.Y -= height;
        return pos.Y >= getLevel().findFloorSectorWithClampedPosition(pos.toInexact(), getCurrentRoom())->ceilingHeight * loader::QuarterSectorSize;
    }

    bool ItemController_Block::canPullBlock(int height, core::Axis axis) const
    {
        if(!isOnFloor(height))
            return false;

        auto pos = getPosition();
        switch(axis)
        {
            case core::Axis::PosZ: pos.Z -= loader::SectorSize; break;
            case core::Axis::PosX: pos.X -= loader::SectorSize; break;
            case core::Axis::NegZ: pos.Z += loader::SectorSize; break;
            case core::Axis::NegX: pos.X += loader::SectorSize; break;
            default: break;
        }

        auto room = getCurrentRoom();
        auto sector = getLevel().findFloorSectorWithClampedPosition(pos.toInexact(), &room);

        LaraState tmp;
        tmp.orientationAxis = axis;
        tmp.collisionRadius = 500;
        if(tmp.checkStaticMeshCollisions(pos, 1000, getLevel()))
            return false;

        if(!irr::core::equals(gsl::narrow_cast<float>(sector->floorHeight*loader::QuarterSectorSize), pos.Y, 1.0f))
            return false;

        auto topPos = pos;
        topPos.Y -= height;
        auto topSector = getLevel().findFloorSectorWithClampedPosition(topPos.toInexact(), getCurrentRoom());
        if(topPos.Y < topSector->ceilingHeight * loader::QuarterSectorSize)
            return false;

        auto laraPos = pos;
        switch(axis)
        {
            case core::Axis::PosZ: laraPos.Z -= loader::SectorSize; break;
            case core::Axis::PosX: laraPos.X -= loader::SectorSize; break;
            case core::Axis::NegZ: laraPos.Z += loader::SectorSize; break;
            case core::Axis::NegX: laraPos.X += loader::SectorSize; break;
            default: break;
        }

        sector = getLevel().findFloorSectorWithClampedPosition(laraPos.toInexact(), &room);
        if(!irr::core::equals(gsl::narrow_cast<float>(sector->floorHeight*loader::QuarterSectorSize), pos.Y, 1.0f))
            return false;

        laraPos.Y -= core::ScalpHeight;
        sector = getLevel().findFloorSectorWithClampedPosition(laraPos.toInexact(), &room);
        if(laraPos.Y < sector->ceilingHeight * loader::QuarterSectorSize)
            return false;

        laraPos = getLevel().m_lara->getPosition();
        switch(axis)
        {
            case core::Axis::PosZ:
                laraPos.Z -= loader::SectorSize;
                tmp.orientationAxis = core::Axis::NegZ;
                break;
            case core::Axis::PosX:
                laraPos.X -= loader::SectorSize;
                tmp.orientationAxis = core::Axis::NegX;
                break;
            case core::Axis::NegZ:
                laraPos.Z += loader::SectorSize;
                tmp.orientationAxis = core::Axis::PosZ;
                break;
            case core::Axis::NegX:
                laraPos.X += loader::SectorSize;
                tmp.orientationAxis = core::Axis::PosX;
                break;
            default: break;
        }
        tmp.collisionRadius = 100;

        return !tmp.checkStaticMeshCollisions(laraPos, core::ScalpHeight, getLevel());
    }

    void ItemController_TallBlock::processAnimCommands(bool advanceFrame)
    {
        ItemController::processAnimCommands(advanceFrame);
        auto room = getCurrentRoom();
        auto sector = getLevel().findFloorSectorWithClampedPosition(getPosition().toInexact(), &room);
        setCurrentRoom(room);

        if(m_flags2_02_toggledOn || !m_flags2_04_ready)
            return;

        m_flags2_02_toggledOn = true;
        m_flags2_04_ready = false;
        loader::Room::patchHeightsForBlock(*this, -2 * loader::SectorSize);
        auto pos = getPosition();
        pos.X = std::floor(pos.X / loader::SectorSize) * loader::SectorSize + loader::SectorSize / 2;
        pos.Z = std::floor(pos.Z / loader::SectorSize) * loader::SectorSize + loader::SectorSize / 2;
        setPosition(pos);
    }
    void ItemController_41_TrapDoorUp::processAnimCommands(bool advanceFrame)
    {
        ItemController::processAnimCommands(advanceFrame);
        auto pos = getRoomBoundPosition();
        getLevel().findFloorSectorWithClampedPosition(pos);
        setCurrentRoom(pos.room);
    }
}
