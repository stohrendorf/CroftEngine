#include "larastatehandler.h"

#include "defaultanimdispatcher.h"
#include "trcamerascenenodeanimator.h"

constexpr int FrobbelFlag01 = 0x01;
constexpr int FrobbelFlag02 = 0x02;
constexpr int FrobbelFlag04 = 0x04;
constexpr int FrobbelFlag08 = 0x08;
constexpr int FrobbelFlag10 = 0x10;
constexpr int FrobbelFlag20 = 0x20;
constexpr int FrobbelFlag40 = 0x40;
constexpr int FrobbelFlag80 = 0x80;

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

    static HeightInfo createFloorInfo(const loader::Sector* roomSector, TRCameraSceneNodeAnimator* camera, const irr::core::vector3df& pos, bool skipSteepSlants)
    {
        HeightInfo hi;
        hi.initFloor(roomSector, camera, pos, skipSteepSlants);
        return hi;
    }

    static HeightInfo createCeilingInfo(const loader::Sector* roomSector, TRCameraSceneNodeAnimator* camera, const irr::core::vector3df& pos, bool skipSteepSlants)
    {
        HeightInfo hi;
        hi.initCeiling(roomSector, camera, pos, skipSteepSlants);
        return hi;
    }

    void initFloor(const loader::Sector* roomSector, TRCameraSceneNodeAnimator* camera, const irr::core::vector3df& pos, bool skipSteepSlants)
    {
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
                        else if(zSlant < 0) // lower edge at +Z
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

    void initCeiling(const loader::Sector* roomSector, TRCameraSceneNodeAnimator* camera, const irr::core::vector3df& pos, bool skipSteepSlants)
    {
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
                        else if(zSlant < 0) // lower edge at +Z
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

struct FullHeightInfo
{
    HeightInfo floor;
    HeightInfo ceiling;

    void init(const loader::Sector* sector, TRCameraSceneNodeAnimator* camera, const irr::core::vector3df& sectorPos, int frobbelFlags, int height, irr::f32 laraHeight, bool skipSteepSlants)
    {
        floor.initFloor(sector, camera, sectorPos, skipSteepSlants);
        ceiling.initCeiling(sector, camera, sectorPos, skipSteepSlants);

        if(floor.height != -loader::HeightLimit)
            floor.height -= laraHeight;
        if(ceiling.height != -loader::HeightLimit)
            ceiling.height -= laraHeight - height;

        if((frobbelFlags & FrobbelFlag01) != 0 && floor.slantClass == HeightInfo::SlantClass::Steep && floor.height < 0)
        {
            floor.height = -32767; //!< @todo MAGICK -32767
        }
        else if(((frobbelFlags & FrobbelFlag02) != 0 && floor.slantClass == HeightInfo::SlantClass::Steep && floor.height > 0)
                || ((frobbelFlags & FrobbelFlag04) != 0 && floor.height > 0 && floor.triggerOrKill && loader::extractFDFunction(*floor.triggerOrKill) == loader::FDFunction::Death))
        {
            floor.height = 2 * loader::QuarterSectorSize;
        }
    }
};

enum class Axis
{
    PosZ,
    PosX,
    NegZ,
    NegX
};

struct LaraState
{
    static constexpr int AxisColl00 = 0x00;
    static constexpr int AxisColl01 = 0x01;
    static constexpr int AxisColl02 = 0x02;
    static constexpr int AxisColl04 = 0x04;
    static constexpr int AxisColl08 = 0x08;
    static constexpr int AxisColl10 = 0x10;
    static constexpr int AxisColl20 = 0x20;
    static constexpr int AxisColl40 = 0x40;
    static constexpr int AxisColl80 = 0x80;

    int axisCollisions;
    irr::core::vector3df collisionFeedback;
    Axis orientationAxis;
    irr::s16 yAngle; // external
    int collisionRadius; // external
    int frobbelFlags; // external
    irr::core::vector3df position; // external
    int fruityFloorLimitMax; // external
    int fruityFloorLimitMin; // external
    int fruityCeilingLimit; // external

    FullHeightInfo currentHI;
    FullHeightInfo frontHI;
    FullHeightInfo frontLeftHI;
    FullHeightInfo frontRightHI;

    int8_t floorSlantX;
    int8_t floorSlantZ;

    static int fruityFeedback(int a, int b)
    {
        const auto sectorA = a / loader::SectorSize;
        const auto sectorB = b / loader::SectorSize;
        if(sectorA == sectorB)
            return 0;

        const auto localA = (a % loader::SectorSize) + 1;
        if(sectorB <= sectorA)
            return -localA;
        else
            return loader::SectorSize - localA;
    }

    void initHeightInfo(LaraStateHandler* lara, const loader::Level& level, int height, bool skipSteepSlants)
    {
        axisCollisions = AxisColl00;
        collisionFeedback = { 0,0,0 };
        orientationAxis = static_cast<Axis>((yAngle + util::degToAu(45)) / util::degToAu(90));

        const loader::Room* room = level.m_camera->getCurrentRoom();
        const irr::core::vector3df deltaHeight(0, height + 160, 0); //!< @todo MAGICK 160
        auto currentSector = level.findSectorForPosition(lara->getLara()->getPosition() - deltaHeight, room);
        currentHI.init(level.m_camera->getCurrentRoom()->getSectorByAbsolutePosition(lara->getLara()->getPosition()),
                       level.m_camera, lara->getLara()->getPosition() - deltaHeight,
                       0,
                       height,
                       lara->getLara()->getPosition().Y,
                       skipSteepSlants);
        if(currentHI.floor.height != -loader::HeightLimit)
            currentHI.floor.height -= lara->getLara()->getPosition().Y;
        if(currentHI.ceiling.height != -loader::HeightLimit)
            currentHI.ceiling.height -= lara->getLara()->getPosition().Y - height;
        std::tie(floorSlantX, floorSlantZ) = level.getFloorSlantInfo(currentSector, lara->getLara()->getPosition());

        int frontX = 0, frontZ = 0;
        int frontLeftX = 0, frontLeftZ = 0;
        int frontRightX = 0, frontRightZ = 0;

        switch(orientationAxis)
        {
            case Axis::PosZ:
                frontX = std::sin(util::auToRad(yAngle)) * collisionRadius;
                frontZ = collisionRadius;
                frontLeftZ = collisionRadius;
                frontLeftX = -collisionRadius;
                frontRightX = collisionRadius;
                frontRightZ = collisionRadius;
                break;
            case Axis::PosX:
                frontX = collisionRadius;
                frontZ = std::cos(util::auToRad(yAngle)) * collisionRadius;
                frontLeftX = collisionRadius;
                frontLeftZ = collisionRadius;
                frontRightX = collisionRadius;
                frontRightZ = -collisionRadius;
                break;
            case Axis::NegZ:
                frontX = std::sin(util::auToRad(yAngle)) * collisionRadius;
                frontZ = -collisionRadius;
                frontLeftX = collisionRadius;
                frontLeftZ = -collisionRadius;
                frontRightX = -collisionRadius;
                frontRightZ = -collisionRadius;
                break;
            case Axis::NegX:
                frontX = -collisionRadius;
                frontZ = std::cos(util::auToRad(yAngle)) * collisionRadius;
                frontLeftX = -collisionRadius;
                frontLeftZ = -collisionRadius;
                frontRightX = -collisionRadius;
                frontRightZ = collisionRadius;
                break;
        }

        // Front
        auto checkPos = lara->getLara()->getPosition() + irr::core::vector3df(frontX, 0, frontZ);
        auto sector = level.findSectorForPosition(checkPos, level.m_camera->getCurrentRoom());
        frontHI.init(sector,
                     level.m_camera, lara->getLara()->getPosition() - deltaHeight,
                     frobbelFlags,
                     height,
                     lara->getLara()->getPosition().Y,
                     skipSteepSlants);

        // Front left
        checkPos = lara->getLara()->getPosition() + irr::core::vector3df(frontLeftX, 0, frontLeftZ);
        sector = level.findSectorForPosition(checkPos, level.m_camera->getCurrentRoom());
        frontLeftHI.init(sector,
                         level.m_camera, lara->getLara()->getPosition() - deltaHeight,
                         frobbelFlags,
                         height,
                         lara->getLara()->getPosition().Y,
                         skipSteepSlants);


        // Front right
        checkPos = lara->getLara()->getPosition() + irr::core::vector3df(frontRightX, 0, frontRightZ);
        sector = level.findSectorForPosition(checkPos, level.m_camera->getCurrentRoom());
        frontRightHI.init(sector,
                          level.m_camera, lara->getLara()->getPosition() - deltaHeight,
                          frobbelFlags,
                          height,
                          lara->getLara()->getPosition().Y,
                          skipSteepSlants);

        //! @todo check static mesh collisions here

        if(currentHI.floor.height == -loader::HeightLimit)
        {
            collisionFeedback = position - lara->getLara()->getPosition();
            axisCollisions = AxisColl01;
            return;
        }

        if(currentHI.floor.height <= currentHI.ceiling.height)
        {
            axisCollisions = AxisColl20;
            collisionFeedback = position - lara->getLara()->getPosition();
            return;
        }

        if(currentHI.ceiling.height >= 0)
        {
            axisCollisions = AxisColl08;
            collisionFeedback.Y = currentHI.ceiling.height;
        }

        if(frontHI.floor.height > fruityFloorLimitMax || frontHI.floor.height < fruityFloorLimitMin || frontHI.ceiling.height > fruityCeilingLimit)
        {
            axisCollisions = AxisColl01;
            switch(orientationAxis)
            {
                case Axis::PosZ:
                case Axis::NegZ:
                    collisionFeedback.X = position.X - lara->getLara()->getPosition().X;
                    collisionFeedback.Z = fruityFeedback(frontZ + lara->getLara()->getPosition().Z, lara->getLara()->getPosition().Z);
                    break;
                case Axis::PosX:
                case Axis::NegX:
                    collisionFeedback.X = fruityFeedback(frontX + lara->getLara()->getPosition().X, lara->getLara()->getPosition().X);
                    collisionFeedback.Z = position.Z - lara->getLara()->getPosition().Z;
                    break;
            }
            return;
        }

        if(frontHI.ceiling.height >= fruityCeilingLimit)
        {
            axisCollisions = AxisColl10;
            collisionFeedback = position - lara->getLara()->getPosition();
            return;
        }

        if(frontLeftHI.floor.height > fruityFloorLimitMax || frontLeftHI.floor.height < fruityFloorLimitMin)
        {
            axisCollisions = AxisColl02;
            switch(orientationAxis)
            {
                case Axis::PosZ:
                case Axis::NegZ:
                    collisionFeedback.X = fruityFeedback(frontLeftX + lara->getLara()->getPosition().X, frontX + lara->getLara()->getPosition().X);
                    break;
                case Axis::PosX:
                case Axis::NegX:
                    collisionFeedback.Z = fruityFeedback(frontLeftZ + lara->getLara()->getPosition().Z, frontZ + lara->getLara()->getPosition().Z);
                    break;
            }
            return;
        }

        if(frontRightHI.floor.height > fruityFloorLimitMax || frontRightHI.floor.height < fruityFloorLimitMin)
        {
            axisCollisions = AxisColl04;
            switch(orientationAxis)
            {
                case Axis::PosZ:
                case Axis::NegZ:
                    collisionFeedback.X = fruityFeedback(frontRightX + lara->getLara()->getPosition().X, frontX + lara->getLara()->getPosition().X);
                    break;
                case Axis::PosX:
                case Axis::NegX:
                    collisionFeedback.Z = fruityFeedback(frontRightZ + lara->getLara()->getPosition().Z, frontZ + lara->getLara()->getPosition().Z);
                    break;
            }
        }
    }
};

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
    if( m_health <= 0 )
    {
        setTargetState(LaraState::Stop);
        return;
    }

    if( m_xMovement == AxisMovement::Left )
        m_yRotationSpeed = std::max(-728, m_yRotationSpeed - 409);
    else if( m_xMovement == AxisMovement::Right )
        m_yRotationSpeed = std::min(728, m_yRotationSpeed + 409);
    if( m_zMovement == AxisMovement::Forward )
    {
        if( m_moveSlow )
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
    if( m_health <= 0 )
    {
        setTargetState(LaraState::Death);
        return;
    }

    if( m_roll )
    {
        //! @todo Play animation from frame 3857
        playAnimation(loader::AnimationId::ROLL_BEGIN);
        setTargetState(LaraState::Stop);
        return;
    }
    if( m_xMovement == AxisMovement::Left )
    {
        m_yRotationSpeed = std::max(-1456, m_yRotationSpeed - 409);
        m_rotation.Z = std::max(-2002, m_rotation.Z - 273);
    }
    else if( m_xMovement == AxisMovement::Right )
    {
        m_yRotationSpeed = std::min(1456, m_yRotationSpeed + 409);
        m_rotation.Z = std::min(2002, m_rotation.Z + 273);
    }
    if( m_jump && !m_falling )
    {
        setTargetState(LaraState::JumpForward);
        return;
    }
    if( m_zMovement != AxisMovement::Forward )
    {
        setTargetState(LaraState::Stop);
        return;
    }
    if( m_moveSlow )
        setTargetState(LaraState::WalkForward);
    else
        setTargetState(LaraState::RunForward);
}

void LaraStateHandler::onInput2Stop()
{
    if( m_health <= 0 )
    {
        setTargetState(LaraState::Death);
        return;
    }

    if( m_roll )
    {
        playAnimation(loader::AnimationId::ROLL_BEGIN);
        setTargetState(LaraState::Stop);
        return;
    }

    setTargetState(LaraState::Stop);
    if( m_stepMovement == AxisMovement::Left )
    {
        setTargetState(LaraState::StepLeft);
    }
    else if( m_stepMovement == AxisMovement::Right )
    {
        setTargetState(LaraState::StepRight);
    }
    if( m_xMovement == AxisMovement::Left )
    {
        setTargetState(LaraState::TurnLeftSlow);
    }
    else if( m_xMovement == AxisMovement::Right )
    {
        setTargetState(LaraState::TurnRightSlow);
    }
    if( m_jump )
    {
        setTargetState(LaraState::JumpPrepare);
    }
    if( m_zMovement == AxisMovement::Forward )
    {
        if( m_moveSlow )
            onInput0WalkForward();
        else
            onInput1RunForward();
    }
    else if( m_zMovement == AxisMovement::Backward )
    {
        if( m_moveSlow )
            onInput16WalkBackward();
        else
            setTargetState(LaraState::RunBack);
    }
}

void LaraStateHandler::onInput3JumpForward()
{
    if( getTargetState() == LaraState::SwandiveBegin || getTargetState() == LaraState::Reach )
        setTargetState(LaraState::JumpForward);

    if( getTargetState() != LaraState::Death && getTargetState() != LaraState::Stop )
    {
        //! @todo Not only m_action, but also free hands!
        if( m_action )
            setTargetState(LaraState::Reach);
        //! @todo Not only m_action, but also free hands!
        if( m_moveSlow )
            setTargetState(LaraState::SwandiveBegin);
        if( m_fallSpeed > 131 )
            setTargetState(LaraState::FreeFall);
    }

    if( m_xMovement == AxisMovement::Left )
    {
        m_yRotationSpeed = std::max(-546, m_yRotationSpeed - 409);
    }
    else if( m_xMovement == AxisMovement::Right )
    {
        m_yRotationSpeed = std::min(546, m_yRotationSpeed + 409);
    }
}

void LaraStateHandler::onInput5RunBackward()
{
    setTargetState(LaraState::Stop);

    if( m_xMovement == AxisMovement::Left )
        m_yRotationSpeed = std::max(-1092, m_yRotationSpeed - 409);
    else if( m_xMovement == AxisMovement::Right )
        m_yRotationSpeed = std::min(1092, m_yRotationSpeed + 409);
}

void LaraStateHandler::onInput6TurnRightSlow()
{
    if( m_health <= 0 )
    {
        setTargetState(LaraState::Stop);
        return;
    }

    m_yRotationSpeed += 409;

    //! @todo Hand status
    if( false /* hands are in combat? */ )
    {
        setTargetState(LaraState::TurnFast);
        return;
    }

    if( m_yRotationSpeed > 728 )
    {
        if( m_moveSlow )
            m_yRotationSpeed = 728;
        else
            setTargetState(LaraState::TurnFast);
    }

    if( m_zMovement != AxisMovement::Forward )
    {
        if( m_xMovement != AxisMovement::Right )
            setTargetState(LaraState::Stop);
        return;
    }

    if( m_moveSlow )
        setTargetState(LaraState::WalkForward);
    else
        setTargetState(LaraState::RunForward);
}

void LaraStateHandler::onInput7TurnLeftSlow()
{
    if( m_health <= 0 )
    {
        setTargetState(LaraState::Stop);
        return;
    }

    m_yRotationSpeed -= 409;

    //! @todo Hand status
    if( false /* hands are in combat? */ )
    {
        setTargetState(LaraState::TurnFast);
        return;
    }

    if( m_yRotationSpeed < -728 )
    {
        if( m_moveSlow )
            m_yRotationSpeed = -728;
        else
            setTargetState(LaraState::TurnFast);
    }

    if( m_zMovement != AxisMovement::Forward )
    {
        if( m_xMovement != AxisMovement::Left )
            setTargetState(LaraState::Stop);
        return;
    }

    if( m_moveSlow )
        setTargetState(LaraState::WalkForward);
    else
        setTargetState(LaraState::RunForward);
}

void LaraStateHandler::onInput16WalkBackward()
{
    if( m_health <= 0 )
    {
        setTargetState(LaraState::Stop);
        return;
    }

    if( m_zMovement == AxisMovement::Backward && m_moveSlow )
        setTargetState(LaraState::WalkBackward);
    else
        setTargetState(LaraState::Stop);

    if( m_xMovement == AxisMovement::Left )
        m_yRotationSpeed = std::max(-728, m_yRotationSpeed - 409);
    else if( m_xMovement == AxisMovement::Right )
        m_yRotationSpeed = std::min(728, m_yRotationSpeed + 409);
}

void LaraStateHandler::onInput20TurnFast()
{
    if( m_health <= 0 )
    {
        setTargetState(LaraState::Stop);
        return;
    }

    if( m_yRotationSpeed >= 0 )
    {
        m_yRotationSpeed = 1456;
        if( m_xMovement == AxisMovement::Right )
            return;
    }
    else
    {
        m_yRotationSpeed = -1456;
        if( m_xMovement == AxisMovement::Left )
            return;
    }
    setTargetState(LaraState::Stop);
}

void LaraStateHandler::onInput25JumpBackward()
{
    //! @todo Set local camera Y rotation to 24570 AU
    if( m_fallSpeed > 131 )
        setTargetState(LaraState::FreeFall);
}

void LaraStateHandler::animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs)
{
    BOOST_ASSERT(m_lara == node);

    const auto currentFrame = 30 * timeMs / 1000;
    if( currentFrame == m_lastActiveFrame )
        return;
    m_lastActiveFrame = currentFrame;

    //! @todo Only when on solid ground
    m_air = 1800;

    ::LaraState laraState;
    laraState.position = m_lara->getPosition();
    laraState.collisionRadius = 100; //!< @todo MAGICK 100
    laraState.frobbelFlags = FrobbelFlag10 | FrobbelFlag08;

    static std::array<InputHandler, 56> inputHandlers{{
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
    }};

    const auto currentState = m_dispatcher->getCurrentState();
    if( currentState >= inputHandlers.size() )
    {
        BOOST_LOG_TRIVIAL(error) << "Unexpected state " << currentState;
        return;
    }

    if( !inputHandlers[currentState] )
        BOOST_LOG_TRIVIAL(warning) << "No input handler for state " << currentState;
    else
        (this->*inputHandlers[currentState])();

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

    if( m_falling )
    {
        m_horizontalSpeed += m_dispatcher->getAccelleration();
        if( m_fallSpeed >= 128 )
            m_fallSpeed += 1;
        else
            m_fallSpeed += 6;
    }
    else
    {
        m_horizontalSpeed = m_dispatcher->calculateFloorSpeed();
    }

    m_movementAngle = m_rotation.Y;

    processAnimCommands();

    // behaviour handling depends on the current state *after* handling the input
    switch( static_cast<LaraState>(m_dispatcher->getCurrentState()) )
    {
        case LaraState::RunForward:
            m_movementAngle = m_rotation.Y;
            laraState.yAngle = m_rotation.Y;
            laraState.fruityFloorLimitMax = loader::HeightLimit;
            laraState.fruityFloorLimitMin = -384; //!< @todo MAGICK -384
            laraState.frobbelFlags |= FrobbelFlag01;
            laraState.initHeightInfo(this, *m_level, 762, true); //! @todo MAGICK 762 and hardcoded parameter value "true"
            if(tryStopOnFloor(laraState) || tryClimb(laraState))
                break;

            //! @todo complete implementation
            break;
        case LaraState::WalkBackward:
        case LaraState::RunBack:
        case LaraState::JumpBack:
            m_movementAngle += 32768;
            break;
        default:
            break;
    }
}

void LaraStateHandler::processAnimCommands()
{
    const loader::Animation& animation = m_level->m_animations[m_dispatcher->getCurrentAnimationId()];
    if( animation.animCommandCount > 0 )
    {
        BOOST_ASSERT(animation.animCommandIndex < m_level->m_animCommands.size());
        const auto* cmd = &m_level->m_animCommands[animation.animCommandIndex];
        for( uint16_t i = 0; i < animation.animCommandCount; ++i )
        {
            BOOST_ASSERT(cmd < &m_level->m_animCommands.back());
            const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
            ++cmd;
            switch( opcode )
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
                if( m_dispatcher->getCurrentFrame() == cmd[0] )
                {
                    //! @todo playsound(cmd[1])
                }
                cmd += 2;
                break;
            case AnimCommandOpcode::PlayEffect:
                if( m_dispatcher->getCurrentFrame() == cmd[0] )
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
        q *= irr::core::quaternion().fromAngleAxis(util::auToRad(m_rotation.Y), {0,1,0});
        q *= irr::core::quaternion().fromAngleAxis(util::auToRad(m_rotation.X), {1,0,0});
        q *= irr::core::quaternion().fromAngleAxis(util::auToRad(m_rotation.Z), {0,0,-1});

        irr::core::vector3df euler;
        q.toEuler(euler);
        m_lara->setRotation(euler * 180 / irr::core::PI);
    }

    m_lara->updateAbsolutePosition();
}

bool LaraStateHandler::tryStopOnFloor(::LaraState& state)
{
    if(state.axisCollisions != ::LaraState::AxisColl08 && state.axisCollisions != ::LaraState::AxisColl20)
        return false;

    m_lara->setPosition(state.position);
    m_lara->updateAbsolutePosition();

    setTargetState(LaraState::Stop);
    playAnimation(loader::AnimationId::STAY_SOLID);
    //! @todo Set frame = 185
    m_horizontalSpeed = 0;
    m_fallSpeed = 0;
    m_falling = false;
    return true;
}

bool LaraStateHandler::tryClimb(::LaraState& state)
{
    if(state.axisCollisions != ::LaraState::AxisColl01 || !m_jump) //! @todo Also return false if Lara's hands are not free
        return false;

    const auto floorGradient = std::abs(state.frontLeftHI.floor.height - state.frontRightHI.floor.height);
    if(floorGradient >= 60) //! @todo MAGICK 60
        return false;

    int alignedRotation;
    //! @todo MAGICK +/- 30 degrees
    if(m_rotation.Y >= util::degToAu(-30) && m_rotation.Y <= util::degToAu(30))
        alignedRotation = util::degToAu(0);
    else if(m_rotation.Y >= util::degToAu(60) && m_rotation.Y <= util::degToAu(120))
        alignedRotation = util::degToAu(90);
    else if(m_rotation.Y >= util::degToAu(150) && m_rotation.Y <= util::degToAu(210))
        alignedRotation = util::degToAu(180);
    else if(m_rotation.Y >= util::degToAu(240) && m_rotation.Y <= util::degToAu(300))
        alignedRotation = util::degToAu(270);
    else
        return false;

    const auto climbHeight = state.frontHI.floor.height;
    if(climbHeight >= -640 && climbHeight <= -384)
    {
        if(climbHeight < state.frontHI.ceiling.height
            || state.frontLeftHI.floor.height < state.frontLeftHI.ceiling.height
            || state.frontRightHI.floor.height < state.frontRightHI.ceiling.height)
            return false;

        setTargetState(LaraState::Stop);
        playAnimation(loader::AnimationId::CLIMB_2CLICK);
        //! @todo Set frame = 759
        //! @todo Set hand status to "Climbing"
        m_lara->setPosition(m_lara->getPosition() + irr::core::vector3df(0, 2 * loader::QuarterSectorSize + climbHeight, 0));
    }
    else if(climbHeight >= -896 && climbHeight <= -640)
    {
        if(state.frontHI.floor.height < state.frontHI.ceiling.height
            || state.frontLeftHI.floor.height < state.frontLeftHI.ceiling.height
            || state.frontRightHI.floor.height < state.frontRightHI.ceiling.height)
            return false;

        setTargetState(LaraState::Stop);
        playAnimation(loader::AnimationId::CLIMB_3CLICK);
        //! @todo Set frame = 614
        //! @todo Set hand status to "Climbing"
        m_lara->setPosition(m_lara->getPosition() + irr::core::vector3df(0, 3 * loader::QuarterSectorSize + climbHeight, 0));
    }
    else
    {
        if(climbHeight < -1920 || climbHeight > -896)
            return false;

        setTargetState(LaraState::JumpUp);
        playAnimation(loader::AnimationId::STAY_SOLID);
        //! @todo Set frame = 185
        m_fallSpeedOverride = std::sqrt(-12 * (climbHeight+800) + 3);
        processAnimCommands();
    }

    m_rotation.Y = alignedRotation;
    applyCollisionFeedback(state);

    return true;
}

void LaraStateHandler::applyCollisionFeedback(::LaraState& state)
{
    m_lara->setPosition(m_lara->getPosition() + state.collisionFeedback);
    state.collisionFeedback = { 0,0,0 };
}
