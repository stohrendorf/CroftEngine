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

constexpr int ScalpHeight = 762;
constexpr int ScalpToHandsHeight = 160;
constexpr int JumpReachableHeight = 896 + loader::SectorSize;

constexpr int FreeFallSpeedThreshold = 131;

enum class SlantClass
{
    None,
    Max512,
    Steep
};

struct HeightInfo
{
    int distance = 0;
    SlantClass slantClass = SlantClass::None;
    const uint16_t* lastTriggerOrKill = nullptr;

    static bool skipSteepSlants;

    static HeightInfo fromFloor(const loader::Sector* roomSector, const loader::TRCoordinates& pos, const TRCameraSceneNodeAnimator* camera)
    {
        HeightInfo hi;

        hi.slantClass = SlantClass::None;

        BOOST_ASSERT(roomSector != nullptr);
        while( roomSector->roomBelow != 0xff )
        {
            BOOST_ASSERT(roomSector->roomBelow < camera->getLevel()->m_rooms.size());
            auto room = &camera->getLevel()->m_rooms[roomSector->roomBelow];
            roomSector = room->getSectorByAbsolutePosition(pos);
        }

        hi.distance = roomSector->floorHeight * loader::QuarterSectorSize;
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
                            hi.distance += static_cast<int>(dist * zSlant * loader::QuarterSectorSize);
                        }
                        else if( zSlant < 0 ) // lower edge at +Z
                        {
                            auto dist = localZ / loader::SectorSize;
                            hi.distance -= static_cast<int>(dist * zSlant * loader::QuarterSectorSize);
                        }

                        if( xSlant > 0 ) // lower edge at -X
                        {
                            auto dist = (loader::SectorSize - localX) / loader::SectorSize;
                            hi.distance += static_cast<int>(dist * xSlant * loader::QuarterSectorSize);
                        }
                        else if( xSlant < 0 ) // lower edge at +X
                        {
                            auto dist = localX / loader::SectorSize;
                            hi.distance -= static_cast<int>(dist * xSlant * loader::QuarterSectorSize);
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
        while( roomSector->roomAbove != 0xff )
        {
            BOOST_ASSERT(roomSector->roomAbove < camera->getLevel()->m_rooms.size());
            auto room = &camera->getLevel()->m_rooms[roomSector->roomAbove];
            roomSector = room->getSectorByAbsolutePosition(pos);
        }

        hi.distance = roomSector->ceilingHeight * loader::QuarterSectorSize;

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
                            hi.distance -= static_cast<int>(dist * zSlant * loader::QuarterSectorSize);
                        }
                        else if( zSlant < 0 ) // lower edge at +Z
                        {
                            auto dist = localZ / loader::SectorSize;
                            hi.distance += static_cast<int>(dist * zSlant * loader::QuarterSectorSize);
                        }

                        if( xSlant > 0 ) // lower edge at -X
                        {
                            auto dist = localX / loader::SectorSize;
                            hi.distance -= static_cast<int>(dist * xSlant * loader::QuarterSectorSize);
                        }
                        else if( xSlant < 0 ) // lower edge at +X
                        {
                            auto dist = (loader::SectorSize - localX) / loader::SectorSize;
                            hi.distance += static_cast<int>(dist * xSlant * loader::QuarterSectorSize);
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

    void init(const loader::Sector* roomSector, const loader::TRCoordinates& position, const TRCameraSceneNodeAnimator* camera, int scalpHeight)
    {
        floor = HeightInfo::fromFloor(roomSector, position, camera);
        if( floor.distance != -loader::HeightLimit )
            floor.distance -= position.Y;

        ceiling = HeightInfo::fromCeiling(roomSector, position, camera);
        if( ceiling.distance != -loader::HeightLimit )
            ceiling.distance -= position.Y - scalpHeight;
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
    loader::ExactTRCoordinates position; // external
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
        const auto reachablePos = pos - loader::TRCoordinates{0, height + ScalpToHandsHeight, 0};
        auto currentSector = level.findSectorForPosition(reachablePos, room);
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
        auto checkPos = loader::TRCoordinates(frontX, 0, frontZ);
        auto sector = level.findSectorForPosition(reachablePos + checkPos, level.m_camera->getCurrentRoom());
        front.init(sector, pos + checkPos, level.m_camera, height);
        if( (frobbelFlags & FrobbelFlag01) != 0 && front.floor.slantClass == SlantClass::Steep && front.floor.distance < 0 )
        {
            front.floor.distance = -32767;
        }
        else if( front.floor.distance > 0
            && (
                ((frobbelFlags & FrobbelFlag02) != 0 && front.floor.slantClass == SlantClass::Steep)
                || ((frobbelFlags & FrobbelFlag04) != 0 && front.floor.lastTriggerOrKill != nullptr && loader::extractFDFunction(*front.floor.lastTriggerOrKill) == loader::FDFunction::Death)
            ) )
        {
            front.floor.distance = 2 * loader::QuarterSectorSize;
        }

        // Front left
        checkPos = loader::TRCoordinates(frontLeftX, 0, frontLeftZ);
        sector = level.findSectorForPosition(reachablePos + checkPos, level.m_camera->getCurrentRoom());
        frontLeft.init(sector, pos + checkPos, level.m_camera, height);

        if( (frobbelFlags & FrobbelFlag01) != 0 && frontLeft.floor.slantClass == SlantClass::Steep && frontLeft.floor.distance < 0 )
        {
            frontLeft.floor.distance = -32767;
        }
        else if( frontLeft.floor.distance > 0
            && (
                ((frobbelFlags & FrobbelFlag02) != 0 && frontLeft.floor.slantClass == SlantClass::Steep)
                || ((frobbelFlags & FrobbelFlag04) != 0 && frontLeft.floor.lastTriggerOrKill != nullptr && loader::extractFDFunction(*frontLeft.floor.lastTriggerOrKill) == loader::FDFunction::Death)
            ) )
        {
            frontLeft.floor.distance = 2 * loader::QuarterSectorSize;
        }

        // Front right
        checkPos = loader::TRCoordinates(frontRightX, 0, frontRightZ);
        sector = level.findSectorForPosition(reachablePos + checkPos, level.m_camera->getCurrentRoom());
        frontRight.init(sector, pos + checkPos, level.m_camera, height);

        if( (frobbelFlags & FrobbelFlag01) != 0 && frontRight.floor.slantClass == SlantClass::Steep && frontRight.floor.distance < 0 )
        {
            frontRight.floor.distance = -32767;
        }
        else if( frontRight.floor.distance > 0
            && (
                ((frobbelFlags & FrobbelFlag02) != 0 && frontRight.floor.slantClass == SlantClass::Steep)
                || ((frobbelFlags & FrobbelFlag04) != 0 && frontRight.floor.lastTriggerOrKill != nullptr && loader::extractFDFunction(*frontRight.floor.lastTriggerOrKill) == loader::FDFunction::Death)
            ) )
        {
            frontRight.floor.distance = 2 * loader::QuarterSectorSize;
        }

        //! @todo check static mesh collisions here

        if( current.floor.distance == -loader::HeightLimit )
        {
            collisionFeedback = position.toInexact() - pos;
            axisCollisions = AxisColl_CannotGoForward;
            return;
        }

        if( current.floor.distance <= current.ceiling.distance )
        {
            axisCollisions = AxisColl_CeilingTooLow;
            collisionFeedback = position.toInexact() - pos;
            return;
        }

        if( current.ceiling.distance >= 0 )
        {
            axisCollisions = AxisColl_HeadInCeiling;
            collisionFeedback.Y = current.ceiling.distance;
        }

        if( front.floor.distance > fruityFloorLimitBottom || front.floor.distance < fruityFloorLimitTop || front.ceiling.distance > fruityCeilingLimit )
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

        if( front.ceiling.distance >= fruityCeilingLimit )
        {
            axisCollisions = AxisColl_BumpHead;
            collisionFeedback = position.toInexact() - pos;
            return;
        }

        if( frontLeft.floor.distance > fruityFloorLimitBottom || frontLeft.floor.distance < fruityFloorLimitTop )
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

        if( frontRight.floor.distance > fruityFloorLimitBottom || frontRight.floor.distance < fruityFloorLimitTop )
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

void LaraStateHandler::setTargetState(loader::LaraStateId st)
{
    m_dispatcher->setTargetState(static_cast<uint16_t>(st));
}

void LaraStateHandler::setStateOverride(loader::LaraStateId st)
{
    m_dispatcher->setStateOverride(static_cast<uint16_t>(st));
}

void LaraStateHandler::clearStateOverride()
{
    m_dispatcher->clearStateOverride();
}


loader::LaraStateId LaraStateHandler::getTargetState() const
{
    return static_cast<LaraStateId>(m_dispatcher->getTargetState());
}

void LaraStateHandler::playAnimation(loader::AnimationId anim, const boost::optional<irr::u32>& firstFrame)
{
    m_dispatcher->playLocalAnimation(static_cast<uint16_t>(anim), firstFrame);
}

void LaraStateHandler::onInput0WalkForward(::LaraState& state)
{
    if( m_health <= 0 )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    if( m_inputState.xMovement == AxisMovement::Left )
        m_yRotationSpeed = std::max(-728, m_yRotationSpeed.get() - 409);
    else if( m_inputState.xMovement == AxisMovement::Right )
        m_yRotationSpeed = std::min(728, m_yRotationSpeed.get() + 409);
    if( m_inputState.zMovement == AxisMovement::Forward )
    {
        if( m_inputState.moveSlow )
            setTargetState(LaraStateId::WalkForward);
        else
            setTargetState(LaraStateId::RunForward);
    }
    else
    {
        setTargetState(LaraStateId::Stop);
    }
}

void LaraStateHandler::onBehave0WalkForward(::LaraState& state)
{
    m_fallSpeed = 0;
    m_falling = false;
    state.yAngle = m_movementAngle = m_rotation.Y;
    state.fruityFloorLimitBottom = ClimbLimit2ClickMin;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 0;
    state.frobbelFlags |= FrobbelFlag01 | FrobbelFlag02 | FrobbelFlag04;
    state.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());

    if( tryStopOnFloor(state) || tryClimb(state) )
        return;

    if( checkWallCollision(state) )
    {
        const auto fr = m_dispatcher->getCurrentFrame();
        if( fr >= 29 && fr <= 47 )
        {
            playAnimation(loader::AnimationId::END_WALK_LEFT, 74);
        }
        else if( (fr >= 22 && fr <= 28) || (fr >= 48 && fr <= 57) )
        {
            playAnimation(loader::AnimationId::END_WALK_RIGHT, 58);
        }
        else
        {
            playAnimation(loader::AnimationId::STAY_SOLID, 185);
        }
    }

    if(state.current.floor.distance > ClimbLimit2ClickMin)
    {
        playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
        setStateOverride(loader::LaraStateId::JumpForward);
        setTargetState(loader::LaraStateId::JumpForward);
        m_fallSpeed = 0;
        m_falling = true;
    }

    if(state.current.floor.distance > SteppableHeight)
    {
        const auto fr = m_dispatcher->getCurrentFrame();
        if(fr < 28 || fr > 45)
        {
            playAnimation(loader::AnimationId::WALK_DOWN_RIGHT, 887);
        }
        else
        {
            playAnimation(loader::AnimationId::WALK_DOWN_LEFT, 874);
        }
    }

    if(state.current.floor.distance >= -ClimbLimit2ClickMin && state.current.floor.distance < -SteppableHeight)
    {
        const auto fr = m_dispatcher->getCurrentFrame();
        if(fr < 27 || fr > 44)
        {
            playAnimation(loader::AnimationId::WALK_UP_STEP_RIGHT, 844);
        }
        else
        {
            playAnimation(loader::AnimationId::WALK_UP_STEP_LEFT, 858);
        }
    }

    if(!tryStartSlide(state))
    {
        m_position.Y += state.current.floor.distance;
    }
}


void LaraStateHandler::onInput1RunForward(::LaraState& state)
{
    if( m_health <= 0 )
    {
        setTargetState(LaraStateId::Death);
        return;
    }

    if( m_inputState.roll )
    {
        playAnimation(loader::AnimationId::ROLL_BEGIN, 3857);
        setTargetState(LaraStateId::Stop);
        setStateOverride(LaraStateId::RollForward);
        return;
    }

    if( m_inputState.xMovement == AxisMovement::Left )
    {
        m_yRotationSpeed = std::max(-1456.f, m_yRotationSpeed.get() - makeSpeedValue(409).getScaledExact(getCurrentDeltaTime()));
        m_rotation.Z = std::max(-2002.f, m_rotation.Z - makeSpeedValue(273).getScaledExact(getCurrentDeltaTime()));
    }
    else if( m_inputState.xMovement == AxisMovement::Right )
    {
        m_yRotationSpeed = std::min(1456.f, m_yRotationSpeed.get() + makeSpeedValue(409).getScaledExact(getCurrentDeltaTime()));
        m_rotation.Z = std::min(2002.f, m_rotation.Z + makeSpeedValue(273).getScaledExact(getCurrentDeltaTime()));
    }

    if( m_inputState.jump && !m_falling )
    {
        setTargetState(LaraStateId::JumpForward);
        return;
    }

    if( m_inputState.zMovement != AxisMovement::Forward )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    if( m_inputState.moveSlow )
        setTargetState(LaraStateId::WalkForward);
    else
        setTargetState(LaraStateId::RunForward);
}

void LaraStateHandler::onInput2Stop(::LaraState& state)
{
    if( m_health <= 0 )
    {
        setTargetState(LaraStateId::Death);
        return;
    }

    if( m_inputState.roll )
    {
        playAnimation(loader::AnimationId::ROLL_BEGIN);
        setTargetState(LaraStateId::Stop);
        setStateOverride(LaraStateId::RollForward);
        return;
    }

    setTargetState(LaraStateId::Stop);

    if( m_inputState.stepMovement == AxisMovement::Left )
    {
        setTargetState(LaraStateId::StepLeft);
    }
    else if( m_inputState.stepMovement == AxisMovement::Right )
    {
        setTargetState(LaraStateId::StepRight);
    }

    if( m_inputState.xMovement == AxisMovement::Left )
    {
        setTargetState(LaraStateId::TurnLeftSlow);
    }
    else if( m_inputState.xMovement == AxisMovement::Right )
    {
        setTargetState(LaraStateId::TurnRightSlow);
    }

    if( m_inputState.jump )
    {
        setTargetState(LaraStateId::JumpPrepare);
    }
    else if( m_inputState.zMovement == AxisMovement::Forward )
    {
        if( m_inputState.moveSlow )
            onInput0WalkForward(state);
        else
            onInput1RunForward(state);
    }
    else if( m_inputState.zMovement == AxisMovement::Backward )
    {
        if( m_inputState.moveSlow )
            onInput16WalkBackward(state);
        else
            setTargetState(LaraStateId::RunBack);
    }
}

void LaraStateHandler::onInput3JumpForward(::LaraState& /*state*/)
{
    if( getTargetState() == LaraStateId::SwandiveBegin || getTargetState() == LaraStateId::Reach )
        setTargetState(LaraStateId::JumpForward);

    if( getTargetState() != LaraStateId::Death && getTargetState() != LaraStateId::Stop )
    {
        if( m_inputState.action && m_handStatus == 0 )
            setTargetState(LaraStateId::Reach);
        if( m_inputState.moveSlow && m_handStatus == 0 )
            setTargetState(LaraStateId::SwandiveBegin);
        if( m_fallSpeed.get() > FreeFallSpeedThreshold)
            setTargetState(LaraStateId::FreeFall);
    }

    if( m_inputState.xMovement == AxisMovement::Left )
    {
        m_yRotationSpeed.subExact(409, getCurrentDeltaTime());
        if(m_yRotationSpeed.get() < -546)
            m_yRotationSpeed = -546;
    }
    else if( m_inputState.xMovement == AxisMovement::Right )
    {
        m_yRotationSpeed.addExact(409, getCurrentDeltaTime());
        if(m_yRotationSpeed.get() > 546)
            m_yRotationSpeed = 546;
    }
}

void LaraStateHandler::onInput5RunBackward(::LaraState& /*state*/)
{
    setTargetState(LaraStateId::Stop);

    if( m_inputState.xMovement == AxisMovement::Left )
        m_yRotationSpeed = std::max(-1092, m_yRotationSpeed.get() - 409);
    else if( m_inputState.xMovement == AxisMovement::Right )
        m_yRotationSpeed = std::min(1092, m_yRotationSpeed.get() + 409);
}

void LaraStateHandler::onInput6TurnRightSlow(::LaraState& /*state*/)
{
    if( m_health <= 0 )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    m_yRotationSpeed.addExact( 409, getCurrentDeltaTime() );

    if( m_handStatus == 4 )
    {
        setTargetState(LaraStateId::TurnFast);
        return;
    }

    if( m_yRotationSpeed.get() > 728 )
    {
        if( m_inputState.moveSlow )
            m_yRotationSpeed = 728;
        else
            setTargetState(LaraStateId::TurnFast);
    }

    if( m_inputState.zMovement != AxisMovement::Forward )
    {
        if( m_inputState.xMovement != AxisMovement::Right )
            setTargetState(LaraStateId::Stop);
        return;
    }

    if( m_inputState.moveSlow )
        setTargetState(LaraStateId::WalkForward);
    else
        setTargetState(LaraStateId::RunForward);
}

void LaraStateHandler::onInput7TurnLeftSlow(::LaraState& /*state*/)
{
    if( m_health <= 0 )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    m_yRotationSpeed.subExact( 409, getCurrentDeltaTime() );

    if( m_handStatus == 4 )
    {
        setTargetState(LaraStateId::TurnFast);
        return;
    }

    if( m_yRotationSpeed.get() < -728 )
    {
        if( m_inputState.moveSlow )
            m_yRotationSpeed = -728;
        else
            setTargetState(LaraStateId::TurnFast);
    }

    if( m_inputState.zMovement != AxisMovement::Forward )
    {
        if( m_inputState.xMovement != AxisMovement::Left )
            setTargetState(LaraStateId::Stop);
        return;
    }

    if( m_inputState.moveSlow )
        setTargetState(LaraStateId::WalkForward);
    else
        setTargetState(LaraStateId::RunForward);
}

void LaraStateHandler::onBehaveTurnSlow(::LaraState& state)
{
    m_fallSpeed = 0;
    m_falling = false;
    m_movementAngle = state.yAngle = m_rotation.Y;
    state.fruityFloorLimitBottom = ClimbLimit2ClickMin;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 0;
    state.frobbelFlags |= FrobbelFlag01 | FrobbelFlag02;
    state.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());

    if(state.current.floor.distance <= 100)
    {
        if(!tryStartSlide(state))
            m_position.Y += state.current.floor.distance;

        return;
    }

    playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
    setTargetState(loader::LaraStateId::JumpForward);
    setStateOverride(loader::LaraStateId::JumpForward);
    m_fallSpeed = 0;
    m_falling = true;
}

void LaraStateHandler::onInput9FreeFall(::LaraState& /*state*/)
{
    m_horizontalSpeed.subExact( m_horizontalSpeed.getExact() * 5 / 100, getCurrentDeltaTime() );
    if( m_fallSpeed.get() > 154 )
    {
        //! @todo playSound(30)
    }
}

void LaraStateHandler::onInput15JumpPrepare(::LaraState& /*state*/)
{
    if( m_inputState.zMovement == AxisMovement::Forward && getRelativeHeightAtDirection(m_rotation.Y, 256) >= -384 )
    {
        m_movementAngle = m_rotation.Y;
        setTargetState(LaraStateId::JumpForward);
    }
    else if( m_inputState.xMovement == AxisMovement::Left && getRelativeHeightAtDirection(m_rotation.Y - util::degToAu(90), 256) >= -384 )
    {
        m_movementAngle = m_rotation.Y - util::degToAu(90);
        setTargetState(LaraStateId::JumpRight);
    }
    else if( m_inputState.xMovement == AxisMovement::Right && getRelativeHeightAtDirection(m_rotation.Y + util::degToAu(90), 256) >= -384 )
    {
        m_movementAngle = m_rotation.Y + util::degToAu(90);
        setTargetState(LaraStateId::JumpLeft);
    }
    else if( m_inputState.zMovement == AxisMovement::Backward && getRelativeHeightAtDirection(m_rotation.Y + util::degToAu(180), 256) >= -384 )
    {
        m_movementAngle = m_rotation.Y + util::degToAu(180);
        setTargetState(LaraStateId::JumpBack);
    }

    if( m_fallSpeed.get() > FreeFallSpeedThreshold)
    {
        setTargetState(LaraStateId::FreeFall);
    }
}

void LaraStateHandler::onInput16WalkBackward(::LaraState& /*state*/)
{
    if( m_health <= 0 )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    if( m_inputState.zMovement == AxisMovement::Backward && m_inputState.moveSlow )
        setTargetState(LaraStateId::WalkBackward);
    else
        setTargetState(LaraStateId::Stop);

    if( m_inputState.xMovement == AxisMovement::Left )
        m_yRotationSpeed = std::max(-728, m_yRotationSpeed.get() - 409);
    else if( m_inputState.xMovement == AxisMovement::Right )
        m_yRotationSpeed = std::min(728, m_yRotationSpeed.get() + 409);
}

void LaraStateHandler::onInput19Climbing(::LaraState& state)
{
    state.frobbelFlags &= ~(FrobbelFlag08 | FrobbelFlag10);
}

void LaraStateHandler::onInput20TurnFast(::LaraState& /*state*/)
{
    if( m_health <= 0 )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    if( m_yRotationSpeed.get() >= 0 )
    {
        m_yRotationSpeed = 1456;
        if( m_inputState.xMovement == AxisMovement::Right )
            return;
    }
    else
    {
        m_yRotationSpeed = -1456;
        if( m_inputState.xMovement == AxisMovement::Left )
            return;
    }
    setTargetState(LaraStateId::Stop);
}

void LaraStateHandler::onInput24SlideForward(::LaraState& /*state*/)
{
    if( m_inputState.jump )
        setTargetState(LaraStateId::JumpForward);
}

void LaraStateHandler::onBehave24SlideForward(::LaraState& state)
{
    m_movementAngle = m_rotation.Y;
    commonSlideHandling(state);
}

void LaraStateHandler::onInput25JumpBackward(::LaraState& /*state*/)
{
    //! @todo Set local camera Y rotation to 24570 AU
    if( m_fallSpeed.get() > FreeFallSpeedThreshold)
        setTargetState(LaraStateId::FreeFall);
}

void LaraStateHandler::onInput28JumpUp(::LaraState& /*state*/)
{
    if( m_fallSpeed.get() > FreeFallSpeedThreshold)
        setTargetState(LaraStateId::FreeFall);
}

void LaraStateHandler::onInput29FallBackward(::LaraState& /*state*/)
{
    if( m_fallSpeed.get() > FreeFallSpeedThreshold)
        setTargetState(LaraStateId::FreeFall);

    if( m_inputState.action && m_handStatus == 0 )
        setTargetState(LaraStateId::Reach);
}

void LaraStateHandler::onBehave29FallBackward(::LaraState& state)
{
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 192;
    state.yAngle = m_rotation.Y + util::degToAu(180);
    state.initHeightInfo(this, *m_level, 870, m_position.toInexact()); //! @todo MAGICK 870
    checkJumpWallSmash(state);
    if( state.current.floor.distance > 0 || m_fallSpeed.get() <= 0 )
        return;

    if( applyLandingDamage(state) )
        setTargetState(LaraStateId::Death);
    else
        setTargetState(LaraStateId::Stop);

    m_fallSpeed = 0;
    m_position.Y += state.current.floor.distance;
    m_falling = false;
}

void LaraStateHandler::onInput32SlideBackward(::LaraState& /*state*/)
{
    if( m_inputState.jump )
        setTargetState(LaraStateId::JumpBack);
}

void LaraStateHandler::handleLaraStateOnLand()
{
    //! @todo Only when on solid ground
    m_air = 1800;

    ::LaraState laraState;
    laraState.position = m_position;
    laraState.collisionRadius = 100; //!< @todo MAGICK 100
    laraState.frobbelFlags = FrobbelFlag10 | FrobbelFlag08;

    static std::array<Handler, 56> inputHandlers{{
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
        &LaraStateHandler::nopHandler,
        nullptr,
        &LaraStateHandler::nopHandler,
        &LaraStateHandler::onInput15JumpPrepare,
        &LaraStateHandler::onInput16WalkBackward,
        nullptr,
        nullptr,
        &LaraStateHandler::onInput19Climbing,
        &LaraStateHandler::onInput20TurnFast,
        nullptr,nullptr,
        &LaraStateHandler::nopHandler,
        &LaraStateHandler::onInput24SlideForward,
        &LaraStateHandler::onInput25JumpBackward,
        &LaraStateHandler::onInput26JumpLeft,
        &LaraStateHandler::onInput27JumpRight,
        &LaraStateHandler::onInput28JumpUp,
        &LaraStateHandler::onInput29FallBackward,
        // 30
        nullptr,nullptr,
        &LaraStateHandler::onInput32SlideBackward,
        nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
        // 40
        nullptr,nullptr,nullptr,nullptr,nullptr,
        &LaraStateHandler::nopHandler,
        nullptr,nullptr,nullptr,nullptr,
        // 50
        nullptr,nullptr,
        &LaraStateHandler::onInput52SwandiveBegin,
        &LaraStateHandler::onInput53SwandiveEnd,
        nullptr,nullptr
    }};

    const auto currentState = m_dispatcher->getCurrentState();

    // BOOST_LOG_TRIVIAL(debug) << "INPUT State=" << currentState << ", pos = (" << m_position.X << "/" << m_position.Y << "/" << m_position.Z << ")";

    if( currentState >= inputHandlers.size() )
    {
        BOOST_LOG_TRIVIAL(error) << "Unexpected state " << currentState;
        return;
    }

    if( !inputHandlers[currentState] )
    BOOST_LOG_TRIVIAL(warning) << "No input handler for state " << currentState;
    else
        (this ->* inputHandlers[currentState])(laraState);

    // "slowly" revert rotations to zero
    if(m_rotation.Z < 0)
    {
        m_rotation.Z += makeSpeedValue(182).getScaledExact(getCurrentDeltaTime());
        if(m_rotation.Z >= 0)
            m_rotation.Z = 0;
    }
    else if(m_rotation.Z > 0)
    {
        m_rotation.Z -= makeSpeedValue(182).getScaledExact(getCurrentDeltaTime());
        if(m_rotation.Z <= 0)
            m_rotation.Z = 0;
    }

    if(m_yRotationSpeed.get() < 0)
    {
        m_yRotationSpeed.addExact(364, getCurrentDeltaTime());
        if(m_yRotationSpeed.get() >= 0)
            m_yRotationSpeed = 0;
    }
    else if(m_yRotationSpeed.get() > 0)
    {
        m_yRotationSpeed.subExact(364, getCurrentDeltaTime());
        if(m_yRotationSpeed.get() <= 0)
            m_yRotationSpeed = 0;
    }

    m_rotation.Y += m_yRotationSpeed.getScaledExact(getCurrentDeltaTime());

    processAnimCommands();

    // @todo test interactions?

    // BOOST_LOG_TRIVIAL(debug) << "BEHAVE State=" << m_dispatcher->getCurrentState() << ", pos = (" << m_position.X << "/" << m_position.Y << "/" << m_position.Z << ")";

    switch( static_cast<LaraStateId>( m_dispatcher->getCurrentState() ) )
    {
    case LaraStateId::Stop:
    case LaraStateId::Pose:
    case LaraStateId::GrabToFall:
    case LaraStateId::TurnFast:
        m_fallSpeed = 0;
        m_falling = false;
        laraState.yAngle = m_rotation.Y;
        m_movementAngle = m_rotation.Y;
        laraState.fruityFloorLimitTop = -ClimbLimit2ClickMin;
        laraState.fruityFloorLimitBottom = ClimbLimit2ClickMin;
        laraState.fruityCeilingLimit = 0;
        laraState.frobbelFlags |= FrobbelFlag01 | FrobbelFlag02;
        laraState.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());
        if( tryStopOnFloor(laraState) )
            break;
        if( laraState.current.floor.distance <= 100 )
        {
            if( !tryStartSlide(laraState) )
            {
                applyCollisionFeedback(laraState);
                m_position.Y += laraState.current.floor.distance;
            }
        }
        else
        {
            playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
            setTargetState(loader::LaraStateId::JumpForward);
            m_fallSpeed = 0;
            m_falling = true;
        }
        break;
    case LaraStateId::WalkForward:
        onBehave0WalkForward(laraState);
        break;
    case LaraStateId::RunForward:
        onBehave1RunForward(laraState);
        break;
    case LaraStateId::RunBack:
        onBehave5RunBackward(laraState);
        break;
    case LaraStateId::WalkBackward:
        onBehave16WalkBackward(laraState);
        break;
    case LaraStateId::JumpBack:
        m_movementAngle = m_rotation.Y + util::degToAu(180);
        commonJumpHandling(laraState);
        break;
    case LaraStateId::JumpLeft:
        m_movementAngle = m_rotation.Y + util::degToAu(90);
        commonJumpHandling(laraState);
        break;
    case LaraStateId::JumpRight:
        m_movementAngle = m_rotation.Y - util::degToAu(90);
        commonJumpHandling(laraState);
        break;
    case LaraStateId::FreeFall:
        onBehave9FreeFall(laraState);
        break;
    case LaraStateId::JumpUp:
        onBehave28JumpUp(laraState);
        break;
    case LaraStateId::JumpForward:
        onBehave3JumpForward(laraState);
        break;
    case LaraStateId::JumpPrepare:
        onBehave15JumpPrepare(laraState);
        break;
    case LaraStateId::Climbing:
        onBehave19Climbing(laraState);
        break;
    case LaraStateId::SlideBackward:
        onBehave32SlideBackward(laraState);
        break;
    case LaraStateId::FallBackward:
        onBehave29FallBackward(laraState);
        break;
    case LaraStateId::SlideForward:
        onBehave24SlideForward(laraState);
        break;
    case LaraStateId::Unknown12:
        onBehave12Unknown(laraState);
        break;
    case LaraStateId::TurnLeftSlow:
    case LaraStateId::TurnRightSlow:
        onBehaveTurnSlow(laraState);
        break;
    case LaraStateId::SwandiveBegin:
        onBehave52SwandiveBegin(laraState);
        break;
    case LaraStateId::SwandiveEnd:
        onBehave53SwandiveEnd(laraState);
        break;
    case LaraStateId::RollForward:
        onBehave45RollForward(laraState);
        break;
    case LaraStateId::RollBackward:
        onBehave23RollBackward(laraState);
        break;
    default:
        BOOST_LOG_TRIVIAL(warning) << "No behaviour handler for state " << currentState;
        break;
    }

    updateFloorHeight(-381);
    handleTriggers(laraState.current.floor.lastTriggerOrKill, false);
}

void LaraStateHandler::onBehave19Climbing(::LaraState& state)
{
    state.fruityFloorLimitBottom = ClimbLimit2ClickMin;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 0;
    state.yAngle = m_movementAngle = m_rotation.Y;
    state.frobbelFlags |= FrobbelFlag01 | FrobbelFlag02;
    state.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());
}

void LaraStateHandler::onBehave15JumpPrepare(::LaraState& state)
{
    m_fallSpeed = 0;
    m_falling = false;
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -loader::HeightLimit;
    state.fruityCeilingLimit = 0;
    state.yAngle = m_rotation.Y;
    state.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());

    if( state.current.ceiling.distance <= -100 )
        return;

    setTargetState(LaraStateId::Stop);
    setStateOverride(LaraStateId::Stop);
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
    state.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());
    if( tryStopOnFloor(state) || tryClimb(state) )
        return;

    if( checkWallCollision(state) )
    {
        m_rotation.Z = 0;
        if( state.front.floor.slantClass == SlantClass::None && state.front.floor.distance < -ClimbLimit2ClickMax )
        {
            setStateOverride(LaraStateId::Unknown12);
            if( m_dispatcher->getCurrentFrame() >= 0 && m_dispatcher->getCurrentFrame() <= 9 )
            {
                playAnimation(loader::AnimationId::WALL_SMASH_LEFT, 800);
                return;
            }
            if( m_dispatcher->getCurrentFrame() >= 10 && m_dispatcher->getCurrentFrame() <= 21 )
            {
                playAnimation(loader::AnimationId::WALL_SMASH_RIGHT, 815);
                return;
            }

            playAnimation(loader::AnimationId::STAY_SOLID, 185);
        }
    }

    if( state.current.floor.distance > ClimbLimit2ClickMin )
    {
        playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
        setStateOverride(LaraStateId::JumpForward);
        setTargetState(LaraStateId::JumpForward);
        m_falling = true;
        m_fallSpeed = 0;
        return;
    }

    if( state.current.floor.distance >= -ClimbLimit2ClickMin && state.current.floor.distance < -SteppableHeight )
    {
        if( m_dispatcher->getCurrentFrame() >= 3 && m_dispatcher->getCurrentFrame() <= 14 )
        {
            playAnimation(loader::AnimationId::RUN_UP_STEP_LEFT, 837);
        }
        else
        {
            playAnimation(loader::AnimationId::RUN_UP_STEP_RIGHT, 830);
        }
    }

    if( !tryStartSlide(state) )
    {
        m_position.Y += std::min(50, state.current.floor.distance);
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
    state.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());
    if( tryStopOnFloor(state) )
        return;

    if( state.current.floor.distance > 200 )
    {
        playAnimation(loader::AnimationId::FREE_FALL_BACK, 1473);
        setTargetState(loader::LaraStateId::FallBackward);
        setStateOverride(LaraStateId::FallBackward);
        m_fallSpeed = 0;
        m_falling = true;
        return;
    }

    if( checkWallCollision(state) )
    {
        playAnimation(loader::AnimationId::STAY_SOLID, 185);
    }
    m_position.Y += state.current.floor.distance;
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
    state.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());
    if( tryStopOnFloor(state) )
        return;

    if( checkWallCollision(state) )
    {
        playAnimation(loader::AnimationId::STAY_SOLID, 185);
    }

    if( state.current.floor.distance > 128 && state.current.floor.distance < 384 )
    {
        if( m_dispatcher->getCurrentFrame() < 964 || m_dispatcher->getCurrentFrame() > 993 )
        {
            playAnimation(loader::AnimationId::WALK_DOWN_BACK_LEFT, 899);
        }
        else
        {
            playAnimation(loader::AnimationId::WALK_DOWN_BACK_RIGHT, 930);
        }
    }

    if( !tryStartSlide(state) )
    {
        m_position.Y += state.current.floor.distance;
    }
}

void LaraStateHandler::onBehave9FreeFall(::LaraState& state)
{
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 192;
    state.yAngle = m_movementAngle;
    m_falling = true;
    state.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());
    jumpAgainstWall(state);
    if( state.current.floor.distance > 0 )
        return;

    if( applyLandingDamage(state) )
    {
        setTargetState(loader::LaraStateId::Death);
    }
    else
    {
        setTargetState(LaraStateId::Stop);
        setStateOverride(LaraStateId::Stop);
        playAnimation(loader::AnimationId::LANDING_HARD, 358);
    }
    m_fallSpeed = 0;
    m_position.Y += state.current.floor.distance;
    m_falling = false;
}

void LaraStateHandler::onBehave12Unknown(::LaraState& state)
{
    state.fruityFloorLimitBottom = ClimbLimit2ClickMin;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 0;
    state.yAngle = m_movementAngle;
    state.frobbelFlags |= FrobbelFlag01 | FrobbelFlag02;
    state.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());
    applyCollisionFeedback(state);
}

void LaraStateHandler::onBehave3JumpForward(::LaraState& state)
{
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 192;
    m_movementAngle = state.yAngle = m_rotation.Y;
    state.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());
    checkJumpWallSmash(state);

    if( state.current.floor.distance > 0 || m_fallSpeed.get() <= 0 )
        return;

    if( applyLandingDamage(state) )
    {
        setTargetState(LaraStateId::Death);
    }
    else if( m_inputState.zMovement != AxisMovement::Forward || m_inputState.moveSlow )
    {
        setTargetState(LaraStateId::Stop);
    }
    else
    {
        setTargetState(LaraStateId::RunForward);
    }

    m_fallSpeed = 0;
    m_falling = false;
    m_horizontalSpeed = 0;
    m_position.Y += state.current.floor.distance;
    processAnimCommands();
}

void LaraStateHandler::onBehave28JumpUp(::LaraState& state)
{
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 192;
    state.yAngle = m_rotation.Y;
    state.initHeightInfo(this, *m_level, 870, m_position.toInexact()); //! @todo MAGICK 870
    if( tryGrabEdge(state) )
        return;

    jumpAgainstWall(state);
    if( m_fallSpeed.get() <= 0 || state.current.floor.distance > 0 )
        return;

    if( applyLandingDamage(state) )
        setTargetState(loader::LaraStateId::Death);
    else
        setTargetState(loader::LaraStateId::Stop);
    m_fallSpeed = 0;
    m_position.Y += state.current.floor.distance;
    m_falling = false;
}

void LaraStateHandler::onBehave32SlideBackward(::LaraState& state)
{
    m_movementAngle = m_rotation.Y + util::degToAu(180);
    commonSlideHandling(state);
}

void LaraStateHandler::jumpAgainstWall(::LaraState& state)
{
    applyCollisionFeedback(state);
    if( state.axisCollisions == ::LaraState::AxisColl_FrontLeftBump )
        m_rotation.Y += makeSpeedValue(910).getScaledExact(getCurrentDeltaTime());
    else if( state.axisCollisions == ::LaraState::AxisColl_FrontRightBump )
        m_rotation.Y -= makeSpeedValue(940).getScaledExact(getCurrentDeltaTime());
    else if( state.axisCollisions == ::LaraState::AxisColl_HeadInCeiling )
    {
        if( m_fallSpeed.get() <= 0 )
            m_fallSpeed = 1;
    }
    else if( state.axisCollisions == ::LaraState::AxisColl_CeilingTooLow )
    {
        m_position.X += makeSpeedValue(100).getScaledExact(getCurrentDeltaTime()) * std::sin(util::auToRad(m_rotation.Y));
        m_position.Z += makeSpeedValue(100).getScaledExact(getCurrentDeltaTime()) * std::cos(util::auToRad(m_rotation.Y));
        m_horizontalSpeed = 0;
        state.current.floor.distance = 0;
        if( m_fallSpeed.get() < 0 )
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
        setTargetState(LaraStateId::FreeFall);
        //! @todo Check formula
        //! @bug Time is too short to properly apply collision momentum!
        m_horizontalSpeed.subExact( m_horizontalSpeed.getExact() * 4 / 5, getCurrentDeltaTime() );
        m_movementAngle -= util::degToAu(180);
        playAnimation(loader::AnimationId::SMASH_JUMP, 481);
        setStateOverride(LaraStateId::FreeFall);
        if( m_fallSpeed.get() <= 0 )
            m_fallSpeed = 1;
        return;
    }

    if( state.axisCollisions == ::LaraState::AxisColl_FrontLeftBump )
    {
        m_rotation.Y += makeSpeedValue(910).getScaledExact(getCurrentDeltaTime());
        return;
    }
    else if( state.axisCollisions == ::LaraState::AxisColl_FrontRightBump )
    {
        m_rotation.Y -= makeSpeedValue(910).getScaledExact(getCurrentDeltaTime());
        return;
    }

    if( state.axisCollisions == ::LaraState::AxisColl_CeilingTooLow )
    {
        m_position.X -= makeSpeedValue(100).getScaledExact(getCurrentDeltaTime()) * std::sin(util::auToRad(state.yAngle));
        m_position.Z -= makeSpeedValue(100).getScaledExact(getCurrentDeltaTime()) * std::cos(util::auToRad(state.yAngle));
        m_horizontalSpeed = 0;
        state.current.floor.distance = 0;
        if( m_fallSpeed.get() <= 0 )
            m_fallSpeed = 16;
    }

    if( state.axisCollisions == ::LaraState::AxisColl_HeadInCeiling && m_fallSpeed.get() <= 0 )
        m_fallSpeed = 1;
}

void LaraStateHandler::animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs)
{
    BOOST_ASSERT(m_lara == node);

    if(m_lastFrameTime < 0)
        m_lastFrameTime = m_currentFrameTime = timeMs;

    if( m_lastFrameTime == timeMs )
        return;

    m_currentFrameTime = timeMs;

    handleLaraStateOnLand();

    m_lastFrameTime = m_currentFrameTime;
}

void LaraStateHandler::processAnimCommands()
{
    clearStateOverride();
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
                m_position += {makeSpeedValue(cmd[0]).getScaledExact(getCurrentDeltaTime()), makeSpeedValue(cmd[1]).getScaledExact(getCurrentDeltaTime()), makeSpeedValue(cmd[2]).getScaledExact(getCurrentDeltaTime())};
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
                    BOOST_LOG_TRIVIAL(debug) << "Anim effect: " << int(cmd[1]);
                    if(cmd[1] == 0)
                        m_rotation.Y += util::degToAu(180);
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
        m_horizontalSpeed.addExact(m_dispatcher->getAccelleration(), getCurrentDeltaTime());
        if( m_fallSpeed.get() >= 128 )
            m_fallSpeed.addExact(1, getCurrentDeltaTime());
        else
            m_fallSpeed.addExact(6, getCurrentDeltaTime());
    }
    else
    {
        m_horizontalSpeed = m_dispatcher->calculateFloorSpeed();
    }

    m_position.X += std::sin(util::auToRad(m_movementAngle)) * m_horizontalSpeed.getScaledExact(getCurrentDeltaTime());
    m_position.Y += m_fallSpeed.getScaledExact(getCurrentDeltaTime());
    m_position.Z += std::cos(util::auToRad(m_movementAngle)) * m_horizontalSpeed.getScaledExact(getCurrentDeltaTime());
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

    setTargetState(LaraStateId::Stop);
    setStateOverride(LaraStateId::Stop);
    playAnimation(loader::AnimationId::STAY_SOLID, 185);
    m_horizontalSpeed = 0;
    m_fallSpeed = 0;
    m_falling = false;
    return true;
}

bool LaraStateHandler::tryClimb(::LaraState& state)
{
    if( state.axisCollisions != ::LaraState::AxisColl_CannotGoForward || !m_inputState.jump || m_handStatus != 0 )
        return false;

    const auto floorGradient = std::abs(state.frontLeft.floor.distance - state.frontRight.floor.distance);
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

    const auto climbHeight = state.front.floor.distance;
    if( climbHeight >= -ClimbLimit2ClickMax && climbHeight <= -ClimbLimit2ClickMin )
    {
        if( climbHeight < state.front.ceiling.distance
            || state.frontLeft.floor.distance < state.frontLeft.ceiling.distance
            || state.frontRight.floor.distance < state.frontRight.ceiling.distance )
            return false;

        setTargetState(LaraStateId::Stop);
        setStateOverride(LaraStateId::Climbing);
        playAnimation(loader::AnimationId::CLIMB_2CLICK, 759);
        m_position.Y += 2 * loader::QuarterSectorSize + climbHeight;
        //m_handStatus = 1;
    }
    else if( climbHeight >= -ClimbLimit3ClickMax && climbHeight <= -ClimbLimit2ClickMax )
    {
        if( state.front.floor.distance < state.front.ceiling.distance
            || state.frontLeft.floor.distance < state.frontLeft.ceiling.distance
            || state.frontRight.floor.distance < state.frontRight.ceiling.distance )
            return false;

        setTargetState(LaraStateId::Stop);
        setStateOverride(LaraStateId::Climbing);
        playAnimation(loader::AnimationId::CLIMB_3CLICK, 614);
        m_position.Y += 3 * loader::QuarterSectorSize + climbHeight;
        //m_handStatus = 1;
    }
    else
    {
        if( climbHeight < -JumpReachableHeight || climbHeight > -ClimbLimit3ClickMax )
            return false;

        setTargetState(LaraStateId::JumpUp);
        setStateOverride(LaraStateId::Stop);
        playAnimation(loader::AnimationId::STAY_SOLID, 185);
        m_fallSpeedOverride = -static_cast<int>(std::sqrt(-12 * (climbHeight + 800) + 3));
        processAnimCommands();
    }

    m_rotation.Y = alignedRotation;
    applyCollisionFeedback(state);

    return true;
}

void LaraStateHandler::applyCollisionFeedback(::LaraState& state)
{
    m_position += loader::ExactTRCoordinates(state.collisionFeedback);
    state.collisionFeedback = {0,0,0};
}

bool LaraStateHandler::checkWallCollision(::LaraState& state)
{
    if( state.axisCollisions == ::LaraState::AxisColl_CannotGoForward || state.axisCollisions == ::LaraState::AxisColl_BumpHead )
    {
        applyCollisionFeedback(state);
        setTargetState(LaraStateId::Stop);
        setStateOverride(LaraStateId::Stop);
        m_falling = false;
        m_horizontalSpeed = 0;
        return true;
    }

    if( state.axisCollisions == ::LaraState::AxisColl_FrontLeftBump )
    {
        applyCollisionFeedback(state);
        m_rotation.Y += makeSpeedValue(910).getScaledExact(getCurrentDeltaTime());
    }
    else if( state.axisCollisions == ::LaraState::AxisColl_FrontRightBump )
    {
        applyCollisionFeedback(state);
        m_rotation.Y -= makeSpeedValue(910).getScaledExact(getCurrentDeltaTime());
    }

    return false;
}

bool LaraStateHandler::tryStartSlide(::LaraState& state)
{
    auto slantX = std::abs(state.floorSlantX);
    auto slantZ = std::abs(state.floorSlantZ);
    if( slantX <= 2 && slantZ <= 2 )
        return false;

    auto targetAngle = util::degToAu(0);
    if( state.floorSlantX < -2 )
        targetAngle = util::degToAu(90);
    else if( state.floorSlantX > 2 )
        targetAngle = util::degToAu(-90);

    if( state.floorSlantZ > std::max(2, slantX) )
        targetAngle = util::degToAu(180);
    else if( state.floorSlantZ < std::min(-2, -slantX) )
        targetAngle = util::degToAu(0);

    auto dy = std::abs(targetAngle - m_rotation.Y);
    applyCollisionFeedback(state);
    if( dy > util::degToAu(90) )
    {
        if( m_dispatcher->getCurrentState() != static_cast<uint16_t>(LaraStateId::SlideBackward) || targetAngle != m_currentSlideAngle )
        {
            playAnimation(loader::AnimationId::START_SLIDE_BACKWARD, 1677);
            setTargetState(LaraStateId::SlideBackward);
            setStateOverride(LaraStateId::SlideBackward);
            m_movementAngle = targetAngle;
            m_currentSlideAngle = targetAngle;
            m_rotation.Y = targetAngle + util::degToAu(180);
        }
    }
    else if( m_dispatcher->getCurrentState() != static_cast<uint16_t>(LaraStateId::SlideForward) || targetAngle != m_currentSlideAngle )
    {
        playAnimation(loader::AnimationId::SLIDE_FORWARD, 1133);
        setTargetState(LaraStateId::SlideForward);
        setStateOverride(LaraStateId::SlideForward);
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
            if( irr::core::equals(m_position.Y, float(m_floorHeight), 1.f) )
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
    loader::TRCoordinates pos = m_position.toInexact();
    pos.Y += dy;
    auto sector = m_level->findSectorForPosition(pos, m_level->m_camera->getCurrentRoom());
    HeightInfo hi = HeightInfo::fromFloor(sector, pos, m_level->m_camera);
    m_floorHeight = hi.distance;

    //! @todo Check room ownership change
}

int LaraStateHandler::getRelativeHeightAtDirection(int16_t angle, int dist) const
{
    auto pos = m_position;
    pos.X += std::sin(util::auToDeg(angle)) * dist;
    pos.Y -= ScalpHeight;
    pos.Z += std::cos(util::auToDeg(angle)) * dist;

    auto sector = m_level->findSectorForPosition(pos.toInexact(), m_level->m_camera->getCurrentRoom());
    BOOST_ASSERT(sector != nullptr);

    HeightInfo h = HeightInfo::fromFloor(sector, pos.toInexact(), m_level->m_camera);

    if( h.distance != -loader::HeightLimit )
        h.distance -= m_position.Y;

    return h.distance;
}

void LaraStateHandler::commonJumpHandling(::LaraState& state)
{
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 192;
    state.yAngle = m_movementAngle;
    state.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());
    checkJumpWallSmash(state);
    if( m_fallSpeed.get() <= 0 || state.current.floor.distance > 0 )
        return;

    if( applyLandingDamage(state) )
        setTargetState(LaraStateId::Death);
    else
        setTargetState(LaraStateId::Stop);
    m_fallSpeed = 0;
    m_position.Y += state.current.floor.distance;
    m_falling = false;
}

void LaraStateHandler::commonSlideHandling(::LaraState& state)
{
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -loader::QuarterSectorSize * 2;
    state.fruityCeilingLimit = 0;
    state.yAngle = m_movementAngle;
    state.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());

    if( tryStopOnFloor(state) )
        return;

    checkWallCollision(state);
    if( state.current.floor.distance <= 200 )
    {
        tryStartSlide(state);
        m_position.Y += state.current.floor.distance;
        const auto absSlantX = std::abs(state.floorSlantX);
        const auto absSlantZ = std::abs(state.floorSlantZ);
        if( absSlantX <= 2 && absSlantZ <= 2 )
        {
            setTargetState(LaraStateId::Stop);
        }
    }
    else
    {
        if( static_cast<LaraStateId>(m_dispatcher->getCurrentState()) == LaraStateId::SlideForward )
        {
            playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
            setTargetState(LaraStateId::JumpForward);
            setStateOverride(LaraStateId::JumpForward);
        }
        else
        {
            playAnimation(loader::AnimationId::FREE_FALL_BACK, 1473);
            setTargetState(LaraStateId::FallBackward);
            setStateOverride(LaraStateId::FallBackward);
        }

        m_fallSpeed = 0;
        m_falling = true;
    }
}

bool LaraStateHandler::applyLandingDamage(::LaraState& /*state*/)
{
    auto sector = m_level->findSectorForPosition(m_position.toInexact(), m_level->m_camera->getCurrentRoom());
    HeightInfo h = HeightInfo::fromFloor(sector, m_position.toInexact() - loader::TRCoordinates{0, ScalpHeight, 0}, m_level->m_camera);
    m_floorHeight = h.distance;
    handleTriggers(h.lastTriggerOrKill, false);
    auto damageSpeed = m_fallSpeed.get() - 140;
    if( damageSpeed <= 0 )
        return false;

    static constexpr int DeathSpeedLimit = 14;

    if( damageSpeed <= DeathSpeedLimit )
        m_health -= 1000 * damageSpeed * damageSpeed / (DeathSpeedLimit * DeathSpeedLimit);
    else
        m_health = -1;
    return m_health <= 0;
}

void LaraStateHandler::onInput52SwandiveBegin(::LaraState& state)
{
    state.frobbelFlags &= ~FrobbelFlag10;
    state.frobbelFlags |= FrobbelFlag08;
    if(m_fallSpeed.get() > FreeFallSpeedThreshold)
        setTargetState(loader::LaraStateId::SwandiveEnd);
}

void LaraStateHandler::onBehave52SwandiveBegin(::LaraState& state)
{
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 192;
    m_movementAngle = state.yAngle = m_rotation.Y;
    state.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());
    checkJumpWallSmash(state);
    if(state.current.floor.distance > 0 || m_fallSpeed.get() <= 0)
        return;

    setTargetState(loader::LaraStateId::Stop);
    m_fallSpeed = 0;
    m_falling = false;
    m_position.Y += state.current.floor.distance;
}

void LaraStateHandler::onInput53SwandiveEnd(::LaraState& state)
{
    state.frobbelFlags &= ~FrobbelFlag10;
    state.frobbelFlags |= FrobbelFlag08;
    m_horizontalSpeed.subExact(m_horizontalSpeed.getExact() * 5 / 100, getCurrentDeltaTime());
}

void LaraStateHandler::onBehave53SwandiveEnd(::LaraState& state)
{
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 192;
    m_movementAngle = state.yAngle = m_rotation.Y;
    state.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());
    checkJumpWallSmash(state);
    if(state.current.floor.distance > 0 || m_fallSpeed.get() <= 0)
        return;

    if(m_fallSpeed.get() <= 133)
        setTargetState(loader::LaraStateId::Stop);
    else
        setTargetState(loader::LaraStateId::Death);

    m_fallSpeed = 0;
    m_falling = false;
    m_position.Y += state.current.floor.distance;
}

void LaraStateHandler::onBehave23RollBackward(::LaraState& state)
{
    m_falling = false;
    m_fallSpeed = 0;
    m_movementAngle = state.yAngle = m_rotation.Y + util::degToAu(180);
    state.frobbelFlags |= FrobbelFlag01;
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 0;
    state.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());

    if(tryStopOnFloor(state) || tryStartSlide(state))
        return;

    if(state.current.floor.distance <= 200)
    {
        applyCollisionFeedback(state);
        m_position.Y += state.current.floor.distance;
        return;
    }

    playAnimation(loader::AnimationId::FREE_FALL_BACK, 1473);
    setTargetState(loader::LaraStateId::FallBackward);
    setStateOverride(loader::LaraStateId::FallBackward);
    m_fallSpeed = 0;
    m_falling = true;
}

void LaraStateHandler::onBehave45RollForward(::LaraState& state)
{
    m_falling = false;
    m_fallSpeed = 0;
    m_movementAngle = state.yAngle = m_rotation.Y;
    state.frobbelFlags |= FrobbelFlag01;
    state.fruityFloorLimitBottom = loader::HeightLimit;
    state.fruityFloorLimitTop = -ClimbLimit2ClickMin;
    state.fruityCeilingLimit = 0;
    state.initHeightInfo(this, *m_level, ScalpHeight, m_position.toInexact());

    if(tryStopOnFloor(state) || tryStartSlide(state))
        return;

    if(state.current.floor.distance <= 200)
    {
        applyCollisionFeedback(state);
        m_position.Y += state.current.floor.distance;
        return;
    }

    playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
    setTargetState(loader::LaraStateId::JumpForward);
    setStateOverride(loader::LaraStateId::JumpForward);
    m_fallSpeed = 0;
    m_falling = true;
}

void LaraStateHandler::onInput26JumpLeft(::LaraState& state)
{
    if(m_fallSpeed.get() > FreeFallSpeedThreshold)
        setTargetState(loader::LaraStateId::FreeFall);
}

void LaraStateHandler::onInput27JumpRight(::LaraState& state)
{
    if(m_fallSpeed.get() > FreeFallSpeedThreshold)
        setTargetState(loader::LaraStateId::FreeFall);
}
