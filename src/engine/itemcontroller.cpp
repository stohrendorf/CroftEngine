#include "itemcontroller.h"

#include "laracontroller.h"
#include "level/level.h"
#include "loader/item.h"


namespace engine
{
    void ItemController::applyTransform()
    {
        glm::vec3 tr;

        if( auto parent = m_position.room )
            tr = m_position.position.toRenderSystem() - parent->position.toRenderSystem();
        else
            tr = m_position.position.toRenderSystem();

        setLocalMatrix(glm::translate(glm::mat4{1.0f}, tr) * getRotation().toMatrix());

        updateSounds();
    }


    ItemController::ItemController(const gsl::not_null<level::Level*>& level,
                                   const std::string& name,
                                   const gsl::not_null<const loader::Room*>& room,
                                   gsl::not_null<loader::Item*> item,
                                   bool hasProcessAnimCommandsOverride,
                                   uint8_t characteristics,
                                   const loader::AnimatedModel& animatedModel)
        : SkeletalModelNode(name, level, animatedModel)
        , m_position(room, core::ExactTRCoordinates(item->position))
        , m_rotation(0_deg, core::Angle{item->rotation}, 0_deg)
        , m_level(level)
        , m_itemFlags(item->flags)
        , m_hasProcessAnimCommandsOverride(hasProcessAnimCommandsOverride)
        , m_characteristics(characteristics)
    {
        if( m_itemFlags & Oneshot )
            setEnabled(false);

        if( (m_itemFlags & Oneshot) != 0 )
        {
            m_itemFlags &= ~Oneshot;
            m_flags2_02_toggledOn = true;
            m_flags2_04_ready = true;
        }

        if( (m_itemFlags & ActivationMask) == ActivationMask )
        {
            m_itemFlags &= ~ActivationMask;
            m_itemFlags |= InvertedActivation;
            activate();
            m_flags2_02_toggledOn = true;
            m_flags2_04_ready = false;
        }
    }


    void ItemController::setCurrentRoom(const loader::Room* newRoom)
    {
        if( newRoom == m_position.room )
            return;

        if( newRoom == nullptr )
        {
            BOOST_LOG_TRIVIAL(fatal) << "No room to switch to.";
            return;
        }
        BOOST_LOG_TRIVIAL(debug) << "Room switch of " << getId() << " to " << newRoom->node->getId();

        newRoom->node->addChild(shared_from_this());

        m_position.room = newRoom;
    }


    void ItemController::processAnimCommands(bool advanceFrame)
    {
        m_flags2_10 = false;

        if( advanceFrame )
            this->advanceFrame();

        bool newFrame = advanceFrame;

        if( handleTRTransitions() || m_recentAnimFrame != getCurrentFrame() )
        {
            m_recentAnimFrame = getCurrentFrame();
            newFrame = true;
        }

        const bool isAnimEnd = getCurrentFrame() > getLastFrame();

        const loader::Animation& animation = getLevel().m_animations[getAnimId()];
        if( animation.animCommandCount > 0 )
        {
            BOOST_ASSERT(animation.animCommandIndex < getLevel().m_animCommands.size());
            const auto* cmd = &getLevel().m_animCommands[animation.animCommandIndex];
            for( uint16_t i = 0; i < animation.animCommandCount; ++i )
            {
                BOOST_ASSERT(cmd < &getLevel().m_animCommands.back());
                const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
                ++cmd;
                switch( opcode )
                {
                    case AnimCommandOpcode::SetPosition:
                        if( isAnimEnd && newFrame )
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
                        if( isAnimEnd && newFrame )
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
                        if( newFrame && getCurrentFrame() == core::Frame(cmd[0]) )
                        {
                            playSoundEffect(cmd[1]);
                        }
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::PlayEffect:
                        if( getCurrentFrame() == core::Frame(cmd[0]) )
                        {
                            BOOST_LOG_TRIVIAL(debug) << "Anim effect: " << int(cmd[1]);
                            if( cmd[1] == 0 && newFrame )
                                addYRotation(180_deg);
                            else if( cmd[1] == 12 )
                                getLevel().m_lara->setHandStatus(0);
                            //! @todo Execute anim effect cmd[1]
                        }
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::Kill:
                        if( isAnimEnd && newFrame )
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

        if( isAnimEnd )
        {
            handleAnimationEnd();
        }

        if( m_falling )
        {
            m_horizontalSpeed.add(getAccelleration(), getCurrentDeltaTime());
            if( getFallSpeed() >= 128 )
                m_fallSpeed.add(1, getCurrentDeltaTime());
            else
                m_fallSpeed.add(6, getCurrentDeltaTime());
        }
        else
        {
            m_horizontalSpeed = calculateFloorSpeed();
        }

        move(
            getRotation().Y.sin() * m_horizontalSpeed.getScaled(getCurrentDeltaTime()),
            m_falling ? m_fallSpeed.getScaled(getCurrentDeltaTime()) : 0,
            getRotation().Y.cos() * m_horizontalSpeed.getScaled(getCurrentDeltaTime())
        );
    }


    void ItemController::activate()
    {
        if( !m_hasProcessAnimCommandsOverride )
        {
            m_flags2_02_toggledOn = false;
            m_flags2_04_ready = false;
            return;
        }

        if( m_isActive )
        BOOST_LOG_TRIVIAL(warning) << "Item controller " << getId() << " already active";
        else
        BOOST_LOG_TRIVIAL(trace) << "Activating item controller " << getId();

        m_isActive = true;
    }


    void ItemController::deactivate()
    {
        if( !m_isActive )
        BOOST_LOG_TRIVIAL(warning) << "Item controller " << getId() << " already inactive";
        else
        BOOST_LOG_TRIVIAL(trace) << "Deactivating item controller " << getId();

        m_isActive = false;
    }


    std::shared_ptr<audio::SourceHandle> ItemController::playSoundEffect(int id)
    {
        auto handle = getLevel().playSound(id, getPosition());
        if( handle != nullptr )
            m_sounds.emplace_back(handle);
        return handle;
    }


    bool ItemController::triggerKey()
    {
        if( getLevel().m_lara->getHandStatus() != 0 )
            return false;

        if( m_flags2_04_ready || !m_flags2_02_toggledOn )
            return false;

        m_flags2_02_toggledOn = false;
        m_flags2_04_ready = true;
        return true;
    }


    void ItemController::updateSounds()
    {
        m_sounds.erase(std::remove_if(m_sounds.begin(), m_sounds.end(), [](const std::weak_ptr<audio::SourceHandle>& h)
                                      {
                                          return h.expired();
                                      }), m_sounds.end());

        for( const std::weak_ptr<audio::SourceHandle>& handle : m_sounds )
        {
            std::shared_ptr<audio::SourceHandle> lockedHandle = handle.lock();
            lockedHandle->setPosition(getPosition().toRenderSystem());
        }
    }


    void ItemController_55_Switch::onInteract(LaraController& lara)
    {
        if( !getLevel().m_inputHandler->getInputState().action )
            return;

        if( lara.getHandStatus() != 0 )
            return;

        if( lara.isFalling() )
            return;

        if( m_flags2_04_ready || m_flags2_02_toggledOn )
            return;

        if( lara.getCurrentState() != loader::LaraStateId::Stop )
            return;

        static const InteractionLimits limits{
            gameplay::BoundingBox{{-200, 0, 312}, {200, 0, 512}},
            {-10_deg,-30_deg,-10_deg},
            {+10_deg,+30_deg,+10_deg}
        };

        if( !limits.canInteract(*this, lara) )
            return;

        lara.setYRotation(getRotation().Y);

        if( getCurrentState() == 1 )
        {
            BOOST_LOG_TRIVIAL(debug) << "Switch " << getId() << ": pull down";
            do
            {
                lara.setTargetState(loader::LaraStateId::SwitchDown);
                lara.processLaraAnimCommands(true);
            } while( lara.getCurrentAnimState() != loader::LaraStateId::SwitchDown );
            lara.setTargetState(loader::LaraStateId::Stop);
            setTargetState(0);
            lara.setHandStatus(1);
        }
        else
        {
            if( getCurrentState() != 0 )
                return;

            BOOST_LOG_TRIVIAL(debug) << "Switch " << getId() << ": pull up";
            do
            {
                lara.setTargetState(loader::LaraStateId::SwitchUp);
                lara.processLaraAnimCommands(true);
            } while( lara.getCurrentAnimState() != loader::LaraStateId::SwitchUp );
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
        if( angle.X < minAngle.X || angle.X > maxAngle.X
            || angle.Y < minAngle.Y || angle.Y > maxAngle.Y
            || angle.Z < minAngle.Z || angle.Z > maxAngle.Z )
        {
            return false;
        }

        glm::quat q;
        q *= glm::quat(item.getRotation().Y.toRad(), {0,1,0});
        q *= glm::quat(item.getRotation().X.toRad(), {-1,0,0});
        q *= glm::quat(item.getRotation().Z.toRad(), {0,0,-1});

        auto dist = lara.getPosition() - item.getPosition();
        glm::vec3 tdist = dist.toRenderSystem() * q;

        return distance.contains(tdist);
    }


    void ItemController_Door::onInteract(LaraController& /*lara*/)
    {
    }


    void ItemController_35_CollapsibleFloor::processAnimCommands(bool advanceFrame)
    {
        if( getCurrentState() == 0 ) // stationary
        {
            if( !util::fuzzyEqual(getPosition().Y - 512, getLevel().m_lara->getPosition().Y, 1.0f) )
            {
                m_flags2_02_toggledOn = false;
                m_flags2_04_ready = false;
                deactivate();
                return;
            }
            setTargetState(1);
        }
        else if( getCurrentState() == 1 ) // shaking
        {
            setTargetState(2);
        }
        else if( getCurrentState() == 2 && getTargetState() != 3 ) // falling, not going to settle
        {
            setFalling(true);
        }

        ItemController::processAnimCommands(advanceFrame);

        if( m_flags2_04_ready && !m_flags2_02_toggledOn )
        {
            deactivate();
            return;
        }

        auto room = getCurrentRoom();
        auto sector = getLevel().findFloorSectorWithClampedPosition(getPosition().toInexact(), &room);
        setCurrentRoom(room);

        HeightInfo h = HeightInfo::fromFloor(sector, getPosition().toInexact(), getLevel().m_cameraController);
        setFloorHeight(h.distance);
        if( getCurrentState() != 2 || getPosition().Y < h.distance )
            return;

        // settle
        setTargetState(3);
        setFallSpeed(core::makeInterpolatedValue(0.0f));
        auto pos = getPosition();
        pos.Y = getFloorHeight();
        setPosition(pos);
        setFalling(false);
    }


    void ItemController_Block::onInteract(LaraController& lara)
    {
        if( !getLevel().m_inputHandler->getInputState().action || (m_flags2_02_toggledOn && !m_flags2_04_ready) || isFalling() || !util::fuzzyEqual(lara.getPosition().Y, getPosition().Y, 1.0f) )
            return;

        static const InteractionLimits limits{
            gameplay::BoundingBox{{-300, 0, -692},{200, 0, -512}},
            {-10_deg,-30_deg,-10_deg},
            {+10_deg,+30_deg,+10_deg}
        };

        auto axis = core::axisFromAngle(lara.getRotation().Y, 45_deg);
        Expects(axis.is_initialized());

        if( lara.getCurrentAnimState() == loader::LaraStateId::Stop )
        {
            if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Null || lara.getHandStatus() != 0 )
                return;

            setYRotation(core::alignRotation(*axis));

            if( !limits.canInteract(*this, lara) )
                return;

            switch( *axis )
            {
                case core::Axis::PosZ:
                {
                    auto pos = lara.getPosition();
                    pos.Z = std::floor(pos.Z / loader::SectorSize) * loader::SectorSize + 924;
                    lara.setPosition(pos);
                    break;
                }
                case core::Axis::PosX:
                {
                    auto pos = lara.getPosition();
                    pos.X = std::floor(pos.X / loader::SectorSize) * loader::SectorSize + 924;
                    lara.setPosition(pos);
                    break;
                }
                case core::Axis::NegZ:
                {
                    auto pos = lara.getPosition();
                    pos.Z = std::floor(pos.Z / loader::SectorSize) * loader::SectorSize + 100;
                    lara.setPosition(pos);
                    break;
                }
                case core::Axis::NegX:
                {
                    auto pos = lara.getPosition();
                    pos.X = std::floor(pos.X / loader::SectorSize) * loader::SectorSize + 100;
                    lara.setPosition(pos);
                    break;
                }
                default:
                    break;
            }

            lara.setYRotation(getRotation().Y);
            lara.setTargetState(loader::LaraStateId::PushableGrab);
            lara.processLaraAnimCommands(true);
            if( lara.getCurrentAnimState() == loader::LaraStateId::PushableGrab )
                lara.setHandStatus(1);
            return;
        }

        if( lara.getCurrentAnimState() != loader::LaraStateId::PushableGrab || lara.getCurrentFrame() != 2091_frame || !limits.canInteract(*this, lara) )
            return;

        if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
        {
            if( !canPushBlock(loader::SectorSize, *axis) )
                return;

            setTargetState(2);
            lara.setTargetState(loader::LaraStateId::PushablePush);
        }
        else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward )
        {
            if( !canPullBlock(loader::SectorSize, *axis) )
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
        if( (m_itemFlags & Oneshot) != 0 )
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
        if( height > pos.position.Y )
        {
            setFalling(true);
        }
        else if( isFalling() )
        {
            pos.position.Y = height;
            setPosition(pos.position);
            setFalling(false);
            m_flags2_02_toggledOn = false;
            m_flags2_04_ready = true;
            //! @todo Shake camera
            playSoundEffect(70);
        }

        setCurrentRoom(pos.room);

        if( m_flags2_02_toggledOn || !m_flags2_04_ready )
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
        return sector->floorHeight == -127 || util::fuzzyEqual(gsl::narrow_cast<float>(sector->floorHeight * loader::QuarterSectorSize), getPosition().Y - height, 1.0f);
    }


    bool ItemController_Block::canPushBlock(int height, core::Axis axis) const
    {
        if( !isOnFloor(height) )
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
        if( tmp.checkStaticMeshCollisions(pos, 1000, getLevel()) )
            return false;

        auto targetSector = getLevel().findFloorSectorWithClampedPosition(pos.toInexact(), getCurrentRoom());
        if( !util::fuzzyEqual(gsl::narrow_cast<float>(targetSector->floorHeight * loader::QuarterSectorSize), pos.Y, 1.0f) )
            return false;

        pos.Y -= height;
        return pos.Y >= getLevel().findFloorSectorWithClampedPosition(pos.toInexact(), getCurrentRoom())->ceilingHeight * loader::QuarterSectorSize;
    }


    bool ItemController_Block::canPullBlock(int height, core::Axis axis) const
    {
        if( !isOnFloor(height) )
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
        auto sector = getLevel().findFloorSectorWithClampedPosition(pos.toInexact(), &room);

        CollisionInfo tmp;
        tmp.orientationAxis = axis;
        tmp.collisionRadius = 500;
        if( tmp.checkStaticMeshCollisions(pos, 1000, getLevel()) )
            return false;

        if( !util::fuzzyEqual(gsl::narrow_cast<float>(sector->floorHeight * loader::QuarterSectorSize), pos.Y, 1.0f) )
            return false;

        auto topPos = pos;
        topPos.Y -= height;
        auto topSector = getLevel().findFloorSectorWithClampedPosition(topPos.toInexact(), getCurrentRoom());
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

        sector = getLevel().findFloorSectorWithClampedPosition(laraPos.toInexact(), &room);
        if( !util::fuzzyEqual(gsl::narrow_cast<float>(sector->floorHeight * loader::QuarterSectorSize), pos.Y, 1.0f) )
            return false;

        laraPos.Y -= core::ScalpHeight;
        sector = getLevel().findFloorSectorWithClampedPosition(laraPos.toInexact(), &room);
        if( laraPos.Y < sector->ceilingHeight * loader::QuarterSectorSize )
            return false;

        laraPos = getLevel().m_lara->getPosition();
        switch( axis )
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
        getLevel().findFloorSectorWithClampedPosition(getPosition().toInexact(), &room);
        setCurrentRoom(room);

        if( m_flags2_02_toggledOn || !m_flags2_04_ready )
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


    void ItemController_SwingingBlade::animateImpl(bool)
    {
        if( updateTriggerTimeout() )
        {
            if( getCurrentState() == 0 )
                setTargetState(2);
        }
        else if( getCurrentState() == 2 )
        {
            setTargetState(0);
        }
    }


    void ItemController_SwingingBlade::processAnimCommands(bool advanceFrame)
    {
        auto room = getCurrentRoom();
        auto sector = getLevel().findFloorSectorWithClampedPosition(getPosition().toInexact(), &room);
        setCurrentRoom(room);
        setFloorHeight(HeightInfo::fromFloor(sector, getPosition().toInexact(), getLevel().m_cameraController).distance);

        ItemController::processAnimCommands(advanceFrame);
    }


    void ItemController::update(const std::chrono::microseconds& deltaTime)
    {
        addTime(deltaTime);

        m_currentDeltaTime = deltaTime;

        if( m_currentDeltaTime <= std::chrono::microseconds::zero() )
            return;

        bool isNewFrame = m_recentAnimFrame != getCurrentFrame();
        m_subFrameTime += deltaTime;

        if( m_subFrameTime >= core::FrameTime )
        {
            isNewFrame = true;
            m_subFrameTime -= m_subFrameTime / core::FrameTime * core::FrameTime;
        }

        animateImpl(isNewFrame);

        if( m_isActive && m_hasProcessAnimCommandsOverride )
            processAnimCommands();

        applyTransform();

        updatePose();
    }
}
