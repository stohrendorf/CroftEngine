#include "larastatehandler.h"

#include "defaultanimdispatcher.h"
#include "trcamerascenenodeanimator.h"

void LaraStateHandler::setTargetState(loader::LaraState st)
{
    m_dispatcher->setTargetState(static_cast<uint16_t>(st));
}

loader::LaraState LaraStateHandler::getTargetState() const
{
    return static_cast<LaraState>(m_dispatcher->getTargetState());
}

void LaraStateHandler::playAnimation(loader::AnimationId anim)
{
    m_dispatcher->playLocalAnimation(static_cast<uint16_t>(anim));
}

void LaraStateHandler::onInput0WalkForward()
{
    if(m_health <= 0)
    {
        setTargetState(LaraState::Stop);
        return;
    }

    if(m_xMovement == AxisMovement::Left)
        m_yRotationSpeed = std::max(-728, m_yRotationSpeed - 409);
    else if(m_xMovement == AxisMovement::Right)
        m_yRotationSpeed = std::min(728, m_yRotationSpeed + 409);
    if(m_zMovement == AxisMovement::Forward)
    {
        if(m_moveSlow)
            setTargetState(LaraState::WalkForward);
        else
            setTargetState(LaraState::RunForward);
    }
    else
    {
        setTargetState(LaraState::Stop);
    }
}

void LaraStateHandler::onInput1RunForward()
{
    if(m_health <= 0)
    {
        setTargetState(LaraState::Death);
        return;
    }

    if(m_roll)
    {
        //! @todo Play animation from frame 3857
        playAnimation(loader::AnimationId::ROLL_BEGIN);
        setTargetState(LaraState::Stop);
        return;
    }
    if(m_xMovement == AxisMovement::Left)
    {
        m_yRotationSpeed = std::max(-1456, m_yRotationSpeed - 409);
        m_rotation.Z = std::max(-2002, m_rotation.Z - 273);
    }
    else if(m_xMovement == AxisMovement::Right)
    {
        m_yRotationSpeed = std::min(1456, m_yRotationSpeed + 409);
        m_rotation.Z = std::min(2002, m_rotation.Z + 273);
    }
    if(m_jump && !m_falling)
    {
        setTargetState(LaraState::JumpForward);
        return;
    }
    if(m_zMovement != AxisMovement::Forward)
    {
        setTargetState(LaraState::Stop);
        return;
    }
    if(m_moveSlow)
        setTargetState(LaraState::WalkForward);
    else
        setTargetState(LaraState::RunForward);
}

void LaraStateHandler::onInput2Stop()
{
    if(m_health <= 0)
    {
        setTargetState(LaraState::Death);
        return;
    }

    if(m_roll)
    {
        playAnimation(loader::AnimationId::ROLL_BEGIN);
        setTargetState(LaraState::Stop);
        return;
    }

    setTargetState(LaraState::Stop);
    if(m_stepMovement == AxisMovement::Left)
    {
        setTargetState(LaraState::StepLeft);
    }
    else if(m_stepMovement == AxisMovement::Right)
    {
        setTargetState(LaraState::StepRight);
    }
    if(m_xMovement == AxisMovement::Left)
    {
        setTargetState(LaraState::TurnLeftSlow);
    }
    else if(m_xMovement == AxisMovement::Right)
    {
        setTargetState(LaraState::TurnRightSlow);
    }
    if(m_jump)
    {
        setTargetState(LaraState::JumpPrepare);
    }
    if(m_zMovement == AxisMovement::Forward)
    {
        if(m_moveSlow)
            onInput0WalkForward();
        else
            onInput1RunForward();
    }
    else if(m_zMovement == AxisMovement::Backward)
    {
        if(m_moveSlow)
            onInput16WalkBackward();
        else
            setTargetState(LaraState::RunBack);
    }
}

void LaraStateHandler::onInput3JumpForward()
{
    if(getTargetState() == LaraState::SwandiveBegin || getTargetState() == LaraState::Reach)
        setTargetState(LaraState::JumpForward);

    if(getTargetState() != LaraState::Death && getTargetState() != LaraState::Stop)
    {
        //! @todo Not only m_action, but also free hands!
        if(m_action)
            setTargetState(LaraState::Reach);
        //! @todo Not only m_action, but also free hands!
        if(m_moveSlow)
            setTargetState(LaraState::SwandiveBegin);
        if(m_fallSpeed > 131)
            setTargetState(LaraState::FreeFall);
    }

    if(m_xMovement == AxisMovement::Left)
    {
        m_yRotationSpeed = std::max(-546, m_yRotationSpeed - 409);
    }
    else if(m_xMovement == AxisMovement::Right)
    {
        m_yRotationSpeed = std::min(546, m_yRotationSpeed + 409);
    }
}

void LaraStateHandler::onInput5RunBackward()
{
    setTargetState(LaraState::Stop);

    if(m_xMovement == AxisMovement::Left)
        m_yRotationSpeed = std::max(-1092, m_yRotationSpeed - 409);
    else if(m_xMovement == AxisMovement::Right)
        m_yRotationSpeed = std::min(1092, m_yRotationSpeed + 409);
}

void LaraStateHandler::onInput6TurnRightSlow()
{
    if(m_health <= 0)
    {
        setTargetState(LaraState::Stop);
        return;
    }

    m_yRotationSpeed += 409;

    //! @todo Hand status
    if(false /* hands are in combat? */)
    {
        setTargetState(LaraState::TurnFast);
        return;
    }

    if(m_yRotationSpeed > 728)
    {
        if(m_moveSlow)
            m_yRotationSpeed = 728;
        else
            setTargetState(LaraState::TurnFast);
    }

    if(m_zMovement != AxisMovement::Forward)
    {
        if(m_xMovement != AxisMovement::Right)
            setTargetState(LaraState::Stop);
        return;
    }

    if(m_moveSlow)
        setTargetState(LaraState::WalkForward);
    else
        setTargetState(LaraState::RunForward);
}

void LaraStateHandler::onInput7TurnLeftSlow()
{
    if(m_health <= 0)
    {
        setTargetState(LaraState::Stop);
        return;
    }

    m_yRotationSpeed -= 409;

    //! @todo Hand status
    if(false /* hands are in combat? */)
    {
        setTargetState(LaraState::TurnFast);
        return;
    }

    if(m_yRotationSpeed < -728)
    {
        if(m_moveSlow)
            m_yRotationSpeed = -728;
        else
            setTargetState(LaraState::TurnFast);
    }

    if(m_zMovement != AxisMovement::Forward)
    {
        if(m_xMovement != AxisMovement::Left)
            setTargetState(LaraState::Stop);
        return;
    }

    if(m_moveSlow)
        setTargetState(LaraState::WalkForward);
    else
        setTargetState(LaraState::RunForward);
}

void LaraStateHandler::onInput16WalkBackward()
{
    if(m_health <= 0)
    {
        setTargetState(LaraState::Stop);
        return;
    }

    if(m_zMovement == AxisMovement::Backward && m_moveSlow)
        setTargetState(LaraState::WalkBackward);
    else
        setTargetState(LaraState::Stop);

    if(m_xMovement == AxisMovement::Left)
        m_yRotationSpeed = std::max(-728, m_yRotationSpeed - 409);
    else if(m_xMovement == AxisMovement::Right)
        m_yRotationSpeed = std::min(728, m_yRotationSpeed + 409);
}

void LaraStateHandler::onInput20TurnFast()
{
    if(m_health <= 0)
    {
        setTargetState(LaraState::Stop);
        return;
    }

    if(m_yRotationSpeed >= 0)
    {
        m_yRotationSpeed = 1456;
        if(m_xMovement == AxisMovement::Right)
            return;
    }
    else
    {
        m_yRotationSpeed = -1456;
        if(m_xMovement == AxisMovement::Left)
            return;
    }
    setTargetState(LaraState::Stop);
}

void LaraStateHandler::onInput25JumpBackward()
{
    //! @todo Set local camera Y rotation to 24570 AU
    if(m_fallSpeed > 131)
        setTargetState(LaraState::FreeFall);
}

void LaraStateHandler::animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs)
{
    BOOST_ASSERT(m_lara == node);

    const auto currentFrame = 30 * timeMs / 1000;
    if(currentFrame == m_lastActiveFrame)
        return;
    m_lastActiveFrame = currentFrame;

    //! @todo Only when on solid ground
    m_air = 1800;

    {
        // "slowly" revert rotations to zero

        if(m_rotation.Z < -182)
            m_rotation.Z += 182;
        else if(m_rotation.Z > 182)
            m_rotation.Z -= 182;
        else
            m_rotation.Z = 0;

        if(m_yRotationSpeed < -364)
            m_yRotationSpeed += 364;
        else if(m_yRotationSpeed > 364)
            m_yRotationSpeed -= 364;
        else
            m_yRotationSpeed = 0;
        m_rotation.Y += m_yRotationSpeed;
    }


    static std::array<InputHandler, 56> inputHandlers{ {
            &LaraStateHandler::onInput0WalkForward,
            &LaraStateHandler::onInput1RunForward,
        &LaraStateHandler::onInput2Stop,
        &LaraStateHandler::onInput3JumpForward,
        nullptr,
        &LaraStateHandler::onInput5RunBackward,
        &LaraStateHandler::onInput6TurnRightSlow,
        &LaraStateHandler::onInput7TurnLeftSlow,
        nullptr,
        nullptr,
        // 10
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        &LaraStateHandler::onInput16WalkBackward,
        nullptr,
        nullptr,
        nullptr,
        &LaraStateHandler::onInput20TurnFast,
        nullptr,nullptr,nullptr,nullptr,
        &LaraStateHandler::onInput25JumpBackward,
        nullptr,nullptr,nullptr,nullptr,
        // 30
        nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
        // 40
        nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
        // 50
        nullptr,nullptr,nullptr,nullptr,nullptr,nullptr
        } };

    const auto currentState = m_dispatcher->getCurrentState();
    if(currentState >= inputHandlers.size())
    {
        BOOST_LOG_TRIVIAL(error) << "Unexpected state " << currentState;
        return;
    }

    if(!inputHandlers[currentState])
        BOOST_LOG_TRIVIAL(warning) << "No input handler for state " << currentState;
    else
        (this->*inputHandlers[currentState])();


    if(m_falling)
    {
        m_horizontalSpeed += m_dispatcher->getAccelleration();
        if(m_fallSpeed >= 128)
            m_fallSpeed += 1;
        else
            m_fallSpeed += 6;
        m_fallSpeed = 0; //!< @todo We're never falling (for now).
    }
    else
    {
        m_horizontalSpeed = m_dispatcher->calculateFloorSpeed();
    }

    m_movementAngle = m_rotation.Y;

    // behaviour handling depends on the current state *after* handling the input
    switch(static_cast<LaraState>(m_dispatcher->getCurrentState()))
    {
        case LaraState::WalkBackward:
        case LaraState::RunBack:
        case LaraState::JumpBack:
            m_movementAngle += 32768;
            break;
        default:
            break;
    }

    processAnimCommands();
}

void LaraStateHandler::processAnimCommands()
{
    const loader::Animation& animation = m_level->m_animations[m_dispatcher->getCurrentAnimationId()];
    if(animation.animCommandCount > 0)
    {
        BOOST_ASSERT(animation.animCommandIndex < m_level->m_animCommands.size());
        const auto* cmd = &m_level->m_animCommands[animation.animCommandIndex];
        for(uint16_t i = 0; i < animation.animCommandCount; ++i)
        {
            BOOST_ASSERT(cmd < &m_level->m_animCommands.back());
            const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
            ++cmd;
            switch(opcode)
            {
                case AnimCommandOpcode::SetPosition:
                {
                    auto dx = cmd[0];
                    auto dy = cmd[1];
                    auto dz = cmd[2];
                    cmd += 3;
                    break;
                }
                case AnimCommandOpcode::SetVelocity:
                    m_fallSpeed = m_fallSpeedOverride == 0 ? cmd[0] : m_fallSpeedOverride;
                    m_fallSpeedOverride = 0;
                    m_horizontalSpeed = cmd[1];
                    m_falling = true;
                    cmd += 2;
                    break;
                case AnimCommandOpcode::EmptyHands:
                    //! @todo Set hand status to "free"
                    break;
                case AnimCommandOpcode::PlaySound:
                    if(m_dispatcher->getCurrentFrame() == cmd[0])
                    {
                        //! @todo playsound(cmd[1])
                    }
                    cmd += 2;
                    break;
                case AnimCommandOpcode::PlayEffect:
                    if(m_dispatcher->getCurrentFrame() == cmd[0])
                    {
                        //! @todo Execute anim effect cmd[1]
                    }
                    cmd += 2;
                    break;
                default:
                    break;
            }
        }
    }

    auto pos = m_lara->getPosition();
    pos.X += std::sin(util::auToRad(m_movementAngle)) * m_horizontalSpeed;
    pos.Z += std::cos(util::auToRad(m_movementAngle)) * m_horizontalSpeed;
    m_lara->setPosition(pos);

    {
        //! @todo This is horribly inefficient code, but it properly converts ZXY angles to XYZ angles.
        irr::core::quaternion q;
        q.makeIdentity();
        q *= irr::core::quaternion().fromAngleAxis(util::auToRad(m_rotation.Y), { 0,1,0 });
        q *= irr::core::quaternion().fromAngleAxis(util::auToRad(m_rotation.X), { 1,0,0 });
        q *= irr::core::quaternion().fromAngleAxis(util::auToRad(m_rotation.Z), { 0,0,-1 });

        irr::core::vector3df euler;
        q.toEuler(euler);
        m_lara->setRotation(euler * 180 / irr::core::PI);
    }

    m_lara->updateAbsolutePosition();
}

struct HeightInfo
{
    enum class SlantClass
    {
        None,
        Max512,
        Steep
    };

    const uint16_t* triggerOrKill = nullptr;
    int height = 0;
    SlantClass slantClass = SlantClass::None;

    void initFloor(TRCameraSceneNodeAnimator* camera, const irr::core::vector3df& pos, bool skipSteepSlants)
    {
        const loader::Sector* roomSector = camera->getCurrentRoom()->getSectorByAbsolutePosition(pos);
        BOOST_ASSERT(roomSector != nullptr);
        for(auto room = camera->getCurrentRoom(); roomSector->roomBelow != 0xff; roomSector = room->getSectorByAbsolutePosition(pos))
        {
            BOOST_ASSERT(roomSector->roomAbove < camera->getLevel()->m_rooms.size());
            room = &camera->getLevel()->m_rooms[roomSector->roomBelow];
        }

        height = roomSector->floorHeight * loader::QuarterSectorSize;

        if(roomSector->floorDataIndex == 0)
        {
            return;
        }

        const uint16_t* floorData = &camera->getLevel()->m_floorData[roomSector->floorDataIndex];
        while(true)
        {
            const bool isLast = loader::isLastFloorataEntry(*floorData);
            const auto currentFd = *floorData;
            ++floorData;
            switch(loader::extractFDFunction(currentFd))
            {
                case loader::FDFunction::FloorSlant:
                {
                    const int8_t xSlant = static_cast<int8_t>(*floorData & 0xff);
                    const auto absX = std::abs(xSlant);
                    const int8_t zSlant = static_cast<int8_t>((*floorData >> 8) & 0xff);
                    const auto absZ = std::abs(zSlant);
                    if(!skipSteepSlants || (absX <= 2 && absZ <= 2))
                    {
                        if(absX <= 2 && absZ <= 2)
                            slantClass = HeightInfo::SlantClass::Max512;
                        else
                            slantClass = HeightInfo::SlantClass::Steep;

                        const irr::f32 localX = std::fmod(pos.X, loader::SectorSize);
                        const irr::f32 localZ = std::fmod(pos.Z, loader::SectorSize);

                        if(zSlant > 0) // lower edge at -Z
                        {
                            auto dist = (loader::SectorSize - localZ) / loader::SectorSize;
                            height -= static_cast<int>(dist * zSlant * loader::QuarterSectorSize);
                        }
                        else if(zSlant < 0)  // lower edge at +Z
                        {
                            auto dist = localZ / loader::SectorSize;
                            height += static_cast<int>(dist * zSlant * loader::QuarterSectorSize);
                        }

                        if(xSlant > 0) // lower edge at -X
                        {
                            auto dist = (loader::SectorSize - localX) / loader::SectorSize;
                            height -= static_cast<int>(dist * xSlant * loader::QuarterSectorSize);
                        }
                        else if(xSlant < 0) // lower edge at +X
                        {
                            auto dist = localX / loader::SectorSize;
                            height += static_cast<int>(dist * xSlant * loader::QuarterSectorSize);
                        }
                    }
                }
                // Fall-through
                case loader::FDFunction::CeilingSlant:
                case loader::FDFunction::PortalSector:
                    ++floorData;
                    break;
                case loader::FDFunction::Death:
                    triggerOrKill = floorData - 1;
                    break;
                case loader::FDFunction::Trigger:
                    if(!triggerOrKill)
                        triggerOrKill = floorData - 1;
                    ++floorData;
                    while(true)
                    {
                        const bool isLast = loader::isLastFloorataEntry(*floorData);

                        const auto func = loader::extractTriggerFunction(*floorData);
                        const auto param = loader::extractTriggerFunctionParam(*floorData);
                        ++floorData;

                        if(func != loader::TriggerFunction::Object)
                        {
                            if(func == loader::TriggerFunction::CameraTarget)
                            {
                                ++floorData;
                            }
                        }
                        else
                        {
                            BOOST_ASSERT(func == loader::TriggerFunction::Object);
                            //! @todo Query height patch from object @c param, e.g. trapdoors or falling floor.
                        }

                        if(isLast)
                            break;
                    }
                default:
                    break;
            }
            if(isLast)
                break;
        }
    }

    void initCeiling(TRCameraSceneNodeAnimator* camera, const irr::core::vector3df& pos, bool skipSteepSlants)
    {
        const loader::Sector* roomSector = camera->getCurrentRoom()->getSectorByAbsolutePosition(pos);
        BOOST_ASSERT(roomSector != nullptr);
        for(auto room = camera->getCurrentRoom(); roomSector->roomAbove != 0xff; roomSector = room->getSectorByAbsolutePosition(pos))
        {
            BOOST_ASSERT(roomSector->roomAbove < camera->getLevel()->m_rooms.size());
            room = &camera->getLevel()->m_rooms[roomSector->roomAbove];
        }

        height = roomSector->ceilingHeight * loader::QuarterSectorSize;

        if(roomSector->floorDataIndex == 0)
        {
            return;
        }

        const uint16_t* floorData = &camera->getLevel()->m_floorData[roomSector->floorDataIndex];
        while(true)
        {
            const bool isLast = loader::isLastFloorataEntry(*floorData);
            const auto currentFd = *floorData;
            ++floorData;
            switch(loader::extractFDFunction(currentFd))
            {
                case loader::FDFunction::CeilingSlant:
                {
                    const int8_t xSlant = static_cast<int8_t>(*floorData & 0xff);
                    const auto absX = std::abs(xSlant);
                    const int8_t zSlant = static_cast<int8_t>((*floorData >> 8) & 0xff);
                    const auto absZ = std::abs(zSlant);
                    if(!skipSteepSlants || (absX <= 2 && absZ <= 2))
                    {
                        if(absX <= 2 && absZ <= 2)
                            slantClass = HeightInfo::SlantClass::Max512;
                        else
                            slantClass = HeightInfo::SlantClass::Steep;

                        const irr::f32 localX = std::fmod(pos.X, loader::SectorSize);
                        const irr::f32 localZ = std::fmod(pos.Z, loader::SectorSize);

                        if(zSlant > 0) // lower edge at -Z
                        {
                            auto dist = (loader::SectorSize - localZ) / loader::SectorSize;
                            height -= static_cast<int>(dist * zSlant * loader::QuarterSectorSize);
                        }
                        else if(zSlant < 0)  // lower edge at +Z
                        {
                            auto dist = localZ / loader::SectorSize;
                            height += static_cast<int>(dist * zSlant * loader::QuarterSectorSize);
                        }

                        if(xSlant > 0) // lower edge at -X
                        {
                            auto dist = (loader::SectorSize - localX) / loader::SectorSize;
                            height -= static_cast<int>(dist * xSlant * loader::QuarterSectorSize);
                        }
                        else if(xSlant < 0) // lower edge at +X
                        {
                            auto dist = localX / loader::SectorSize;
                            height += static_cast<int>(dist * xSlant * loader::QuarterSectorSize);
                        }
                    }
                }
                // Fall-through
                case loader::FDFunction::FloorSlant:
                case loader::FDFunction::PortalSector:
                    ++floorData;
                    break;
                case loader::FDFunction::Death:
                    triggerOrKill = floorData - 1;
                    break;
                case loader::FDFunction::Trigger:
                    if(!triggerOrKill)
                        triggerOrKill = floorData - 1;
                    ++floorData;
                    while(true)
                    {
                        const bool isLast = loader::isLastFloorataEntry(*floorData);

                        const auto func = loader::extractTriggerFunction(*floorData);
                        const auto param = loader::extractTriggerFunctionParam(*floorData);
                        ++floorData;

                        if(func != loader::TriggerFunction::Object)
                        {
                            if(func == loader::TriggerFunction::CameraTarget)
                            {
                                ++floorData;
                            }
                        }
                        else
                        {
                            BOOST_ASSERT(func == loader::TriggerFunction::Object);
                            //! @todo Query height patch from object @c param.
                        }

                        if(isLast)
                            break;
                    }
                default:
                    break;
            }
            if(isLast)
                break;
        }
    }
};
