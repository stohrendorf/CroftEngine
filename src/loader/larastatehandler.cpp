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

constexpr int SteppableHeight = loader::QuarterSectorSize / 2;
constexpr int ClimbLimit2ClickMin = loader::QuarterSectorSize + SteppableHeight;
constexpr int ClimbLimit2ClickMax = loader::QuarterSectorSize + ClimbLimit2ClickMin;
constexpr int ClimbLimit3ClickMax = loader::QuarterSectorSize + ClimbLimit2ClickMax;
constexpr int ReachableHeight = 896 + loader::SectorSize;

enum class SlantClass
{
    None,
    Max512,
    Steep
};

struct HeightInfo
{
    int height = 0;
    SlantClass slantClass = SlantClass::None;
    const uint16_t* lastTriggerOrKill = nullptr;

    static bool skipSteepSlants;

    static HeightInfo fromFloor(const loader::Sector* roomSector, const loader::TRCoordinates& pos, const TRCameraSceneNodeAnimator* camera)
    {
        HeightInfo hi;

        hi.slantClass = SlantClass::None;

        BOOST_ASSERT(roomSector != nullptr);
        for( auto room = camera->getCurrentRoom(); roomSector->roomBelow != 0xff; roomSector = room->getSectorByAbsolutePosition(pos) )
        {
            BOOST_ASSERT(roomSector->roomAbove < camera->getLevel()->m_rooms.size());
            room = &camera->getLevel()->m_rooms[roomSector->roomBelow];
        }

        hi.height = roomSector->floorHeight * loader::QuarterSectorSize;
        hi.lastTriggerOrKill = nullptr;

        if( roomSector->floorDataIndex == 0 )
        {
            return hi;
        }

        const uint16_t* floorData = &camera->getLevel()->m_floorData[roomSector->floorDataIndex];
        while( true )
        {
            const bool isLast = loader::isLastFloorataEntry(*floorData);
            const auto currentFd = *floorData;
            ++floorData;
            switch( loader::extractFDFunction(currentFd) )
            {
            case loader::FDFunction::FloorSlant:
                {
                    const int8_t xSlant = static_cast<int8_t>(*floorData & 0xff);
                    const auto absX = std::abs(xSlant);
                    const int8_t zSlant = static_cast<int8_t>((*floorData >> 8) & 0xff);
                    const auto absZ = std::abs(zSlant);
                    if( !skipSteepSlants || (absX <= 2 && absZ <= 2) )
                    {
                        if( absX <= 2 && absZ <= 2 )
                            hi.slantClass = SlantClass::Max512;
                        else
                            hi.slantClass = SlantClass::Steep;

                        const irr::f32 localX = pos.X % loader::SectorSize;
                        const irr::f32 localZ = pos.Z % loader::SectorSize;

                        if( zSlant > 0 ) // lower edge at -Z
                        {
                            auto dist = (loader::SectorSize - localZ) / loader::SectorSize;
                            hi.height += static_cast<int>(dist * zSlant * loader::QuarterSectorSize);
                        }
                        else if( zSlant < 0 ) // lower edge at +Z
                        {
                            auto dist = localZ / loader::SectorSize;
                            hi.height -= static_cast<int>(dist * zSlant * loader::QuarterSectorSize);
                        }

                        if( xSlant > 0 ) // lower edge at -X
                        {
                            auto dist = (loader::SectorSize - localX) / loader::SectorSize;
                            hi.height += static_cast<int>(dist * xSlant * loader::QuarterSectorSize);
                        }
                        else if( xSlant < 0 ) // lower edge at +X
                        {
                            auto dist = localX / loader::SectorSize;
                            hi.height -= static_cast<int>(dist * xSlant * loader::QuarterSectorSize);
                        }
                    }
                }
                // Fall-through
            case loader::FDFunction::CeilingSlant:
            case loader::FDFunction::PortalSector:
                ++floorData;
                break;
            case loader::FDFunction::Death:
                hi.lastTriggerOrKill = floorData - 1;
                break;
            case loader::FDFunction::Trigger:
                if( !hi.lastTriggerOrKill )
                    hi.lastTriggerOrKill = floorData - 1;
                ++floorData;
                while( true )
                {
                    const bool isLastTrigger = loader::isLastFloorataEntry(*floorData);

                    const auto func = loader::extractTriggerFunction(*floorData);
                    const auto param = loader::extractTriggerFunctionParam(*floorData);
                    ++floorData;

                    if( func != loader::TriggerFunction::Object )
                    {
                        if( func == loader::TriggerFunction::CameraTarget )
                        {
                            ++floorData;
                        }
                    }
                    else
                    {
                        BOOST_ASSERT(func == loader::TriggerFunction::Object);
                        //! @todo Query height patch from object @c param, e.g. trapdoors or falling floor.
                    }

                    if( isLastTrigger )
                        break;
                }
            default:
                break;
            }
            if( isLast )
                break;
        }

        return hi;
    }

    static HeightInfo fromCeiling(const loader::Sector* roomSector, const loader::TRCoordinates& pos, const TRCameraSceneNodeAnimator* camera)
    {
        HeightInfo hi;

        BOOST_ASSERT(roomSector != nullptr);
        for( auto room = camera->getCurrentRoom(); roomSector->roomAbove != 0xff; roomSector = room->getSectorByAbsolutePosition(pos) )
        {
            BOOST_ASSERT(roomSector->roomAbove < camera->getLevel()->m_rooms.size());
            room = &camera->getLevel()->m_rooms[roomSector->roomAbove];
        }

        hi.height = roomSector->ceilingHeight * loader::QuarterSectorSize;

        if( roomSector->floorDataIndex == 0 )
        {
            return hi;
        }

        const uint16_t* floorData = &camera->getLevel()->m_floorData[roomSector->floorDataIndex];
        while( true )
        {
            const bool isLast = loader::isLastFloorataEntry(*floorData);
            const auto currentFd = *floorData;
            ++floorData;
            switch( loader::extractFDFunction(currentFd) )
            {
            case loader::FDFunction::CeilingSlant:
                {
                    const int8_t xSlant = static_cast<int8_t>(*floorData & 0xff);
                    const auto absX = std::abs(xSlant);
                    const int8_t zSlant = static_cast<int8_t>((*floorData >> 8) & 0xff);
                    const auto absZ = std::abs(zSlant);
                    if( !skipSteepSlants || (absX <= 2 && absZ <= 2) )
                    {
                        const irr::f32 localX = pos.X % loader::SectorSize;
                        const irr::f32 localZ = pos.Z % loader::SectorSize;

                        if( zSlant > 0 ) // lower edge at -Z
                        {
                            auto dist = (loader::SectorSize - localZ) / loader::SectorSize;
                            hi.height -= static_cast<int>(dist * zSlant * loader::QuarterSectorSize);
                        }
                        else if( zSlant < 0 ) // lower edge at +Z
                        {
                            auto dist = localZ / loader::SectorSize;
                            hi.height += static_cast<int>(dist * zSlant * loader::QuarterSectorSize);
                        }

                        if( xSlant > 0 ) // lower edge at -X
                        {
                            auto dist = (loader::SectorSize - localX) / loader::SectorSize;
                            hi.height -= static_cast<int>(dist * xSlant * loader::QuarterSectorSize);
                        }
                        else if( xSlant < 0 ) // lower edge at +X
                        {
                            auto dist = localX / loader::SectorSize;
                            hi.height += static_cast<int>(dist * xSlant * loader::QuarterSectorSize);
                        }
                    }
                }
                // Fall-through
            case loader::FDFunction::FloorSlant:
            case loader::FDFunction::PortalSector:
                ++floorData;
                break;
            case loader::FDFunction::Death:
                break;
            case loader::FDFunction::Trigger:
                ++floorData;
                while( true )
                {
                    const bool isLastTrigger = loader::isLastFloorataEntry(*floorData);

                    const auto func = loader::extractTriggerFunction(*floorData);
                    const auto param = loader::extractTriggerFunctionParam(*floorData);
                    ++floorData;

                    if( func != loader::TriggerFunction::Object )
                    {
                        if( func == loader::TriggerFunction::CameraTarget )
                        {
                            ++floorData;
                        }
                    }
                    else
                    {
                        BOOST_ASSERT(func == loader::TriggerFunction::Object);
                        //! @todo Query height patch from object @c param.
                    }

                    if( isLastTrigger )
                        break;
                }
            default:
                break;
            }
            if( isLast )
                break;
        }

        return hi;
    }

    HeightInfo() = default;
};

bool HeightInfo::skipSteepSlants = false;

struct VerticalInfo
{
    HeightInfo floor;
    HeightInfo ceiling;

    void init(const loader::Sector* roomSector, const loader::TRCoordinates& pos, const TRCameraSceneNodeAnimator* camera, int height)
    {
        floor = HeightInfo::fromFloor(roomSector, pos, camera);
        if( floor.height != -loader::HeightLimit )
            floor.height -= pos.Y;

        ceiling = HeightInfo::fromCeiling(roomSector, pos, camera);
        if( ceiling.height != -loader::HeightLimit )
            ceiling.height -= pos.Y - height;
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
    static constexpr int AxisColl_None = 0x00;
    static constexpr int AxisColl_CannotGoForward = 0x01;
    static constexpr int AxisColl_FrontLeftBump = 0x02;
    static constexpr int AxisColl_FrontRightBump = 0x04;
    static constexpr int AxisColl_HeadInCeiling = 0x08;
    static constexpr int AxisColl_BumpHead = 0x10;
    static constexpr int AxisColl_CeilingTooLow = 0x20;
    static constexpr int AxisColl40 = 0x40;
    static constexpr int AxisColl80 = 0x80;

    int axisCollisions;
    loader::TRCoordinates collisionFeedback;
    Axis orientationAxis;
    irr::s16 yAngle; // external
    int collisionRadius; // external
    int frobbelFlags; // external
    loader::TRCoordinates position; // external
    int fruityFloorLimitBottom; // external
    int fruityFloorLimitTop; // external
    int fruityCeilingLimit; // external

    VerticalInfo current;
    VerticalInfo front;
    VerticalInfo frontLeft;
    VerticalInfo frontRight;

    int8_t floorSlantX;
    int8_t floorSlantZ;

    static int fruityFeedback(int a, int b)
    {
        const auto sectorA = a / loader::SectorSize;
        const auto sectorB = b / loader::SectorSize;
        if( sectorA == sectorB )
            return 0;

        const auto localA = (a % loader::SectorSize) + 1;
        if( sectorB <= sectorA )
            return -localA;
        else
            return loader::SectorSize - localA;
    }

    void initHeightInfo(LaraStateHandler* lara, const loader::Level& level, int height, const loader::TRCoordinates& pos)
    {
        axisCollisions = AxisColl_None;
        collisionFeedback = {0,0,0};
        orientationAxis = static_cast<Axis>(static_cast<irr::u16>(yAngle + util::degToAu(45)) / util::degToAu(90));

        const loader::Room* room = level.m_camera->getCurrentRoom();
        const loader::TRCoordinates deltaHeight(0, height + 160, 0); //!< @todo MAGICK 160
        auto currentSector = level.findSectorForPosition(pos - deltaHeight, room);
        BOOST_ASSERT(currentSector != nullptr);

        current.init(currentSector, pos, level.m_camera, height);

        std::tie(floorSlantX, floorSlantZ) = level.getFloorSlantInfo(currentSector, loader::TRCoordinates(pos.X, lara->getPosition().Y, pos.Z));

        int frontX = 0, frontZ = 0;
        int frontLeftX = 0, frontLeftZ = 0;
        int frontRightX = 0, frontRightZ = 0;

        switch( orientationAxis )
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
        auto checkPos = pos + loader::TRCoordinates(frontX, 0, frontZ);
        auto sector = level.findSectorForPosition(checkPos, level.m_camera->getCurrentRoom());
        front.init(sector, checkPos, level.m_camera, height);
        if( (frobbelFlags & FrobbelFlag01) != 0 && front.floor.slantClass == SlantClass::Steep && front.floor.height < 0 )
        {
            front.floor.height = -32767;
        }
        else if( front.floor.height > 0
            && (
                ((frobbelFlags & FrobbelFlag02) != 0 && front.floor.slantClass == SlantClass::Steep)
                || ((frobbelFlags & FrobbelFlag04) != 0 && front.floor.lastTriggerOrKill != nullptr && loader::extractFDFunction(*front.floor.lastTriggerOrKill) == loader::FDFunction::Death)
            ) )
        {
            front.floor.height = 2 * loader::QuarterSectorSize;
        }

        // Front left
        checkPos = pos + loader::TRCoordinates(frontLeftX, 0, frontLeftZ);
        sector = level.findSectorForPosition(checkPos, level.m_camera->getCurrentRoom());
        frontLeft.init(sector, checkPos, level.m_camera, height);

        if( (frobbelFlags & FrobbelFlag01) != 0 && frontLeft.floor.slantClass == SlantClass::Steep && frontLeft.floor.height < 0 )
        {
            frontLeft.floor.height = -32767;
        }
        else if( frontLeft.floor.height > 0
            && (
                ((frobbelFlags & FrobbelFlag02) != 0 && frontLeft.floor.slantClass == SlantClass::Steep)
                || ((frobbelFlags & FrobbelFlag04) != 0 && frontLeft.floor.lastTriggerOrKill != nullptr && loader::extractFDFunction(*frontLeft.floor.lastTriggerOrKill) == loader::FDFunction::Death)
            ) )
        {
            frontLeft.floor.height = 2 * loader::QuarterSectorSize;
        }

        // Front right
        checkPos = pos + loader::TRCoordinates(frontRightX, 0, frontRightZ);
        sector = level.findSectorForPosition(checkPos, level.m_camera->getCurrentRoom());
        frontRight.init(sector, checkPos, level.m_camera, height);

        if( (frobbelFlags & FrobbelFlag01) != 0 && frontRight.floor.slantClass == SlantClass::Steep && frontRight.floor.height < 0 )
        {
            frontRight.floor.height = -32767;
        }
        else if( frontRight.floor.height > 0
            && (
                ((frobbelFlags & FrobbelFlag02) != 0 && frontRight.floor.slantClass == SlantClass::Steep)
                || ((frobbelFlags & FrobbelFlag04) != 0 && frontRight.floor.lastTriggerOrKill != nullptr && loader::extractFDFunction(*frontRight.floor.lastTriggerOrKill) == loader::FDFunction::Death)
            ) )
        {
            frontRight.floor.height = 2 * loader::QuarterSectorSize;
        }

        //! @todo check static mesh collisions here

        if( current.floor.height == -loader::HeightLimit )
        {
            collisionFeedback = position - pos;
            axisCollisions = AxisColl_CannotGoForward;
            return;
        }

        if( current.floor.height <= current.ceiling.height )
        {
            axisCollisions = AxisColl_CeilingTooLow;
            collisionFeedback = position - pos;
            return;
        }

        if( current.ceiling.height >= 0 )
        {
            axisCollisions = AxisColl_HeadInCeiling;
            collisionFeedback.Y = current.ceiling.height;
        }

        if( front.floor.height > fruityFloorLimitBottom || front.floor.height < fruityFloorLimitTop || front.ceiling.height > fruityCeilingLimit )
        {
            axisCollisions = AxisColl_CannotGoForward;
            switch( orientationAxis )
            {
            case Axis::PosZ:
            case Axis::NegZ:
                collisionFeedback.X = position.X - pos.X;
                collisionFeedback.Z = fruityFeedback(frontZ + pos.Z, pos.Z);
                break;
            case Axis::PosX:
            case Axis::NegX:
                collisionFeedback.X = fruityFeedback(frontX + pos.X, pos.X);
                collisionFeedback.Z = position.Z - pos.Z;
                break;
            }
            return;
        }

        if( front.ceiling.height >= fruityCeilingLimit )
        {
            axisCollisions = AxisColl_BumpHead;
            collisionFeedback = position - pos;
            return;
        }

        if( frontLeft.floor.height > fruityFloorLimitBottom || frontLeft.floor.height < fruityFloorLimitTop )
        {
            axisCollisions = AxisColl_FrontLeftBump;
            switch( orientationAxis )
            {
            case Axis::PosZ:
            case Axis::NegZ:
                collisionFeedback.X = fruityFeedback(frontLeftX + pos.X, frontX + pos.X);
                break;
            case Axis::PosX:
            case Axis::NegX:
                collisionFeedback.Z = fruityFeedback(frontLeftZ + pos.Z, frontZ + pos.Z);
                break;
            }
            return;
        }

        if( frontRight.floor.height > fruityFloorLimitBottom || frontRight.floor.height < fruityFloorLimitTop )
        {
            axisCollisions = AxisColl_FrontRightBump;
            switch( orientationAxis )
            {
            case Axis::PosZ:
            case Axis::NegZ:
                collisionFeedback.X = fruityFeedback(frontRightX + pos.X, frontX + pos.X);
                break;
            case Axis::PosX:
            case Axis::NegX:
                collisionFeedback.Z = fruityFeedback(frontRightZ + pos.Z, frontZ + pos.Z);
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

void LaraStateHandler::playAnimation(loader::AnimationId anim, const boost::optional<irr::u32>& firstFrame)
{
    m_dispatcher->playLocalAnimation(static_cast<uint16_t>(anim), firstFrame);
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
        playAnimation(loader::AnimationId::ROLL_BEGIN, 3857);
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
        if( m_action && m_handStatus == 0 )
            setTargetState(LaraState::Reach);
        if( m_moveSlow && m_handStatus == 0 )
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

    if( m_handStatus == 4 )
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

    if( m_handStatus == 4 )
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

void LaraStateHandler::onInput9FreeFall()
{
    m_horizontalSpeed = 95 * m_horizontalSpeed / 100;
    if( m_fallSpeed > 154 )
    {
        //! @todo playSound(30)
    }
}

void LaraStateHandler::onInput15JumpPrepare()
{
    if(m_zMovement == AxisMovement::Forward && getRelativeHeightAtDirection(m_rotation.Y, 256) >= -384)
    {
        m_movementAngle = m_rotation.Y;
        setTargetState(LaraState::JumpForward);
    }
    else if(m_xMovement == AxisMovement::Left && getRelativeHeightAtDirection(m_rotation.Y - util::degToAu(90), 256) >= -384)
    {
        m_movementAngle = m_rotation.Y - util::degToAu(90);
        setTargetState(LaraState::JumpRight);
    }
    else if(m_xMovement == AxisMovement::Right && getRelativeHeightAtDirection(m_rotation.Y + util::degToAu(90), 256) >= -384)
    {
        m_movementAngle = m_rotation.Y + util::degToAu(90);
        setTargetState(LaraState::JumpLeft);
    }
    else if(m_zMovement == AxisMovement::Backward && getRelativeHeightAtDirection(m_rotation.Y + util::degToAu(180), 256) >= -384)
    {
        m_movementAngle = m_rotation.Y + util::degToAu(180);
        setTargetState(LaraState::JumpBack);
    }

    if(m_fallSpeed > 131)
    {
        setTargetState(LaraState::FreeFall);
    }
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

void LaraStateHandler::onInput28JumpUp()
{
    if( m_fallSpeed > 131 )
        setTargetState(LaraState::FreeFall);
}

void LaraStateHandler::handleLaraStateOnLand()
{
    //! @todo Only when on solid ground
    m_air = 1800;

    ::LaraState laraState;
    laraState.position = m_position;
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
        &LaraStateHandler::onInput9FreeFall,
        // 10
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        &LaraStateHandler::onInput15JumpPrepare,
        &LaraStateHandler::onInput16WalkBackward,
        nullptr,
        nullptr,
        nullptr,
        &LaraStateHandler::onInput20TurnFast,
        nullptr,nullptr,nullptr,nullptr,
        &LaraStateHandler::onInput25JumpBackward,
        nullptr,nullptr,
        &LaraStateHandler::onInput28JumpUp,
        nullptr,
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
        (this ->* inputHandlers[currentState])();

    // "slowly" revert rotations to zero
    if( m_rotation.Z < -182 )
        m_rotation.Z += 182;
    else if( m_rotation.Z > 182 )
        m_rotation.Z -= 182;
    else
        m_rotation.Z = 0;

    if( m_yRotationSpeed < -364 )
        m_yRotationSpeed += 364;
    else if( m_yRotationSpeed > 364 )
        m_yRotationSpeed -= 364;
    else
        m_yRotationSpeed = 0;

    m_rotation.Y += m_yRotationSpeed;

    processAnimCommands();

    // @todo test interactions?

    // behaviour handling depends on the current state *after* handling the input
    switch( static_cast<LaraState>(m_dispatcher->getNextFrameState()) )
    {
    case LaraState::Stop:
    case LaraState::Pose:
    case LaraState::GrabToFall:
    case LaraState::TurnFast:
        m_fallSpeed = 0;
        m_falling = false;
        laraState.yAngle = m_rotation.Y;
        m_movementAngle = m_rotation.Y;
        laraState.fruityFloorLimitTop = -ClimbLimit2ClickMin;
        laraState.fruityFloorLimitBottom = ClimbLimit2ClickMin;
        laraState.fruityCeilingLimit = 0;
        laraState.frobbelFlags |= FrobbelFlag01 | FrobbelFlag02;
        laraState.initHeightInfo(this, *m_level, 762, m_position); //! @todo MAGICK 762
        if( tryStopOnFloor(laraState) )
            break;
        if( laraState.current.floor.height <= 100 )
        {
            if( !tryStartSlide(laraState) )
            {
                applyCollisionFeedback(laraState);
                m_position.Y += laraState.current.floor.height;
            }
        }
        else
        {
            playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
            setTargetState(loader::LaraState::JumpForward);
            m_fallSpeed = 0;
            m_falling = true;
        }
        break;
    case LaraState::RunForward:
        onBehave1RunForward(laraState);
        break;
    case LaraState::RunBack:
        onBehave5RunBackward(laraState);
        break;
    case LaraState::WalkBackward:
        onBehave16WalkBackward(laraState);
        break;
    case LaraState::JumpBack:
        m_movementAngle = m_rotation.Y + util::degToAu(180);
        commonJumpHandling(laraState);
        break;
    case LaraState::JumpLeft:
        m_movementAngle = m_rotation.Y + util::degToAu(90);
        commonJumpHandling(laraState);
        break;
    case LaraState::JumpRight:
        m_movementAngle = m_rotation.Y - util::degToAu(90);
        commonJumpHandling(laraState);
        break;
    case LaraState::FreeFall:
        onBehave9FreeFall(laraState);
        break;
    case LaraState::JumpUp:
        onBehave28JumpUp(laraState);
        break;
    case LaraState::JumpForward:
        onBehave3JumpForward(laraState);
        break;
    case LaraState::JumpPrepare:
        onBehave15JumpPrepare(laraState);
        break;
    case LaraState::Climbing:
        laraState.fruityFloorLimitBottom = ClimbLimit2ClickMin;
        laraState.fruityFloorLimitTop = -ClimbLimit2ClickMin;
        laraState.fruityCeilingLimit = 0;
        laraState.yAngle = m_movementAngle = m_rotation.Y;
        laraState.frobbelFlags |= FrobbelFlag01 | FrobbelFlag02;
        laraState.initHeightInfo(this, *m_level, 762, m_position);
        break;
    default:
        BOOST_LOG_TRIVIAL(warning) << "No behaviour handler for state " << m_dispatcher->getNextFrameState();
        break;
    }

    updateFloorHeight(-381);
    handleTriggers(laraState.current.floor.lastTriggerOrKill, false);
}

void LaraStateHandler::onBehave15JumpPrepare(::LaraState& state)
{
    m_fallSpeed = 0;
    m_falling = false;
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -loader::HeightLimit;
    state.fruityCeilingLimit = 0;
    state.yAngle = m_rotation.Y;
    state.initHeightInfo(this, *m_level, 762, m_position); //! @todo MAGICK 762

    if(state.current.ceiling.height <= -100)
        return;

    setTargetState(LaraState::Stop);
    playAnimation(loader::AnimationId::STAY_SOLID, 185);
    m_horizontalSpeed = 0;
    m_falling = false;
    m_position = state.position;
}


void LaraStateHandler::onBehave1RunForward(::LaraState& state)
{
    m_movementAngle = m_rotation.Y;
    state.yAngle = m_rotation.Y;
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 0;
    state.frobbelFlags |= FrobbelFlag01;
    state.initHeightInfo(this, *m_level, 762, m_position); //! @todo MAGICK 762
    if(tryStopOnFloor(state) || tryClimb(state))
        return;

    if(checkWallCollision(state))
    {
        m_rotation.Z = 0;
        if(state.front.floor.slantClass == SlantClass::None && state.front.floor.height < -ClimbLimit2ClickMax)
        {
            if(m_dispatcher->getCurrentFrame() >= 0 && m_dispatcher->getCurrentFrame() <= 9)
            {
                playAnimation(loader::AnimationId::WALL_SMASH_LEFT, 800);
                return;
            }
            if(m_dispatcher->getCurrentFrame() >= 10 && m_dispatcher->getCurrentFrame() <= 21)
            {
                playAnimation(loader::AnimationId::WALL_SMASH_RIGHT, 815);
                return;
            }

            playAnimation(loader::AnimationId::STAY_SOLID, 185);
        }
    }

    if(state.current.floor.height > ClimbLimit2ClickMin)
    {
        playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
        //! @todo set current state = jump forward
        setTargetState(loader::LaraState::JumpForward);
        m_falling = true;
        m_fallSpeed = 0;
        return;
    }

    if(state.current.floor.height >= -ClimbLimit2ClickMin && state.current.floor.height < -SteppableHeight)
    {
        if(m_dispatcher->getCurrentFrame() >= 3 && m_dispatcher->getCurrentFrame() <= 14)
        {
            playAnimation(loader::AnimationId::RUN_UP_STEP_LEFT, 837);
        }
        else
        {
            playAnimation(loader::AnimationId::RUN_UP_STEP_RIGHT, 830);
        }
    }

    if(!tryStartSlide(state))
    {
        m_position.Y += std::min(50, state.current.floor.height);
    }
}

void LaraStateHandler::onBehave5RunBackward(::LaraState& state)
{
    m_fallSpeed = 0;
    m_falling = false;
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 0;
    state.frobbelFlags |= FrobbelFlag01 | FrobbelFlag02;
    m_movementAngle = m_rotation.Y + util::degToAu(180);
    state.yAngle = m_rotation.Y + util::degToAu(180);
    state.initHeightInfo(this, *m_level, 762, m_position); //! @todo MAGICK 762
    if(tryStopOnFloor(state))
        return;

    if(state.current.floor.height > 200)
    {
        playAnimation(loader::AnimationId::FREE_FALL_BACK, 1473);
        setTargetState(loader::LaraState::FallBackward);
        m_fallSpeed = 0;
        m_falling = true;
        return;
    }

    if(checkWallCollision(state))
    {
        playAnimation(loader::AnimationId::STAY_SOLID, 185);
    }
    m_position.Y += state.current.floor.height;
}


void LaraStateHandler::onBehave16WalkBackward(::LaraState& state)
{
    m_fallSpeed = 0;
    m_falling = false;
    state.fruityFloorLimitBottom = ClimbLimit2ClickMin;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 0;
    m_movementAngle = state.yAngle = m_rotation.Y + util::degToAu(180);
    state.frobbelFlags |= FrobbelFlag01 | FrobbelFlag02;
    state.initHeightInfo(this, *m_level, 762, m_position); //! @todo MAGICK 762
    if(tryStopOnFloor(state))
        return;

    if(checkWallCollision(state))
    {
        playAnimation(loader::AnimationId::STAY_SOLID, 185);
    }

    if(state.current.floor.height > 128 && state.current.floor.height < 384)
    {
        if(m_dispatcher->getCurrentFrame() < 964 && m_dispatcher->getCurrentFrame() > 993)
        {
            playAnimation(loader::AnimationId::WALK_DOWN_BACK_LEFT, 899);
        }
        else
        {
            playAnimation(loader::AnimationId::WALK_DOWN_BACK_RIGHT, 930);
        }
    }

    if(!tryStartSlide(state))
    {
        m_position.Y += state.current.floor.height;
    }
}


void LaraStateHandler::onBehave9FreeFall(::LaraState& state)
{
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 192;
    state.yAngle = m_movementAngle;
    m_falling = true;
    state.initHeightInfo(this, *m_level, 762, m_position); //! @todo MAGICK 762
    jumpAgainstWall(state);
    if(state.current.floor.height > 0)
        return;

    if(false) //! @todo if(applyLandingDamage())
    {
        setTargetState(loader::LaraState::Death);
    }
    else
    {
        setTargetState(loader::LaraState::Stop);
        playAnimation(loader::AnimationId::LANDING_HARD, 358);
    }
    m_fallSpeed = 0;
    m_position.Y += state.current.floor.height;
    m_falling = false;
}


void LaraStateHandler::onBehave3JumpForward(::LaraState& state)
{
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 192;
    m_movementAngle = state.yAngle = m_rotation.Y;
    state.initHeightInfo(this, *m_level, 762, m_position); //! @todo MAGICK 762
    checkJumpWallSmash(state);

    if(state.current.floor.height > 0 || m_fallSpeed <= 0)
        return;

    if(false) //! @todo applyLandingDamage(laraState)
    {
        setTargetState(LaraState::Death);
    }
    else if(m_zMovement != AxisMovement::Forward || m_moveSlow)
    {
        setTargetState(LaraState::Stop);
    }
    else
    {
        setTargetState(LaraState::RunForward);
    }

    m_fallSpeed = 0;
    m_falling = false;
    m_horizontalSpeed = 0;
    m_position.Y += state.current.floor.height;
    processAnimCommands();
}

void LaraStateHandler::onBehave28JumpUp(::LaraState& state)
{
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 192;
    state.yAngle = m_rotation.Y;
    state.initHeightInfo(this, *m_level, 870, m_position); //! @todo MAGICK 870
    if(tryGrabEdge(state))
        return;

    jumpAgainstWall(state);
    if(m_fallSpeed <= 0 || state.current.floor.height > 0)
        return;

    //! @todo Apply landing damage
    setTargetState(loader::LaraState::Stop);
    m_fallSpeed = 0;
    m_position.Y += state.current.floor.height;
    m_falling = false;
}

void LaraStateHandler::jumpAgainstWall(::LaraState& state)
{
    applyCollisionFeedback(state);
    if(state.axisCollisions == ::LaraState::AxisColl_FrontLeftBump)
        m_rotation.Y += 910;
    else if(state.axisCollisions == ::LaraState::AxisColl_FrontRightBump)
        m_rotation.Y -= 910;
    else if(state.axisCollisions == ::LaraState::AxisColl_HeadInCeiling)
    {
        if(m_fallSpeed <= 0)
            m_fallSpeed = 1;
    }
    else if(state.axisCollisions == ::LaraState::AxisColl_CeilingTooLow)
    {
        m_position.X += static_cast<long>(100 * std::sin(util::auToRad(m_rotation.Y)));
        m_position.Z += static_cast<long>(100 * std::cos(util::auToRad(m_rotation.Y)));
        m_horizontalSpeed = 0;
        state.current.floor.height = 0;
        if(m_fallSpeed < 0)
            m_fallSpeed = 16;
    }
}

void LaraStateHandler::checkJumpWallSmash(::LaraState& state)
{
    applyCollisionFeedback(state);

    if( state.axisCollisions == ::LaraState::AxisColl_None )
        return;

    if( state.axisCollisions == ::LaraState::AxisColl_CannotGoForward || state.axisCollisions == ::LaraState::AxisColl_BumpHead )
    {
        setTargetState(LaraState::FreeFall);
        m_horizontalSpeed -= m_horizontalSpeed * 3 / 4; //!< @todo Check formula
        m_movementAngle -= 32768;
        playAnimation(loader::AnimationId::SMASH_JUMP, 481);
        if( m_fallSpeed <= 0 )
            m_fallSpeed = 1;
        return;
    }

    if( state.axisCollisions == ::LaraState::AxisColl_FrontLeftBump )
    {
        m_rotation.Y += 910;
        return;
    }
    else if( state.axisCollisions == ::LaraState::AxisColl_FrontRightBump )
    {
        m_rotation.Y -= 910;
        return;
    }

    if( state.axisCollisions == ::LaraState::AxisColl_CeilingTooLow )
    {
        m_position.X -= 100 * std::sin(util::auToRad(state.yAngle));
        m_position.Z -= 100 * std::cos(util::auToRad(state.yAngle));
        m_horizontalSpeed = 0;
        state.current.floor.height = 0;
        if( m_fallSpeed <= 0 )
            m_fallSpeed = 16;
    }

    if( state.axisCollisions == ::LaraState::AxisColl_HeadInCeiling && m_fallSpeed <= 0 )
        m_fallSpeed = 1;
}

void LaraStateHandler::animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs)
{
    BOOST_ASSERT(m_lara == node);

    const auto currentFrame = 30 * timeMs / 1000;
    if( currentFrame == m_lastActiveFrame )
        return;
    m_lastActiveFrame = currentFrame;

    handleLaraStateOnLand();
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
                m_position += {cmd[0], cmd[1], cmd[2]};
                cmd += 3;
                break;
            case AnimCommandOpcode::SetVelocity:
                m_fallSpeed = m_fallSpeedOverride == 0 ? cmd[0] : m_fallSpeedOverride;
                m_fallSpeedOverride = 0;
                m_horizontalSpeed = cmd[1];
                m_falling = true;
                cmd += 2;
                break;
            case AnimCommandOpcode::EmptyHands:
                m_handStatus = 0;
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

    m_position.X += static_cast<long>(std::sin(util::auToRad(m_movementAngle)) * m_horizontalSpeed);
    m_position.Y += m_fallSpeed;
    m_position.Z += static_cast<long>(std::cos(util::auToRad(m_movementAngle)) * m_horizontalSpeed);
    m_lara->setPosition(m_position.toIrrlicht());

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
    if( state.axisCollisions != ::LaraState::AxisColl_HeadInCeiling && state.axisCollisions != ::LaraState::AxisColl_CeilingTooLow )
        return false;

    m_position = state.position;

    setTargetState(LaraState::Stop);
    playAnimation(loader::AnimationId::STAY_SOLID, 185);
    m_horizontalSpeed = 0;
    m_fallSpeed = 0;
    m_falling = false;
    return true;
}

bool LaraStateHandler::tryClimb(::LaraState& state)
{
    if( state.axisCollisions != ::LaraState::AxisColl_CannotGoForward || !m_jump || m_handStatus != 0 )
        return false;

    const auto floorGradient = std::abs(state.frontLeft.floor.height - state.frontRight.floor.height);
    if( floorGradient >= 60 ) //! @todo MAGICK 60
        return false;

    int alignedRotation;
    //! @todo MAGICK +/- 30 degrees
    if( m_rotation.Y >= util::degToAu(-30) && m_rotation.Y <= util::degToAu(30) )
        alignedRotation = util::degToAu(0);
    else if( m_rotation.Y >= util::degToAu(60) && m_rotation.Y <= util::degToAu(120) )
        alignedRotation = util::degToAu(90);
    else if( m_rotation.Y >= util::degToAu(150) && m_rotation.Y <= util::degToAu(210) )
        alignedRotation = util::degToAu(180);
    else if( m_rotation.Y >= util::degToAu(240) && m_rotation.Y <= util::degToAu(300) )
        alignedRotation = util::degToAu(270);
    else
        return false;

    const auto climbHeight = state.front.floor.height;
    if( climbHeight >= -ClimbLimit2ClickMax && climbHeight <= -ClimbLimit2ClickMin )
    {
        if( climbHeight < state.front.ceiling.height
            || state.frontLeft.floor.height < state.frontLeft.ceiling.height
            || state.frontRight.floor.height < state.frontRight.ceiling.height )
            return false;

        setTargetState(LaraState::Stop);
        playAnimation(loader::AnimationId::CLIMB_2CLICK, 759);
        m_position.Y -= 2 * loader::QuarterSectorSize + climbHeight;
        //! @todo m_handStatus = 1;
    }
    else if( climbHeight >= -ClimbLimit3ClickMax && climbHeight <= -ClimbLimit2ClickMax )
    {
        if( state.front.floor.height < state.front.ceiling.height
            || state.frontLeft.floor.height < state.frontLeft.ceiling.height
            || state.frontRight.floor.height < state.frontRight.ceiling.height )
            return false;

        setTargetState(LaraState::Stop);
        playAnimation(loader::AnimationId::CLIMB_3CLICK, 614);
        m_position.Y -= 3 * loader::QuarterSectorSize + climbHeight;
        //! @todo m_handStatus = 1;
    }
    else
    {
        if( climbHeight < -ReachableHeight || climbHeight > -ClimbLimit3ClickMax )
            return false;

        setTargetState(LaraState::JumpUp);
        playAnimation(loader::AnimationId::STAY_SOLID, 185);
        m_fallSpeedOverride = static_cast<int>(std::sqrt(-12 * (climbHeight + 800) + 3));
        processAnimCommands();
    }

    m_rotation.Y = alignedRotation;
    applyCollisionFeedback(state);

    return true;
}

void LaraStateHandler::applyCollisionFeedback(::LaraState& state)
{
    m_position += state.collisionFeedback;
    state.collisionFeedback = {0,0,0};
}

bool LaraStateHandler::checkWallCollision(::LaraState& state)
{
    if( state.axisCollisions == ::LaraState::AxisColl_CannotGoForward || state.axisCollisions == ::LaraState::AxisColl_BumpHead )
    {
        applyCollisionFeedback(state);
        setTargetState(LaraState::Stop);
        m_falling = false;
        m_horizontalSpeed = 0;
        // @todo Set current state to "Stop"
        return true;
    }

    if( state.axisCollisions == ::LaraState::AxisColl_FrontLeftBump )
    {
        applyCollisionFeedback(state);
        m_rotation.Y += 910;
    }
    else if( state.axisCollisions == ::LaraState::AxisColl_FrontRightBump )
    {
        applyCollisionFeedback(state);
        m_rotation.Y -= 910;
    }

    return false;
}

bool LaraStateHandler::tryStartSlide(::LaraState& state)
{
    auto slantX = std::abs(state.floorSlantX);
    auto slantZ = std::abs(state.floorSlantZ);
    if( slantX <= 2 && slantZ <= 2 )
        return false;

    int targetAngle = util::degToAu(0);
    if( state.floorSlantX < -2 )
        targetAngle = util::degToAu(90);
    else if( state.floorSlantX > 2 )
        targetAngle = util::degToAu(-90);

    if( state.floorSlantZ > std::max(2, slantX) )
        targetAngle = util::degToAu(180);
    else if( state.floorSlantZ < std::min(-2, -slantX) )
        targetAngle = util::degToAu(0);

    auto dy = targetAngle - m_rotation.Y;
    applyCollisionFeedback(state);
    if( dy < util::degToAu(-90) || dy > util::degToAu(90) )
    {
        if( m_dispatcher->getCurrentState() != static_cast<uint16_t>(LaraState::SlideBackward) || targetAngle != m_currentSlideAngle )
        {
            playAnimation(loader::AnimationId::START_SLIDE_BACKWARD, 1677);
            setTargetState(LaraState::SlideBackward);
            m_movementAngle = targetAngle;
            m_currentSlideAngle = targetAngle;
            m_rotation.Y = targetAngle + util::degToAu(180);
        }
    }
    else if( m_dispatcher->getCurrentState() != static_cast<uint16_t>(LaraState::SlideForward) || targetAngle != m_currentSlideAngle )
    {
        playAnimation(loader::AnimationId::SLIDE_FORWARD, 1133);
        setTargetState(LaraState::SlideForward);
        m_movementAngle = targetAngle;
        m_currentSlideAngle = targetAngle;
        m_rotation.Y = targetAngle;
    }
    return true;
}

void LaraStateHandler::handleTriggers(const uint16_t* floorData, bool isDoppelganger)
{
    if( floorData == nullptr )
        return;

    if( loader::extractFDFunction(*floorData) == loader::FDFunction::Death )
    {
        if( !isDoppelganger )
        {
            if( irr::core::equals(m_position.Y, m_floorHeight, 1) )
            {
                //! @todo kill Lara
            }
        }

        if( *floorData & 0x8000 )
            return;

        ++floorData;
    }

    //! @todo Implement the rest
}

void LaraStateHandler::updateFloorHeight(int dy)
{
    loader::TRCoordinates pos = m_position;
    pos.Y += dy;
    auto sector = m_level->findSectorForPosition(pos, m_level->m_camera->getCurrentRoom());
    HeightInfo hi = HeightInfo::fromFloor(sector, pos, m_level->m_camera);
    m_floorHeight = hi.height;

    //! @todo Check room ownership change
}

int LaraStateHandler::getRelativeHeightAtDirection(int16_t angle, int dist) const
{
    auto pos = m_position;
    pos.X += std::sin(util::auToDeg(angle)) * dist;
    pos.Y -= 762; //! @todo MAGICK 762
    pos.Z += std::cos(util::auToDeg(angle)) * dist;

    auto sector = m_level->findSectorForPosition(pos, m_level->m_camera->getCurrentRoom());
    BOOST_ASSERT(sector != nullptr);

    HeightInfo h = HeightInfo::fromFloor(sector, pos, m_level->m_camera);

    if(h.height != -loader::HeightLimit)
        h.height -= m_position.Y;

    return h.height;
}

void LaraStateHandler::commonJumpHandling(::LaraState& state)
{
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 192;
    state.yAngle = m_movementAngle;
    state.initHeightInfo(this, *m_level, 762, m_position); //! @todo MAGICK 762
    checkJumpWallSmash(state);
    if(m_fallSpeed <= 0 || state.current.floor.height > 0)
        return;

    //! @todo if(applyLandingDamage())
    if(false)
        setTargetState(LaraState::Death);
    else
        setTargetState(LaraState::Stop);
    m_fallSpeed = 0;
    m_position.Y += state.current.floor.height;
    m_falling = false;
}
