#include "larastatehandler.h"

#include "defaultanimdispatcher.h"
#include "trcamerascenenodeanimator.h"

constexpr int FrobbelFlag_UnpassableSteepUpslant = 0x01;
constexpr int FrobbelFlag_UnwalkableSteepFloor = 0x02;
constexpr int FrobbelFlag_UnwalkableDeadlyFloor = 0x04;
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
constexpr int MaxGrabableGradient = 60;

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
    static constexpr int AxisColl_InsufficientFrontSpace = 0x01;
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
    int neededFloorDistanceBottom; // external
    int neededFloorDistanceTop; // external
    int neededCeilingDistance; // external

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
        if( (frobbelFlags & FrobbelFlag_UnpassableSteepUpslant) != 0 && front.floor.slantClass == SlantClass::Steep && front.floor.distance < 0 )
        {
            front.floor.distance = -32767;
        }
        else if( front.floor.distance > 0
            && (
                ((frobbelFlags & FrobbelFlag_UnwalkableSteepFloor) != 0 && front.floor.slantClass == SlantClass::Steep)
                || ((frobbelFlags & FrobbelFlag_UnwalkableDeadlyFloor) != 0 && front.floor.lastTriggerOrKill != nullptr && loader::extractFDFunction(*front.floor.lastTriggerOrKill) == loader::FDFunction::Death)
            ) )
        {
            front.floor.distance = 2 * loader::QuarterSectorSize;
        }

        // Front left
        checkPos = loader::TRCoordinates(frontLeftX, 0, frontLeftZ);
        sector = level.findSectorForPosition(reachablePos + checkPos, level.m_camera->getCurrentRoom());
        frontLeft.init(sector, pos + checkPos, level.m_camera, height);

        if( (frobbelFlags & FrobbelFlag_UnpassableSteepUpslant) != 0 && frontLeft.floor.slantClass == SlantClass::Steep && frontLeft.floor.distance < 0 )
        {
            frontLeft.floor.distance = -32767;
        }
        else if( frontLeft.floor.distance > 0
            && (
                ((frobbelFlags & FrobbelFlag_UnwalkableSteepFloor) != 0 && frontLeft.floor.slantClass == SlantClass::Steep)
                || ((frobbelFlags & FrobbelFlag_UnwalkableDeadlyFloor) != 0 && frontLeft.floor.lastTriggerOrKill != nullptr && loader::extractFDFunction(*frontLeft.floor.lastTriggerOrKill) == loader::FDFunction::Death)
            ) )
        {
            frontLeft.floor.distance = 2 * loader::QuarterSectorSize;
        }

        // Front right
        checkPos = loader::TRCoordinates(frontRightX, 0, frontRightZ);
        sector = level.findSectorForPosition(reachablePos + checkPos, level.m_camera->getCurrentRoom());
        frontRight.init(sector, pos + checkPos, level.m_camera, height);

        if( (frobbelFlags & FrobbelFlag_UnpassableSteepUpslant) != 0 && frontRight.floor.slantClass == SlantClass::Steep && frontRight.floor.distance < 0 )
        {
            frontRight.floor.distance = -32767;
        }
        else if( frontRight.floor.distance > 0
            && (
                ((frobbelFlags & FrobbelFlag_UnwalkableSteepFloor) != 0 && frontRight.floor.slantClass == SlantClass::Steep)
                || ((frobbelFlags & FrobbelFlag_UnwalkableDeadlyFloor) != 0 && frontRight.floor.lastTriggerOrKill != nullptr && loader::extractFDFunction(*frontRight.floor.lastTriggerOrKill) == loader::FDFunction::Death)
            ) )
        {
            frontRight.floor.distance = 2 * loader::QuarterSectorSize;
        }

        //! @todo check static mesh collisions here

        if( current.floor.distance == -loader::HeightLimit )
        {
            collisionFeedback = position.toInexact() - pos;
            axisCollisions = AxisColl_InsufficientFrontSpace;
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

        if( front.floor.distance > neededFloorDistanceBottom || front.floor.distance < neededFloorDistanceTop || front.ceiling.distance > neededCeilingDistance )
        {
            axisCollisions = AxisColl_InsufficientFrontSpace;
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

        if( front.ceiling.distance >= neededCeilingDistance )
        {
            axisCollisions = AxisColl_BumpHead;
            collisionFeedback = position.toInexact() - pos;
            return;
        }

        if( frontLeft.floor.distance > neededFloorDistanceBottom || frontLeft.floor.distance < neededFloorDistanceTop )
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

        if( frontRight.floor.distance > neededFloorDistanceBottom || frontRight.floor.distance < neededFloorDistanceTop )
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

void LaraStateHandler::setTargetState(LaraStateId st)
{
    m_dispatcher->setTargetState(static_cast<uint16_t>(st));
}

void LaraStateHandler::setStateOverride(LaraStateId st)
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

void LaraStateHandler::onInput0WalkForward(LaraState& /*state*/)
{
    if( getHealth() <= 0 )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    if( getInputState().xMovement == AxisMovement::Left )
        subYRotationSpeed(409, -728);
    else if( getInputState().xMovement == AxisMovement::Right )
        addYRotationSpeed(409, 728);
    if( getInputState().zMovement == AxisMovement::Forward )
    {
        if( getInputState().moveSlow )
            setTargetState(LaraStateId::WalkForward);
        else
            setTargetState(LaraStateId::RunForward);
    }
    else
    {
        setTargetState(LaraStateId::Stop);
    }
}

void LaraStateHandler::onBehave0WalkForward(LaraState& state)
{
    setFallSpeed(0);
    setFalling(false);
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.neededFloorDistanceBottom = ClimbLimit2ClickMin;
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.frobbelFlags |= FrobbelFlag_UnpassableSteepUpslant | FrobbelFlag_UnwalkableSteepFloor | FrobbelFlag_UnwalkableDeadlyFloor;
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());

    if( tryStopOnFloor(state) || tryClimb(state) )
        return;

    if( checkWallCollision(state) )
    {
        const auto fr = getCurrentFrame();
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

    if( state.current.floor.distance > ClimbLimit2ClickMin )
    {
        playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
        setStateOverride(LaraStateId::JumpForward);
        setTargetState(LaraStateId::JumpForward);
        setFallSpeed(0);
        setFalling(true);
    }

    if( state.current.floor.distance > SteppableHeight )
    {
        const auto fr = getCurrentFrame();
        if( fr < 28 || fr > 45 )
        {
            playAnimation(loader::AnimationId::WALK_DOWN_RIGHT, 887);
        }
        else
        {
            playAnimation(loader::AnimationId::WALK_DOWN_LEFT, 874);
        }
    }

    if( state.current.floor.distance >= -ClimbLimit2ClickMin && state.current.floor.distance < -SteppableHeight )
    {
        const auto fr = getCurrentFrame();
        if( fr < 27 || fr > 44 )
        {
            playAnimation(loader::AnimationId::WALK_UP_STEP_RIGHT, 844);
        }
        else
        {
            playAnimation(loader::AnimationId::WALK_UP_STEP_LEFT, 858);
        }
    }

    if( !tryStartSlide(state) )
    {
        placeOnFloor(state);
    }
}

void LaraStateHandler::onInput1RunForward(LaraState& /*state*/)
{
    if( getHealth() <= 0 )
    {
        setTargetState(LaraStateId::Death);
        return;
    }

    if( getInputState().roll )
    {
        playAnimation(loader::AnimationId::ROLL_BEGIN, 3857);
        setTargetState(LaraStateId::Stop);
        setStateOverride(LaraStateId::RollForward);
        return;
    }

    if( getInputState().xMovement == AxisMovement::Left )
    {
        subYRotationSpeed(409, -1456);
        setZRotationExact( std::max(-2002.f, getRotation().Z - makeSpeedValue(273).getScaledExact(getCurrentDeltaTime())) );
    }
    else if( getInputState().xMovement == AxisMovement::Right )
    {
        addYRotationSpeed(409, 1456);
        setZRotationExact( std::min(2002.f, getRotation().Z + makeSpeedValue(273).getScaledExact(getCurrentDeltaTime())) );
    }

    if( getInputState().jump && !isFalling() )
    {
        setTargetState(LaraStateId::JumpForward);
        return;
    }

    if( getInputState().zMovement != AxisMovement::Forward )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    if( getInputState().moveSlow )
        setTargetState(LaraStateId::WalkForward);
    else
        setTargetState(LaraStateId::RunForward);
}

void LaraStateHandler::onInput2Stop(LaraState& state)
{
    if( getHealth() <= 0 )
    {
        setTargetState(LaraStateId::Death);
        return;
    }

    if( getInputState().roll )
    {
        playAnimation(loader::AnimationId::ROLL_BEGIN);
        setTargetState(LaraStateId::Stop);
        setStateOverride(LaraStateId::RollForward);
        return;
    }

    setTargetState(LaraStateId::Stop);

    if( getInputState().stepMovement == AxisMovement::Left )
    {
        setTargetState(LaraStateId::StepLeft);
    }
    else if( getInputState().stepMovement == AxisMovement::Right )
    {
        setTargetState(LaraStateId::StepRight);
    }

    if( getInputState().xMovement == AxisMovement::Left )
    {
        setTargetState(LaraStateId::TurnLeftSlow);
    }
    else if( getInputState().xMovement == AxisMovement::Right )
    {
        setTargetState(LaraStateId::TurnRightSlow);
    }

    if( getInputState().jump )
    {
        setTargetState(LaraStateId::JumpPrepare);
    }
    else if( getInputState().zMovement == AxisMovement::Forward )
    {
        if( getInputState().moveSlow )
            onInput0WalkForward(state);
        else
            onInput1RunForward(state);
    }
    else if( getInputState().zMovement == AxisMovement::Backward )
    {
        if( getInputState().moveSlow )
            onInput16WalkBackward(state);
        else
            setTargetState(LaraStateId::RunBack);
    }
}

void LaraStateHandler::onInput3JumpForward(LaraState& /*state*/)
{
    if( getTargetState() == LaraStateId::SwandiveBegin || getTargetState() == LaraStateId::Reach )
        setTargetState(LaraStateId::JumpForward);

    if( getTargetState() != LaraStateId::Death && getTargetState() != LaraStateId::Stop )
    {
        if( getInputState().action && getHandStatus() == 0 )
            setTargetState(LaraStateId::Reach);
        if( getInputState().moveSlow && getHandStatus() == 0 )
            setTargetState(LaraStateId::SwandiveBegin);
        if( getFallSpeed().get() > FreeFallSpeedThreshold )
            setTargetState(LaraStateId::FreeFall);
    }

    if( getInputState().xMovement == AxisMovement::Left )
    {
        subYRotationSpeed(409, -546);
    }
    else if( getInputState().xMovement == AxisMovement::Right )
    {
        addYRotationSpeed(409, 546);
    }
}

void LaraStateHandler::onInput5RunBackward(LaraState& /*state*/)
{
    setTargetState(LaraStateId::Stop);

    if( getInputState().xMovement == AxisMovement::Left )
        subYRotationSpeed(409, -1092);
    else if( getInputState().xMovement == AxisMovement::Right )
        addYRotationSpeed(409, 1092);
}

void LaraStateHandler::onInput6TurnRightSlow(LaraState& /*state*/)
{
    if( getHealth() <= 0 )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    addYRotationSpeed(409);

    if( getHandStatus() == 4 )
    {
        setTargetState(LaraStateId::TurnFast);
        return;
    }

    if( getYRotationSpeed() > 728 )
    {
        if( getInputState().moveSlow )
            setYRotationSpeed( 728 );
        else
            setTargetState(LaraStateId::TurnFast);
    }

    if( getInputState().zMovement != AxisMovement::Forward )
    {
        if( getInputState().xMovement != AxisMovement::Right )
            setTargetState(LaraStateId::Stop);
        return;
    }

    if( getInputState().moveSlow )
        setTargetState(LaraStateId::WalkForward);
    else
        setTargetState(LaraStateId::RunForward);
}

void LaraStateHandler::onInput7TurnLeftSlow(LaraState& /*state*/)
{
    if( getHealth() <= 0 )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    subYRotationSpeed(409);

    if( getHandStatus() == 4 )
    {
        setTargetState(LaraStateId::TurnFast);
        return;
    }

    if( getYRotationSpeed() < -728 )
    {
        if( getInputState().moveSlow )
            setYRotationSpeed(-728);
        else
            setTargetState(LaraStateId::TurnFast);
    }

    if( getInputState().zMovement != AxisMovement::Forward )
    {
        if( getInputState().xMovement != AxisMovement::Left )
            setTargetState(LaraStateId::Stop);
        return;
    }

    if( getInputState().moveSlow )
        setTargetState(LaraStateId::WalkForward);
    else
        setTargetState(LaraStateId::RunForward);
}

void LaraStateHandler::onBehaveTurnSlow(LaraState& state)
{
    setFallSpeed(0);
    setFalling(false);
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.neededFloorDistanceBottom = ClimbLimit2ClickMin;
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.frobbelFlags |= FrobbelFlag_UnpassableSteepUpslant | FrobbelFlag_UnwalkableSteepFloor;
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());

    if( state.current.floor.distance <= 100 )
    {
        if( !tryStartSlide(state) )
            placeOnFloor(state);

        return;
    }

    playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
    setTargetState(LaraStateId::JumpForward);
    setStateOverride(LaraStateId::JumpForward);
    setFallSpeed(0);
    setFalling(true);
}

void LaraStateHandler::onInput9FreeFall(LaraState& /*state*/)
{
    dampenHorizontalSpeed(5, 100);
    if( getFallSpeed().get() > 154 )
    {
        //! @todo playSound(30)
    }
}

void LaraStateHandler::onInput15JumpPrepare(LaraState& /*state*/)
{
    if( getInputState().zMovement == AxisMovement::Forward && getRelativeHeightAtDirection(getRotation().Y, 256) >= -ClimbLimit2ClickMin )
    {
        setMovementAngle(getRotation().Y);
        setTargetState(LaraStateId::JumpForward);
    }
    else if( getInputState().xMovement == AxisMovement::Left && getRelativeHeightAtDirection(getRotation().Y - util::degToAu(90), 256) >= -ClimbLimit2ClickMin )
    {
        setMovementAngle(getRotation().Y - util::degToAu(90));
        setTargetState(LaraStateId::JumpRight);
    }
    else if( getInputState().xMovement == AxisMovement::Right && getRelativeHeightAtDirection(getRotation().Y + util::degToAu(90), 256) >= -ClimbLimit2ClickMin )
    {
        setMovementAngle(getRotation().Y + util::degToAu(90));
        setTargetState(LaraStateId::JumpLeft);
    }
    else if( getInputState().zMovement == AxisMovement::Backward && getRelativeHeightAtDirection(getRotation().Y + util::degToAu(180), 256) >= -ClimbLimit2ClickMin )
    {
        setMovementAngle(getRotation().Y + util::degToAu(180));
        setTargetState(LaraStateId::JumpBack);
    }

    if( getFallSpeed().get() > FreeFallSpeedThreshold )
    {
        setTargetState(LaraStateId::FreeFall);
    }
}

void LaraStateHandler::onInput16WalkBackward(LaraState& /*state*/)
{
    if( getHealth() <= 0 )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    if( getInputState().zMovement == AxisMovement::Backward && getInputState().moveSlow )
        setTargetState(LaraStateId::WalkBackward);
    else
        setTargetState(LaraStateId::Stop);

    if( getInputState().xMovement == AxisMovement::Left )
        subYRotationSpeed(409, -728);
    else if( getInputState().xMovement == AxisMovement::Right )
        addYRotationSpeed(409, 728);
}

void LaraStateHandler::onInput19Climbing(LaraState& state)
{
    state.frobbelFlags &= ~(FrobbelFlag08 | FrobbelFlag10);
}

void LaraStateHandler::onInput20TurnFast(LaraState& /*state*/)
{
    if( getHealth() <= 0 )
    {
        setTargetState(LaraStateId::Stop);
        return;
    }

    if( getYRotationSpeed() >= 0 )
    {
        setYRotationSpeed( 1456 );
        if( getInputState().xMovement == AxisMovement::Right )
            return;
    }
    else
    {
        setYRotationSpeed( -1456 );
        if( getInputState().xMovement == AxisMovement::Left )
            return;
    }
    setTargetState(LaraStateId::Stop);
}

void LaraStateHandler::onInput24SlideForward(LaraState& /*state*/)
{
    if( getInputState().jump )
        setTargetState(LaraStateId::JumpForward);
}

void LaraStateHandler::onBehave24SlideForward(LaraState& state)
{
    setMovementAngle(getRotation().Y);
    commonSlideHandling(state);
}

void LaraStateHandler::onInput25JumpBackward(LaraState& /*state*/)
{
    //! @todo Set local camera Y rotation to 24570 AU
    if( getFallSpeed().get() > FreeFallSpeedThreshold )
        setTargetState(LaraStateId::FreeFall);
}

void LaraStateHandler::onBehave25JumpBackward(LaraState& state)
{
    setMovementAngle(getRotation().Y + util::degToAu(180));
    commonJumpHandling(state);
}

void LaraStateHandler::onInput28JumpUp(LaraState& /*state*/)
{
    if( getFallSpeed().get() > FreeFallSpeedThreshold )
        setTargetState(LaraStateId::FreeFall);
}

void LaraStateHandler::onInput29FallBackward(LaraState& /*state*/)
{
    if( getFallSpeed().get() > FreeFallSpeedThreshold )
        setTargetState(LaraStateId::FreeFall);

    if( getInputState().action && getHandStatus() == 0 )
        setTargetState(LaraStateId::Reach);
}

void LaraStateHandler::onBehave29FallBackward(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededCeilingDistance = 192;
    state.yAngle = getRotation().Y + util::degToAu(180);
    state.initHeightInfo(this, getLevel(), 870, getPosition()); //! @todo MAGICK 870
    checkJumpWallSmash(state);
    if( state.current.floor.distance > 0 || getFallSpeed().get() <= 0 )
        return;

    if( applyLandingDamage(state) )
        setTargetState(LaraStateId::Death);
    else
        setTargetState(LaraStateId::Stop);

    setFallSpeed(0);
    placeOnFloor(state);
    setFalling(false);
}

void LaraStateHandler::onInput32SlideBackward(LaraState& /*state*/)
{
    if( getInputState().jump )
        setTargetState(LaraStateId::JumpBack);
}

void LaraStateHandler::handleLaraStateOnLand()
{
    //! @todo Only when on solid ground
    m_air = 1800;

    LaraState laraState;
    laraState.position = loader::ExactTRCoordinates(getPosition());
    laraState.collisionRadius = 100; //!< @todo MAGICK 100
    laraState.frobbelFlags = FrobbelFlag10 | FrobbelFlag08;

    static HandlersArray inputHandlers{{
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
        &LaraStateHandler::onInput11Reach,
        &LaraStateHandler::nopHandler,
        nullptr,
        &LaraStateHandler::nopHandler,
        &LaraStateHandler::onInput15JumpPrepare,
        &LaraStateHandler::onInput16WalkBackward,
        nullptr,
        nullptr,
        &LaraStateHandler::onInput19Climbing,
        &LaraStateHandler::onInput20TurnFast,
        nullptr,
        nullptr,
        &LaraStateHandler::nopHandler,
        &LaraStateHandler::onInput24SlideForward,
        &LaraStateHandler::onInput25JumpBackward,
        &LaraStateHandler::onInput26JumpLeft,
        &LaraStateHandler::onInput27JumpRight,
        &LaraStateHandler::onInput28JumpUp,
        &LaraStateHandler::onInput29FallBackward,
        // 30
        nullptr,
        nullptr,
        &LaraStateHandler::onInput32SlideBackward,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        // 40
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        &LaraStateHandler::nopHandler,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        // 50
        nullptr,
        nullptr,
        &LaraStateHandler::onInput52SwandiveBegin,
        &LaraStateHandler::onInput53SwandiveEnd,
        nullptr,
        nullptr
    }};

    callHandler(inputHandlers, m_dispatcher->getCurrentState(), laraState, "input");

    // "slowly" revert rotations to zero
    if( getRotation().Z < 0 )
    {
        m_rotation.Z += makeSpeedValue(182).getScaledExact(getCurrentDeltaTime());
        if( getRotation().Z >= 0 )
            m_rotation.Z = 0;
    }
    else if( getRotation().Z > 0 )
    {
        m_rotation.Z -= makeSpeedValue(182).getScaledExact(getCurrentDeltaTime());
        if( getRotation().Z <= 0 )
            m_rotation.Z = 0;
    }

    if( getYRotationSpeed() < 0 )
    {
        m_yRotationSpeed.addExact(364, getCurrentDeltaTime()).limitMax(0);
    }
    else if( getYRotationSpeed() > 0 )
    {
        m_yRotationSpeed.subExact(364, getCurrentDeltaTime()).limitMin(0);
    }
    else
    {
        setYRotationSpeed(0);
    }

    m_rotation.Y += m_yRotationSpeed.getScaledExact(getCurrentDeltaTime());

    processAnimCommands();

    // @todo test interactions?

    // BOOST_LOG_TRIVIAL(debug) << "BEHAVE State=" << m_dispatcher->getCurrentState() << ", pos = (" << m_position.X << "/" << m_position.Y << "/" << m_position.Z << ")";

    static HandlersArray behaviourHandlers{{
        &LaraStateHandler::onBehave0WalkForward,
        &LaraStateHandler::onBehave1RunForward,
        &LaraStateHandler::onBehaveStanding,
        &LaraStateHandler::onBehave3JumpForward,
        &LaraStateHandler::onBehaveStanding,
        &LaraStateHandler::onBehave5RunBackward,
        &LaraStateHandler::onBehaveTurnSlow,
        &LaraStateHandler::onBehaveTurnSlow,
        nullptr,
        &LaraStateHandler::onBehave9FreeFall,
        // 10
        nullptr,
        &LaraStateHandler::onBehave11Reach,
        &LaraStateHandler::onBehave12Unknown,
        nullptr,
        &LaraStateHandler::onBehaveStanding,
        &LaraStateHandler::onBehave15JumpPrepare,
        &LaraStateHandler::onBehave16WalkBackward,
        nullptr,
        nullptr,
        &LaraStateHandler::onBehave19Climbing,
        &LaraStateHandler::onBehaveStanding,
        nullptr,
        nullptr,
        &LaraStateHandler::onBehave23RollBackward,
        &LaraStateHandler::onBehave24SlideForward,
        &LaraStateHandler::onBehave25JumpBackward,
        &LaraStateHandler::onBehave26JumpLeft,
        &LaraStateHandler::onBehave27JumpRight,
        &LaraStateHandler::onBehave28JumpUp,
        &LaraStateHandler::onBehave29FallBackward,
        // 30
        nullptr,
        nullptr,
        &LaraStateHandler::onBehave32SlideBackward,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        // 40
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        &LaraStateHandler::onBehave45RollForward,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        // 50
        nullptr,
        nullptr,
        &LaraStateHandler::onBehave52SwandiveBegin,
        &LaraStateHandler::onBehave53SwandiveEnd,
        nullptr,
        nullptr
    }};

    callHandler(behaviourHandlers, m_dispatcher->getCurrentState(), laraState, "behaviour");

    updateFloorHeight(-381);
    handleTriggers(laraState.current.floor.lastTriggerOrKill, false);
}

irr::u32 LaraStateHandler::getCurrentFrame() const
{
    return m_dispatcher->getCurrentFrame();
}

void LaraStateHandler::placeOnFloor(const LaraState & state)
{
    m_position.Y += state.current.floor.distance;
}

loader::LaraStateId LaraStateHandler::getCurrentState() const
{
    return static_cast<LaraStateId>(m_dispatcher->getCurrentState());
}

void LaraStateHandler::onBehave19Climbing(LaraState& state)
{
    state.neededFloorDistanceBottom = ClimbLimit2ClickMin;
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.frobbelFlags |= FrobbelFlag_UnpassableSteepUpslant | FrobbelFlag_UnwalkableSteepFloor;
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());
}

void LaraStateHandler::onBehave15JumpPrepare(LaraState& state)
{
    setFallSpeed(0);
    setFalling(false);
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -loader::HeightLimit;
    state.neededCeilingDistance = 0;
    state.yAngle = getRotation().Y;
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());

    if( state.current.ceiling.distance <= -100 )
        return;

    setTargetState(LaraStateId::Stop);
    setStateOverride(LaraStateId::Stop);
    playAnimation(loader::AnimationId::STAY_SOLID, 185);
    setHorizontalSpeed(0);
    setPosition(state.position);
}

void LaraStateHandler::onBehave1RunForward(LaraState& state)
{
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.frobbelFlags |= FrobbelFlag_UnpassableSteepUpslant;
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());
    if( tryStopOnFloor(state) || tryClimb(state) )
        return;

    if( checkWallCollision(state) )
    {
        setZRotation(0);
        if( state.front.floor.slantClass == SlantClass::None && state.front.floor.distance < -ClimbLimit2ClickMax )
        {
            setStateOverride(LaraStateId::Unknown12);
            if( getCurrentFrame() >= 0 && getCurrentFrame() <= 9 )
            {
                playAnimation(loader::AnimationId::WALL_SMASH_LEFT, 800);
                return;
            }
            if( getCurrentFrame() >= 10 && getCurrentFrame() <= 21 )
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
        setFalling(true);
        setFallSpeed(0);
        return;
    }

    if( state.current.floor.distance >= -ClimbLimit2ClickMin && state.current.floor.distance < -SteppableHeight )
    {
        if( getCurrentFrame() >= 3 && getCurrentFrame() <= 14 )
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
        if(state.current.floor.distance > 50)
            state.current.floor.distance = 50;
        placeOnFloor(state);
    }
}

void LaraStateHandler::onBehave5RunBackward(LaraState& state)
{
    setFallSpeed(0);
    setFalling(false);
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.frobbelFlags |= FrobbelFlag_UnpassableSteepUpslant | FrobbelFlag_UnwalkableSteepFloor;
    state.yAngle = getRotation().Y + util::degToAu(180);
    setMovementAngle(state.yAngle);
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());
    if( tryStopOnFloor(state) )
        return;

    if( state.current.floor.distance > 200 )
    {
        playAnimation(loader::AnimationId::FREE_FALL_BACK, 1473);
        setTargetState(LaraStateId::FallBackward);
        setStateOverride(LaraStateId::FallBackward);
        setFallSpeed(0);
        setFalling(true);
        return;
    }

    if( checkWallCollision(state) )
    {
        playAnimation(loader::AnimationId::STAY_SOLID, 185);
    }
    placeOnFloor(state);
}

void LaraStateHandler::onBehave16WalkBackward(LaraState& state)
{
    setFallSpeed(0);
    setFalling(false);
    state.neededFloorDistanceBottom = ClimbLimit2ClickMin;
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.yAngle = getRotation().Y + util::degToAu(180);
    setMovementAngle(state.yAngle);
    state.frobbelFlags |= FrobbelFlag_UnpassableSteepUpslant | FrobbelFlag_UnwalkableSteepFloor;
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());
    if( tryStopOnFloor(state) )
        return;

    if( checkWallCollision(state) )
    {
        playAnimation(loader::AnimationId::STAY_SOLID, 185);
    }

    if( state.current.floor.distance > loader::QuarterSectorSize && state.current.floor.distance < ClimbLimit2ClickMin )
    {
        if( getCurrentFrame() < 964 || getCurrentFrame() > 993 )
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
        placeOnFloor(state);
    }
}

void LaraStateHandler::onBehave9FreeFall(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededCeilingDistance = 192;
    state.yAngle = getMovementAngle();
    setFalling(true);
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());
    jumpAgainstWall(state);
    if( state.current.floor.distance > 0 )
        return;

    if( applyLandingDamage(state) )
    {
        setTargetState(LaraStateId::Death);
    }
    else
    {
        setTargetState(LaraStateId::Stop);
        setStateOverride(LaraStateId::Stop);
        playAnimation(loader::AnimationId::LANDING_HARD, 358);
    }
    setFallSpeed(0);
    placeOnFloor(state);
    setFalling(false);
}

void LaraStateHandler::onBehave12Unknown(LaraState& state)
{
    state.neededFloorDistanceBottom = ClimbLimit2ClickMin;
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.yAngle = getMovementAngle();
    state.frobbelFlags |= FrobbelFlag_UnpassableSteepUpslant | FrobbelFlag_UnwalkableSteepFloor;
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());
    applyCollisionFeedback(state);
}

void LaraStateHandler::onBehave3JumpForward(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededCeilingDistance = 192;
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());
    checkJumpWallSmash(state);

    if( state.current.floor.distance > 0 || getFallSpeed().get() <= 0 )
        return;

    if( applyLandingDamage(state) )
    {
        setTargetState(LaraStateId::Death);
    }
    else if( getInputState().zMovement != AxisMovement::Forward || getInputState().moveSlow )
    {
        setTargetState(LaraStateId::Stop);
    }
    else
    {
        setTargetState(LaraStateId::RunForward);
    }

    setFallSpeed(0);
    setFalling(false);
    setHorizontalSpeed(0);
    placeOnFloor(state);
    processAnimCommands();
}

void LaraStateHandler::onBehave28JumpUp(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededCeilingDistance = 192;
    state.yAngle = getRotation().Y;
    state.initHeightInfo(this, getLevel(), 870, getPosition()); //! @todo MAGICK 870
    if( tryGrabEdge(state) )
        return;

    jumpAgainstWall(state);
    if( getFallSpeed().get() <= 0 || state.current.floor.distance > 0 )
        return;

    if( applyLandingDamage(state) )
        setTargetState(LaraStateId::Death);
    else
        setTargetState(LaraStateId::Stop);
    setFallSpeed(0);
    placeOnFloor(state);
    setFalling(false);
}

void LaraStateHandler::onBehave32SlideBackward(LaraState& state)
{
    setMovementAngle(getRotation().Y + util::degToAu(180));
    commonSlideHandling(state);
}

void LaraStateHandler::jumpAgainstWall(LaraState& state)
{
    applyCollisionFeedback(state);
    if( state.axisCollisions == LaraState::AxisColl_FrontLeftBump )
        addYRotation( makeSpeedValue(910).getScaledExact(getCurrentDeltaTime()) );
    else if( state.axisCollisions == LaraState::AxisColl_FrontRightBump )
        addYRotation( -makeSpeedValue(940).getScaledExact(getCurrentDeltaTime()) );
    else if( state.axisCollisions == LaraState::AxisColl_HeadInCeiling )
    {
        if( getFallSpeed().get() <= 0 )
            setFallSpeed(1);
    }
    else if( state.axisCollisions == LaraState::AxisColl_CeilingTooLow )
    {
        moveXZ(
            -makeSpeedValue(100).getScaledExact(getCurrentDeltaTime()) * std::sin(util::auToRad(getRotation().Y)),
            -makeSpeedValue(100).getScaledExact(getCurrentDeltaTime()) * std::cos(util::auToRad(getRotation().Y))
        );
        setHorizontalSpeed(0);
        state.current.floor.distance = 0;
        if( getFallSpeed().get() < 0 )
            setFallSpeed(16);
    }
}

void LaraStateHandler::checkJumpWallSmash(LaraState& state)
{
    applyCollisionFeedback(state);

    if( state.axisCollisions == LaraState::AxisColl_None )
        return;

    if( state.axisCollisions == LaraState::AxisColl_InsufficientFrontSpace || state.axisCollisions == LaraState::AxisColl_BumpHead )
    {
        setTargetState(LaraStateId::FreeFall);
        //! @todo Check values
        //! @bug Time is too short to properly apply collision momentum!
        dampenHorizontalSpeed(4, 5);
        setMovementAngle(getMovementAngle() - util::degToAu(180));
        playAnimation(loader::AnimationId::SMASH_JUMP, 481);
        setStateOverride(LaraStateId::FreeFall);
        if( getFallSpeed().get() <= 0 )
            setFallSpeed(1);
        return;
    }

    if( state.axisCollisions == LaraState::AxisColl_FrontLeftBump )
    {
        addYRotation( makeSpeedValue(910).getScaledExact(getCurrentDeltaTime()) );
        return;
    }
    else if( state.axisCollisions == LaraState::AxisColl_FrontRightBump )
    {
        addYRotation( -makeSpeedValue(910).getScaledExact(getCurrentDeltaTime()) );
        return;
    }

    if( state.axisCollisions == LaraState::AxisColl_CeilingTooLow )
    {
        moveXZ(
            -makeSpeedValue(100).getScaledExact(getCurrentDeltaTime()) * std::sin(util::auToRad(state.yAngle)),
            -makeSpeedValue(100).getScaledExact(getCurrentDeltaTime()) * std::cos(util::auToRad(state.yAngle))
        );
        setHorizontalSpeed(0);
        state.current.floor.distance = 0;
        if( getFallSpeed().get() <= 0 )
            setFallSpeed(16);
    }

    if( state.axisCollisions == LaraState::AxisColl_HeadInCeiling && getFallSpeed().get() <= 0 )
        setFallSpeed(1);
}

void LaraStateHandler::animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs)
{
    BOOST_ASSERT(m_lara == node);

    if( m_lastFrameTime < 0 )
        m_lastFrameTime = m_currentFrameTime = timeMs;

    if( m_lastFrameTime == timeMs )
        return;

    m_currentFrameTime = timeMs;

    handleLaraStateOnLand();

    m_lastFrameTime = m_currentFrameTime;
}

void LaraStateHandler::processAnimCommands()
{
    if( m_dispatcher->handleTRTransitions() || m_lastAnimFrame != getCurrentFrame() )
    {
        clearStateOverride();
        m_lastAnimFrame = getCurrentFrame();
    }

    const loader::Animation& animation = getLevel().m_animations[m_dispatcher->getCurrentAnimationId()];
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
                m_position += {makeSpeedValue(cmd[0]).getScaledExact(getCurrentDeltaTime()), makeSpeedValue(cmd[1]).getScaledExact(getCurrentDeltaTime()), makeSpeedValue(cmd[2]).getScaledExact(getCurrentDeltaTime())};
                cmd += 3;
                break;
            case AnimCommandOpcode::SetVelocity:
                setFallSpeed(m_fallSpeedOverride == 0 ? cmd[0] : m_fallSpeedOverride);
                m_fallSpeedOverride = 0;
                setHorizontalSpeed(cmd[1]);
                setFalling(true);
                cmd += 2;
                break;
            case AnimCommandOpcode::EmptyHands:
                setHandStatus(0);
                break;
            case AnimCommandOpcode::PlaySound:
                if( getCurrentFrame() == cmd[0] )
                {
                    //! @todo playsound(cmd[1])
                }
                cmd += 2;
                break;
            case AnimCommandOpcode::PlayEffect:
                if( getCurrentFrame() == cmd[0] )
                {
                    BOOST_LOG_TRIVIAL(debug) << "Anim effect: " << int(cmd[1]);
                    if( cmd[1] == 0 )
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
        if( getFallSpeed().get() >= 128 )
            m_fallSpeed.addExact(1, getCurrentDeltaTime());
        else
            m_fallSpeed.addExact(6, getCurrentDeltaTime());
    }
    else
    {
        m_horizontalSpeed = m_dispatcher->calculateFloorSpeed();
    }

    m_position.X += std::sin(util::auToRad(getMovementAngle())) * m_horizontalSpeed.getScaledExact(getCurrentDeltaTime());
    m_position.Y += getFallSpeed().getScaledExact(getCurrentDeltaTime());
    m_position.Z += std::cos(util::auToRad(getMovementAngle())) * m_horizontalSpeed.getScaledExact(getCurrentDeltaTime());
    m_lara->setPosition(m_position.toIrrlicht());

    {
        //! @todo This is horribly inefficient code, but it properly converts ZXY angles to XYZ angles.
        irr::core::quaternion q;
        q.makeIdentity();
        q *= irr::core::quaternion().fromAngleAxis(util::auToRad(getRotation().Y), {0,1,0});
        q *= irr::core::quaternion().fromAngleAxis(util::auToRad(getRotation().X), {1,0,0});
        q *= irr::core::quaternion().fromAngleAxis(util::auToRad(getRotation().Z), {0,0,-1});

        irr::core::vector3df euler;
        q.toEuler(euler);
        m_lara->setRotation(euler * 180 / irr::core::PI);
    }

    m_lara->updateAbsolutePosition();
}

bool LaraStateHandler::tryStopOnFloor(LaraState& state)
{
    if( state.axisCollisions != LaraState::AxisColl_HeadInCeiling && state.axisCollisions != LaraState::AxisColl_CeilingTooLow )
        return false;

    setPosition(state.position);

    setTargetState(LaraStateId::Stop);
    setStateOverride(LaraStateId::Stop);
    playAnimation(loader::AnimationId::STAY_SOLID, 185);
    setHorizontalSpeed(0);
    setFallSpeed(0);
    setFalling(false);
    return true;
}

bool LaraStateHandler::tryClimb(LaraState& state)
{
    if( state.axisCollisions != LaraState::AxisColl_InsufficientFrontSpace || !getInputState().jump || getHandStatus() != 0 )
        return false;

    const auto floorGradient = std::abs(state.frontLeft.floor.distance - state.frontRight.floor.distance);
    if( floorGradient >= MaxGrabableGradient )
        return false;

    int alignedRotation;
    //! @todo MAGICK +/- 30 degrees
    if( getRotation().Y >= util::degToAu(-30) && getRotation().Y <= util::degToAu(30) )
        alignedRotation = util::degToAu(0);
    else if( getRotation().Y >= util::degToAu(60) && getRotation().Y <= util::degToAu(120) )
        alignedRotation = util::degToAu(90);
    else if( getRotation().Y >= util::degToAu(150) && getRotation().Y <= util::degToAu(210) )
        alignedRotation = util::degToAu(180);
    else if( getRotation().Y >= util::degToAu(240) && getRotation().Y <= util::degToAu(300) )
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
        moveY(2 * loader::QuarterSectorSize + climbHeight);
        setHandStatus(1);
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
        moveY(3 * loader::QuarterSectorSize + climbHeight);
        setHandStatus(1);
    }
    else
    {
        if( climbHeight < -JumpReachableHeight || climbHeight > -ClimbLimit3ClickMax )
            return false;

        setTargetState(LaraStateId::JumpUp);
        setStateOverride(LaraStateId::Stop);
        playAnimation(loader::AnimationId::STAY_SOLID, 185);
        setFallSpeedOverride( -static_cast<int>(std::sqrt(-12 * (climbHeight + 800) + 3)) );
        processAnimCommands();
    }

    setYRotation(alignedRotation);
    applyCollisionFeedback(state);

    return true;
}

void LaraStateHandler::applyCollisionFeedback(LaraState& state)
{
    setPosition(loader::ExactTRCoordinates(getPosition() + state.collisionFeedback));
    state.collisionFeedback = {0,0,0};
}

bool LaraStateHandler::checkWallCollision(LaraState& state)
{
    if( state.axisCollisions == LaraState::AxisColl_InsufficientFrontSpace || state.axisCollisions == LaraState::AxisColl_BumpHead )
    {
        applyCollisionFeedback(state);
        setTargetState(LaraStateId::Stop);
        setStateOverride(LaraStateId::Stop);
        setFalling(false);
        setHorizontalSpeed(0);
        return true;
    }

    if( state.axisCollisions == LaraState::AxisColl_FrontLeftBump )
    {
        applyCollisionFeedback(state);
        addYRotation( makeSpeedValue(910).getScaledExact(getCurrentDeltaTime()) );
    }
    else if( state.axisCollisions == LaraState::AxisColl_FrontRightBump )
    {
        applyCollisionFeedback(state);
        addYRotation( -makeSpeedValue(910).getScaledExact(getCurrentDeltaTime()) );
    }

    return false;
}

bool LaraStateHandler::tryStartSlide(LaraState& state)
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

    int16_t dy = std::abs(targetAngle - getRotation().Y);
    applyCollisionFeedback(state);
    if( dy > util::degToAu(90) || dy < util::degToAu(-90) )
    {
        if( m_dispatcher->getCurrentState() != static_cast<uint16_t>(LaraStateId::SlideBackward) || targetAngle != getCurrentSlideAngle() )
        {
            playAnimation(loader::AnimationId::START_SLIDE_BACKWARD, 1677);
            setTargetState(LaraStateId::SlideBackward);
            setStateOverride(LaraStateId::SlideBackward);
            setMovementAngle(targetAngle);
            setCurrentSlideAngle( targetAngle );
            setYRotation(targetAngle + util::degToAu(180));
        }
    }
    else if( m_dispatcher->getCurrentState() != static_cast<uint16_t>(LaraStateId::SlideForward) || targetAngle != getCurrentSlideAngle() )
    {
        playAnimation(loader::AnimationId::SLIDE_FORWARD, 1133);
        setTargetState(LaraStateId::SlideForward);
        setStateOverride(LaraStateId::SlideForward);
        setMovementAngle(targetAngle);
        setCurrentSlideAngle( targetAngle );
        setYRotation(targetAngle);
    }
    return true;
}

bool LaraStateHandler::tryGrabEdge(LaraState& state)
{
    if( state.axisCollisions != LaraState::AxisColl_InsufficientFrontSpace || !getInputState().action || getHandStatus() != 0 )
        return false;

    const auto floorGradient = std::abs(state.frontLeft.floor.distance - state.frontRight.floor.distance);
    if( floorGradient >= MaxGrabableGradient )
        return false;

    if( state.front.ceiling.distance > 0 || state.current.ceiling.distance > -ClimbLimit2ClickMin )
        return false;

    getLara()->updateAbsolutePosition();

    const auto spaceToReach = state.front.ceiling.distance - (loader::TRCoordinates(getLara()->getTransformedBoundingBox().MaxEdge).Y - getPosition().Y);

    if( spaceToReach < 0 && spaceToReach + getFallSpeed().getExact() < 0 )
        return false;
    if( spaceToReach > 0 && spaceToReach + getFallSpeed().getExact() > 0 )
        return false;

    int16_t rot = static_cast<int16_t>(getRotation().Y);
    if( std::abs(rot) <= util::degToAu(35) )
        rot = 0;
    else if( rot >= util::degToAu(90 - 35) && rot <= util::degToAu(90 + 35) )
        rot = util::degToAu(90);
    else if( std::abs(rot) >= util::degToAu(180 - 35) )
        rot = util::degToAu(180);
    else if( -rot >= util::degToAu(90 - 35) && -rot <= util::degToAu(90 + 35) )
        rot = util::degToAu(-90);
    else
        return false;

    setTargetState(LaraStateId::Hang);
    setStateOverride(LaraStateId::Hang);
    playAnimation(loader::AnimationId::HANG_IDLE, 1505);

    moveY(spaceToReach);
    applyCollisionFeedback(state);
    setHorizontalSpeed(0);
    setFallSpeed(0);
    setFalling(false);
    setHandStatus(1);
    setYRotation(rot);

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
            if( irr::core::equals(getPosition().Y, m_floorHeight, 1) )
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
    auto pos = getPosition();
    pos.Y += dy;
    auto sector = getLevel().findSectorForPosition(pos, getLevel().m_camera->getCurrentRoom());
    HeightInfo hi = HeightInfo::fromFloor(sector, pos, getLevel().m_camera);
    setFloorHeight(hi.distance);

    //! @todo Check room ownership change
}

int LaraStateHandler::getRelativeHeightAtDirection(int16_t angle, int dist) const
{
    auto pos = getPosition();
    pos.X += std::sin(util::auToRad(angle)) * dist;
    pos.Y -= ScalpHeight;
    pos.Z += std::cos(util::auToRad(angle)) * dist;

    auto sector = getLevel().findSectorForPosition(pos, getLevel().m_camera->getCurrentRoom());
    BOOST_ASSERT(sector != nullptr);

    HeightInfo h = HeightInfo::fromFloor(sector, pos, getLevel().m_camera);

    if( h.distance != -loader::HeightLimit )
        h.distance -= getPosition().Y;

    return h.distance;
}

void LaraStateHandler::commonJumpHandling(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededCeilingDistance = 192;
    state.yAngle = getMovementAngle();
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());
    checkJumpWallSmash(state);
    if( getFallSpeed().get() <= 0 || state.current.floor.distance > 0 )
        return;

    if( applyLandingDamage(state) )
        setTargetState(LaraStateId::Death);
    else
        setTargetState(LaraStateId::Stop);
    setFallSpeed(0);
    placeOnFloor(state);
    setFalling(false);
}

void LaraStateHandler::commonSlideHandling(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -loader::QuarterSectorSize * 2;
    state.neededCeilingDistance = 0;
    state.yAngle = getMovementAngle();
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());

    if( tryStopOnFloor(state) )
        return;

    checkWallCollision(state);
    if( state.current.floor.distance <= 200 )
    {
        tryStartSlide(state);
        placeOnFloor(state);
        const auto absSlantX = std::abs(state.floorSlantX);
        const auto absSlantZ = std::abs(state.floorSlantZ);
        if( absSlantX <= 2 && absSlantZ <= 2 )
        {
            setTargetState(LaraStateId::Stop);
        }
    }
    else
    {
        if( getCurrentState() == LaraStateId::SlideForward )
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

        setFallSpeed(0);
        setFalling(true);
    }
}

bool LaraStateHandler::applyLandingDamage(LaraState& /*state*/)
{
    auto sector = getLevel().findSectorForPosition(getPosition(), getLevel().m_camera->getCurrentRoom());
    HeightInfo h = HeightInfo::fromFloor(sector, getPosition() - loader::TRCoordinates{0, ScalpHeight, 0}, getLevel().m_camera);
    setFloorHeight(h.distance);
    handleTriggers(h.lastTriggerOrKill, false);
    auto damageSpeed = getFallSpeed().get() - 140;
    if( damageSpeed <= 0 )
        return false;

    static constexpr int DeathSpeedLimit = 14;

    if( damageSpeed <= DeathSpeedLimit )
        setHealth(getHealth() - 1000 * damageSpeed * damageSpeed / (DeathSpeedLimit * DeathSpeedLimit));
    else
        setHealth(-1);
    return getHealth() <= 0;
}

void LaraStateHandler::onInput52SwandiveBegin(LaraState& state)
{
    state.frobbelFlags &= ~FrobbelFlag10;
    state.frobbelFlags |= FrobbelFlag08;
    if( getFallSpeed().get() > FreeFallSpeedThreshold )
        setTargetState(LaraStateId::SwandiveEnd);
}

void LaraStateHandler::onBehave52SwandiveBegin(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededCeilingDistance = 192;
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());
    checkJumpWallSmash(state);
    if( state.current.floor.distance > 0 || getFallSpeed().get() <= 0 )
        return;

    setTargetState(LaraStateId::Stop);
    setFallSpeed(0);
    setFalling(false);
    placeOnFloor(state);
}

void LaraStateHandler::onInput53SwandiveEnd(LaraState& state)
{
    state.frobbelFlags &= ~FrobbelFlag10;
    state.frobbelFlags |= FrobbelFlag08;
    dampenHorizontalSpeed(5, 100);
}

void LaraStateHandler::onBehave53SwandiveEnd(LaraState& state)
{
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededCeilingDistance = 192;
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());
    checkJumpWallSmash(state);
    if( state.current.floor.distance > 0 || getFallSpeed().get() <= 0 )
        return;

    if( getFallSpeed().get() <= 133 )
        setTargetState(LaraStateId::Stop);
    else
        setTargetState(LaraStateId::Death);

    setFallSpeed(0);
    setFalling(false);
    placeOnFloor(state);
}

void LaraStateHandler::onBehave23RollBackward(LaraState& state)
{
    setFalling(false);
    setFallSpeed(0);
    state.yAngle = getRotation().Y + util::degToAu(180);
    setMovementAngle(state.yAngle);
    state.frobbelFlags |= FrobbelFlag_UnpassableSteepUpslant;
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());

    if( tryStopOnFloor(state) || tryStartSlide(state) )
        return;

    if( state.current.floor.distance <= 200 )
    {
        applyCollisionFeedback(state);
        placeOnFloor(state);
        return;
    }

    playAnimation(loader::AnimationId::FREE_FALL_BACK, 1473);
    setTargetState(LaraStateId::FallBackward);
    setStateOverride(LaraStateId::FallBackward);
    setFallSpeed(0);
    setFalling(true);
}

void LaraStateHandler::onBehave45RollForward(LaraState& state)
{
    setFalling(false);
    setFallSpeed(0);
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.frobbelFlags |= FrobbelFlag_UnpassableSteepUpslant;
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());

    if( tryStopOnFloor(state) || tryStartSlide(state) )
        return;

    if( state.current.floor.distance <= 200 )
    {
        applyCollisionFeedback(state);
        placeOnFloor(state);
        return;
    }

    playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
    setTargetState(LaraStateId::JumpForward);
    setStateOverride(LaraStateId::JumpForward);
    setFallSpeed(0);
    setFalling(true);
}

void LaraStateHandler::onInput26JumpLeft(LaraState& /*state*/)
{
    if( getFallSpeed().get() > FreeFallSpeedThreshold )
        setTargetState(LaraStateId::FreeFall);
}

void LaraStateHandler::onBehave26JumpLeft(LaraState& state)
{
    setMovementAngle(getRotation().Y + util::degToAu(90));
    commonJumpHandling(state);
}

void LaraStateHandler::onInput27JumpRight(LaraState& /*state*/)
{
    if( getFallSpeed().get() > FreeFallSpeedThreshold )
        setTargetState(LaraStateId::FreeFall);
}

void LaraStateHandler::onBehave27JumpRight(LaraState& state)
{
    setMovementAngle(getRotation().Y - util::degToAu(90));
    commonJumpHandling(state);
}

void LaraStateHandler::onInput11Reach(LaraState& /*state*/)
{
    if( getFallSpeed().get() > FreeFallSpeedThreshold )
        setTargetState(LaraStateId::FreeFall);
}

void LaraStateHandler::onBehave11Reach(LaraState& state)
{
    setFalling(true);
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.neededFloorDistanceBottom = loader::HeightLimit;
    state.neededFloorDistanceTop = 0;
    state.neededCeilingDistance = 192;
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());

    if( tryReach(state) )
        return;

    jumpAgainstWall(state);
    if( getFallSpeed().get() <= 0 || state.current.floor.distance > 0 )
        return;

    if( applyLandingDamage(state) )
        setTargetState(LaraStateId::Death);
    else
        setTargetState(LaraStateId::Stop);

    setFallSpeed(0);
    setFalling(false);
    placeOnFloor(state);
}

bool LaraStateHandler::tryReach(LaraState& state)
{
    if( state.axisCollisions != LaraState::AxisColl_InsufficientFrontSpace || !getInputState().action || getHandStatus() != 0 )
        return false;

    if( std::abs(state.frontLeft.floor.distance - state.frontRight.floor.distance) >= MaxGrabableGradient )
        return false;

    if( state.front.ceiling.distance > 0 || state.current.ceiling.distance > -ClimbLimit2ClickMin || state.current.floor.distance < 200 )
        return false;

    getLara()->updateAbsolutePosition();

    const auto spaceToReach = state.front.floor.distance - (loader::TRCoordinates(getLara()->getTransformedBoundingBox().MaxEdge).Y - getPosition().Y);
    BOOST_LOG_TRIVIAL(debug) << "spaceToReach=" << spaceToReach << ", fallSpeed=" << getFallSpeed().getExact();
    if( spaceToReach < 0 && spaceToReach + getFallSpeed().getExact() < 0 )
        return false;
    if( spaceToReach > 0 && spaceToReach + getFallSpeed().getExact() > 0 )
        return false;

    int16_t rot = static_cast<int16_t>(getRotation().Y);
    if( std::abs(rot) <= util::degToAu(35) )
        rot = 0;
    else if( rot >= util::degToAu(90 - 35) && rot <= util::degToAu(90 + 35) )
        rot = util::degToAu(90);
    else if( std::abs(rot) >= util::degToAu(180 - 35) )
        rot = util::degToAu(180);
    else if( -rot >= util::degToAu(90 - 35) && -rot <= util::degToAu(90 + 35) )
        rot = util::degToAu(-90);
    else
        return false;

    if( canClimbOnto(rot) )
        playAnimation(loader::AnimationId::OSCILLATE_HANG_ON, 3974);
    else
        playAnimation(loader::AnimationId::HANG_IDLE, 1493);

    setStateOverride(LaraStateId::Hang);
    setTargetState(LaraStateId::Hang);
    moveY(spaceToReach);
    setHorizontalSpeed(0);
    applyCollisionFeedback(state);
    setYRotation(rot);
    setFalling(false);
    setFallSpeed(0);
    setHandStatus(1);
    return true;
}

bool LaraStateHandler::canClimbOnto(int16_t angle) const
{
    auto pos = getPosition();
    if( angle == 0 )
        pos.Z += 256;
    else if( angle == util::degToAu(90) )
        pos.X += 256;
    else if( angle == util::degToAu(180) )
        pos.Z -= 256;
    else if( angle == util::degToAu(-90) )
        pos.X -= 256;

    auto sector = getLevel().findSectorForPosition(pos, getLevel().m_camera->getCurrentRoom());
    HeightInfo floor = HeightInfo::fromFloor(sector, pos, getLevel().m_camera);
    HeightInfo ceil = HeightInfo::fromCeiling(sector, pos, getLevel().m_camera);
    return floor.distance != -loader::HeightLimit && floor.distance - pos.Y > 0 && ceil.distance - pos.Y < -400;
}

void LaraStateHandler::onBehaveStanding(LaraState& state)
{
    setFallSpeed(0);
    setFalling(false);
    state.yAngle = getRotation().Y;
    setMovementAngle(state.yAngle);
    state.neededFloorDistanceTop = -ClimbLimit2ClickMin;
    state.neededFloorDistanceBottom = ClimbLimit2ClickMin;
    state.neededCeilingDistance = 0;
    state.frobbelFlags |= FrobbelFlag_UnpassableSteepUpslant | FrobbelFlag_UnwalkableSteepFloor;
    state.initHeightInfo(this, getLevel(), ScalpHeight, getPosition());
    if( tryStopOnFloor(state) )
        return;

    if( state.current.floor.distance <= 100 )
    {
        if( !tryStartSlide(state) )
        {
            applyCollisionFeedback(state);
            placeOnFloor(state);
        }
    }
    else
    {
        playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
        setTargetState(LaraStateId::JumpForward);
        setFallSpeed(0);
        setFalling(true);
    }
}
