#include "cameracontroller.h"

#include "laranode.h"
#include "level/level.h"
#include "render/portaltracer.h"

#include <queue>
#include <utility>

namespace engine
{
namespace
{
const char* toString(const CameraMode mode)
{
    switch( mode )
    {

        case CameraMode::Chase:
            return "Chase";
        case CameraMode::Fixed:
            return "Fixed";
        case CameraMode::FreeLook:
            return "FreeLook";
        case CameraMode::Combat:
            return "Combat";
        case CameraMode::Cinematic:
            return "Cinematic";
        case CameraMode::Heavy:
            return "Heavy";
        default:
            BOOST_THROW_EXCEPTION( std::domain_error( "Invalid CameraMode" ) );
    }
}

CameraMode parseCameraMode(const std::string& m)
{
    if( m == "Chase" )
        return CameraMode::Chase;
    if( m == "Fixed" )
        return CameraMode::Fixed;
    if( m == "FreeLook" )
        return CameraMode::FreeLook;
    if( m == "Combat" )
        return CameraMode::Combat;
    if( m == "Cinematic" )
        return CameraMode::Cinematic;
    if( m == "Heavy" )
        return CameraMode::Heavy;
    BOOST_THROW_EXCEPTION( std::domain_error( "Invalid CameraMode" ) );
}

const char* toString(const CameraModifier mode)
{
    switch( mode )
    {

        case CameraModifier::None:
            return "None";
        case CameraModifier::FollowCenter:
            return "FollowCenter";
        case CameraModifier::AllowSteepSlants:
            return "AllowSteepSlants";
        case CameraModifier::Chase:
            return "Chase";
        default:
            BOOST_THROW_EXCEPTION( std::domain_error( "Invalid CameraModifier" ) );
    }
}

CameraModifier parseCameraModifier(const std::string& m)
{
    if( m == "None" )
        return CameraModifier::None;
    if( m == "FollowCenter" )
        return CameraModifier::FollowCenter;
    if( m == "AllowSteepSlants" )
        return CameraModifier::AllowSteepSlants;
    if( m == "Chase" )
        return CameraModifier::Chase;
    BOOST_THROW_EXCEPTION( std::domain_error( "Invalid CameraModifier" ) );
}
}

CameraController::CameraController(const gsl::not_null<level::Level*>& level,
                                   gsl::not_null<std::shared_ptr<gameplay::Camera>> camera)
        : Listener{&level->m_soundEngine}
        , m_camera{std::move( camera )}
        , m_level{level}
        , m_eye{level->m_lara->m_state.position.room}
        , m_center{level->m_lara->m_state.position.room, level->m_lara->m_state.position.position}
        , m_eyeYOffset{level->m_lara->m_state.position.position.Y - core::SectorSize}
{
    Expects( level->m_lara != nullptr );

    m_center.position.Y -= m_eyeYOffset;
    m_eye = m_center;
    m_eye.position.Z -= 100_len;

    update();
}

void CameraController::setRotationAroundCenter(const core::Angle x, const core::Angle y)
{
    setRotationAroundCenterX( x );
    setRotationAroundCenterY( y );
}

void CameraController::setRotationAroundCenterX(const core::Angle x)
{
    m_rotationAroundCenter.X = x;
}

void CameraController::setRotationAroundCenterY(const core::Angle y)
{
    m_rotationAroundCenter.Y = y;
}

void CameraController::setCamOverride(const floordata::CameraParameters& camParams,
                                      const uint16_t camId,
                                      const floordata::SequenceCondition condition,
                                      const bool fromHeavy,
                                      const core::Frame timeout,
                                      const bool switchIsOn)
{
    Expects( camId < m_level->m_cameras.size() );
    if( m_level->m_cameras[camId].isActive() )
        return;

    m_fixedCameraId = camId;
    if( m_mode == CameraMode::FreeLook || m_mode == CameraMode::Combat
        || condition == floordata::SequenceCondition::LaraInCombatMode )
        return;

    if( condition == floordata::SequenceCondition::ItemActivated && timeout != 0_frame && switchIsOn )
        return;

    if( condition != floordata::SequenceCondition::ItemActivated && m_fixedCameraId == m_currentFixedCameraId )
        return;

    if( camParams.timeout != 1_sec )
        m_camOverrideTimeout = camParams.timeout * core::FrameRate;

    if( camParams.oneshot )
        m_level->m_cameras[camId].setActive( true );

    m_smoothness = 1 + (camParams.smoothness * 4);
    if( fromHeavy )
        m_mode = CameraMode::Heavy;
    else
        m_mode = CameraMode::Fixed;
}

void CameraController::handleCommandSequence(const engine::floordata::FloorDataValue* cmdSequence)
{
    if( m_mode == CameraMode::Heavy )
        return;

    enum class Type
    {
        Invalid, FixedCamChange, NoChange
    };

    Type type = Type::NoChange;
    while( true )
    {
        const floordata::Command command{*cmdSequence++};

        if( command.opcode == floordata::CommandOpcode::LookAt
            && m_mode != CameraMode::FreeLook
            && m_mode != CameraMode::Combat )
        {
            m_item = m_level->getItem( command.parameter );
        }
        else if( command.opcode == floordata::CommandOpcode::SwitchCamera )
        {
            ++cmdSequence; // skip camera parameters

            if( command.parameter != m_currentFixedCameraId )
            {
                type = Type::Invalid; // new override
            }
            else
            {
                m_fixedCameraId = m_currentFixedCameraId;
                if( m_camOverrideTimeout >= 0_frame && m_mode != CameraMode::FreeLook && m_mode != CameraMode::Combat )
                {
                    type = Type::FixedCamChange;
                    m_mode = CameraMode::Fixed;
                }
                else
                {
                    type = Type::Invalid;
                    m_camOverrideTimeout = -1_frame;
                }
            }
        }

        if( command.isLast )
            break;
    }

    if( m_item == nullptr )
        return;

    if( type == Type::NoChange && m_item->m_state.already_looked_at && m_item != m_previousItem )
        m_item = nullptr;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void CameraController::tracePortals()
{
    for( const loader::Room& room : m_level->m_rooms )
        room.node->setVisible( false );

    auto startRoom = m_eye.room;
    startRoom->node->setVisible( true );

    // Breadth-first queue
    std::queue<render::PortalTracer> toVisit;

    // always process direct neighbors of the starting room
    for( const loader::Portal& portal : startRoom->portals )
    {
        render::PortalTracer path;
        if( !path.checkVisibility( &portal, *m_camera.get() ) )
            continue;

        m_level->m_rooms.at( portal.adjoining_room.get() ).node->setVisible( true );

        toVisit.emplace( path );
    }

    // Avoid infinite loops
    std::set<const loader::Portal*> visited;
    while( !toVisit.empty() )
    {
        const render::PortalTracer currentPath = toVisit.front();
        toVisit.pop();

        if( !visited.insert( currentPath.getLastPortal() ).second )
        {
            continue; // already tested
        }

        // iterate through the last room's portals and add the destinations if suitable
        const auto destRoom = currentPath.getLastDestinationRoom();
        for( const loader::Portal& srcPortal : m_level->m_rooms.at( destRoom.get() ).portals )
        {
            render::PortalTracer newPath = currentPath;
            if( !newPath.checkVisibility( &srcPortal, *m_camera.get() ) )
                continue;

            m_level->m_rooms.at( srcPortal.adjoining_room.get() ).node->setVisible( true );
            toVisit.emplace( newPath );
        }
    }
}

bool CameraController::clampY(const core::TRVec& start,
                              core::TRVec& end,
                              const gsl::not_null<const loader::Sector*>& sector,
                              const level::Level& level)
{
    const HeightInfo floor = HeightInfo::fromFloor( sector, end, level.m_itemNodes );
    const HeightInfo ceiling = HeightInfo::fromCeiling( sector, end, level.m_itemNodes );

    const auto d = end - start;
    if( floor.y < end.Y && floor.y > start.Y )
    {
        end.Y = floor.y;
        end.X = d.X * (floor.y - start.Y) / d.Y + start.X;
        end.Z = d.Z * (floor.y - start.Y) / d.Y + start.Z;
        return false;
    }

    if( ceiling.y > end.Y && ceiling.y < start.Y )
    {
        end.Y = ceiling.y;
        end.X = d.X * (ceiling.y - start.Y) / d.Y + start.X;
        end.Z = d.Z * (ceiling.y - start.Y) / d.Y + start.Z;
        return false;
    }

    return true;
}

CameraController::ClampType CameraController::clampAlongX(const core::RoomBoundPosition& start,
                                                          core::RoomBoundPosition& end,
                                                          const level::Level& level)
{
    if( end.position.X == start.position.X )
    {
        return ClampType::None;
    }

    const auto d = end.position - start.position;

    const auto sign = d.X < 0_len ? -1 : 1;

    core::TRVec testPos;
    testPos.X = (start.position.X / core::SectorSize) * core::SectorSize;
    if( sign > 0 )
        testPos.X += core::SectorSize - 1_len;

    testPos.Y = start.position.Y + (testPos.X - start.position.X) * d.Y / d.X;
    testPos.Z = start.position.Z + (testPos.X - start.position.X) * d.Z / d.X;

    core::TRVec step;
    step.X = sign * core::SectorSize;
    step.Y = step.X * d.Y / d.X;
    step.Z = step.X * d.Z / d.X;

    auto room = start.room;

    while( true )
    {
        if( sign > 0 && testPos.X >= end.position.X )
        {
            end.room = room;
            return ClampType::None;
        }
        if( sign < 0 && testPos.X <= end.position.X )
        {
            end.room = room;
            return ClampType::None;
        }

        core::TRVec heightPos = testPos;
        auto sector = level::Level::findRealFloorSector( heightPos, &room );
        if( testPos.Y > HeightInfo::fromFloor( sector, heightPos, level.m_itemNodes ).y
            || testPos.Y < HeightInfo::fromCeiling( sector, heightPos, level.m_itemNodes ).y )
        {
            end.position = testPos;
            end.room = room;
            return ClampType::Ceiling;
        }

        heightPos.X = testPos.X + sign * 1_len;
        sector = level::Level::findRealFloorSector( heightPos, room );
        if( testPos.Y > HeightInfo::fromFloor( sector, heightPos, level.m_itemNodes ).y
            || testPos.Y < HeightInfo::fromCeiling( sector, heightPos, level.m_itemNodes ).y )
        {
            end.position = testPos;
            end.room = room;
            return ClampType::Wall;
        }

        testPos += step;
    }
}

CameraController::ClampType CameraController::clampAlongZ(const core::RoomBoundPosition& start,
                                                          core::RoomBoundPosition& end,
                                                          const level::Level& level)
{
    if( end.position.Z == start.position.Z )
    {
        return ClampType::None;
    }

    const auto d = end.position - start.position;

    const auto sign = d.Z < 0_len ? -1 : 1;

    core::TRVec testPos;
    testPos.Z = (start.position.Z / core::SectorSize) * core::SectorSize;
    if( sign > 0 )
        testPos.Z += core::SectorSize - 1_len;

    testPos.X = start.position.X + (testPos.Z - start.position.Z) * d.X / d.Z;
    testPos.Y = start.position.Y + (testPos.Z - start.position.Z) * d.Y / d.Z;

    core::TRVec step;
    step.Z = sign * core::SectorSize;
    step.X = step.Z * d.X / d.Z;
    step.Y = step.Z * d.Y / d.Z;

    auto room = start.room;

    while( true )
    {
        if( sign > 0 && testPos.Z >= end.position.Z )
        {
            end.room = room;
            return ClampType::None;
        }
        if( sign < 0 && testPos.Z <= end.position.Z )
        {
            end.room = room;
            return ClampType::None;
        }

        core::TRVec heightPos = testPos;
        auto sector = level::Level::findRealFloorSector( heightPos, &room );
        if( testPos.Y > HeightInfo::fromFloor( sector, heightPos, level.m_itemNodes ).y
            || testPos.Y < HeightInfo::fromCeiling( sector, heightPos, level.m_itemNodes ).y )
        {
            end.position = testPos;
            end.room = room;
            return ClampType::Ceiling;
        }

        heightPos.Z = testPos.Z + sign * 1_len;
        sector = level::Level::findRealFloorSector( heightPos, room );
        if( testPos.Y > HeightInfo::fromFloor( sector, heightPos, level.m_itemNodes ).y
            || testPos.Y < HeightInfo::fromCeiling( sector, heightPos, level.m_itemNodes ).y )
        {
            end.position = testPos;
            end.room = room;
            return ClampType::Wall;
        }

        testPos += step;
    }
}

bool CameraController::clampPosition(const core::RoomBoundPosition& start,
                                     core::RoomBoundPosition& end,
                                     const level::Level& level)
{
    bool firstUnclamped;
    ClampType secondClamp;
    if( abs( end.position.Z - start.position.Z ) <= abs( end.position.X - start.position.X ) )
    {
        firstUnclamped = clampAlongZ( start, end, level ) == ClampType::None;
        secondClamp = clampAlongX( start, end, level );
    }
    else
    {
        firstUnclamped = clampAlongX( start, end, level ) == ClampType::None;
        secondClamp = clampAlongZ( start, end, level );
    }

    if( secondClamp == ClampType::Wall )
    {
        return false;
    }

    const auto sector = level::Level::findRealFloorSector( end );
    return clampY( start.position, end.position, sector, level ) && firstUnclamped && secondClamp == ClampType::None;
}

void CameraController::update()
{
    m_rotationAroundCenter.X = util::clamp( m_rotationAroundCenter.X, -85_deg, +85_deg );

    if( m_mode == CameraMode::Cinematic )
    {
        if( ++m_cinematicFrame >= m_level->m_cinematicFrames.size() )
        {
            m_cinematicFrame = m_level->m_cinematicFrames.size() - 1;
        }

        updateCinematic( m_level->m_cinematicFrames[m_cinematicFrame], true );
        tracePortals();
        return;
    }

    if( m_modifier != CameraModifier::AllowSteepSlants )
        HeightInfo::skipSteepSlants = true;

    const bool fixed = m_item != nullptr && (m_mode == CameraMode::Fixed || m_mode == CameraMode::Heavy);
    const bool fixedCenterOnly = m_item != nullptr && !(m_mode == CameraMode::Fixed || m_mode == CameraMode::Heavy);

    // if we have a fixed position, we also have an item we're looking at
    std::shared_ptr<items::ItemNode> focusedItem = fixed ? m_item : m_level->m_lara;
    BOOST_ASSERT( focusedItem != nullptr );
    auto focusBBox = focusedItem->getBoundingBox();
    auto focusY = focusedItem->m_state.position.position.Y;
    if( fixed )
        focusY += (focusBBox.minY + focusBBox.maxY) / 2;
    else
        focusY += (focusBBox.minY - focusBBox.maxY) * 3 / 4 + focusBBox.maxY;

    if( fixedCenterOnly )
    {
        // lara moves around and looks at some item, some sort of involuntary free look;
        // in this case, we have an item to look at, but the camera is _not_ fixed

        BOOST_ASSERT( m_item != focusedItem );
        BOOST_ASSERT( focusedItem != nullptr );
        const auto distToFocused = m_item->m_state.position.position
                                         .distanceTo( focusedItem->m_state.position.position );
        auto eyeRotY =
                core::Angle::fromAtan( m_item->m_state.position.position.X - focusedItem->m_state.position.position.X,
                                       m_item->m_state.position.position.Z - focusedItem->m_state.position.position.Z )
                - focusedItem->m_state.rotation.Y;
        eyeRotY *= 0.5f;
        focusBBox = m_item->getBoundingBox();
        auto eyeRotX = core::Angle::fromAtan( distToFocused,
                                              focusY - (focusBBox.minY + focusBBox.maxY) / 2
                                              + m_item->m_state.position.position.Y );
        eyeRotX *= 0.5f;

        if( eyeRotY < 50_deg && eyeRotY > -50_deg && eyeRotX < 85_deg && eyeRotX > -85_deg )
        {
            eyeRotY -= m_level->m_lara->m_headRotation.Y;
            m_level->m_lara->m_headRotation.Y += util::clamp( eyeRotY, -4_deg, +4_deg );
            m_level->m_lara->m_torsoRotation.Y = m_level->m_lara->m_headRotation.Y;

            eyeRotX -= m_level->m_lara->m_headRotation.X;
            m_level->m_lara->m_headRotation.X += util::clamp( eyeRotX, -4_deg, +4_deg );
            m_level->m_lara->m_torsoRotation.X = m_level->m_lara->m_headRotation.X;

            m_mode = CameraMode::FreeLook;
            m_item->m_state.already_looked_at = true;
        }
    }

    m_center.room = focusedItem->m_state.position.room;

    if( m_mode == CameraMode::FreeLook || m_mode == CameraMode::Combat )
    {
        if( m_fixed )
        {
            m_center.position.Y = focusY - core::QuarterSectorSize;
            m_smoothness = 1;
        }
        else
        {
            m_center.position.Y += (focusY - core::QuarterSectorSize - m_center.position.Y) / 4;
            if( m_mode == CameraMode::FreeLook )
                m_smoothness = 4;
            else
                m_smoothness = 8;
        }
        m_fixed = false;
        if( m_mode == CameraMode::FreeLook )
            handleFreeLook( *focusedItem );
        else
            handleEnemy( *focusedItem );
    }
    else
    {
        m_center.position.X = focusedItem->m_state.position.position.X;
        m_center.position.Z = focusedItem->m_state.position.position.Z;

        if( m_modifier == CameraModifier::FollowCenter )
        {
            const auto midZ = (focusBBox.minZ + focusBBox.maxZ) / 2;
            m_center.position += util::pitch( midZ, focusedItem->m_state.rotation.Y );
        }

        if( m_fixed == fixed )
        {
            m_fixed = false;
            m_center.position.Y += (focusY - m_center.position.Y) / 4;
        }
        else
        {
            // switching between fixed cameras, so we're not doing any smoothing
            m_fixed = true;
            m_center.position.Y = focusY;
            m_smoothness = 1;
        }

        const auto sector = level::Level::findRealFloorSector( m_center );
        if( HeightInfo::fromFloor( sector, m_center.position, getLevel()->m_itemNodes ).y < m_center.position.Y )
            HeightInfo::skipSteepSlants = false;

        if( m_mode == CameraMode::Chase || m_modifier == CameraModifier::Chase )
            chaseItem( focusedItem );
        else
            handleFixedCamera();
    }

    m_fixed = fixed;
    m_currentFixedCameraId = m_fixedCameraId;
    if( m_mode != CameraMode::Heavy || m_camOverrideTimeout < 0_frame )
    {
        m_modifier = CameraModifier::None;
        m_mode = CameraMode::Chase;
        m_previousItem = std::exchange( m_item, nullptr );
        m_rotationAroundCenter.X = m_rotationAroundCenter.Y = 0_deg;
        m_eyeCenterDistance = core::DefaultCameraLaraDistance;
        m_fixedCameraId = -1;
    }
    HeightInfo::skipSteepSlants = false;

    tracePortals();
}

void CameraController::handleFixedCamera()
{
    Expects( m_fixedCameraId >= 0 );

    const loader::Camera& camera = m_level->m_cameras.at( m_fixedCameraId );
    core::RoomBoundPosition pos( &m_level->m_rooms.at( camera.room ) );
    pos.position = camera.position;

    if( !clampPosition( m_center, pos, *m_level ) )
    {
        // ReSharper disable once CppExpressionWithoutSideEffects
        moveIntoGeometry( pos, core::QuarterSectorSize );
    }

    m_fixed = true;
    updatePosition( pos, m_smoothness );

    if( m_camOverrideTimeout != 0_frame )
    {
        m_camOverrideTimeout -= 1_frame;
        if( m_camOverrideTimeout == 0_frame )
            m_camOverrideTimeout = -1_frame;
    }
}

core::Length CameraController::moveIntoGeometry(core::RoomBoundPosition& pos, const core::Length& margin) const
{
    const auto sector = level::Level::findRealFloorSector( pos );
    BOOST_ASSERT( sector->box != nullptr );

    const auto room = pos.room;

    if( sector->box->zmin + margin > pos.position.Z
        && isVerticallyOutsideRoom( pos.position - core::TRVec( 0_len, 0_len, margin ), room ) )
        pos.position.Z = sector->box->zmin + margin;
    else if( sector->box->zmax - margin > pos.position.Z
             && isVerticallyOutsideRoom( pos.position + core::TRVec( 0_len, 0_len, margin ), room ) )
        pos.position.Z = sector->box->zmax - margin;

    if( sector->box->xmin + margin > pos.position.X
        && isVerticallyOutsideRoom( pos.position - core::TRVec( margin, 0_len, 0_len ), room ) )
        pos.position.X = sector->box->xmin + margin;
    else if( sector->box->xmax - margin > pos.position.X
             && isVerticallyOutsideRoom( pos.position + core::TRVec( margin, 0_len, 0_len ), room ) )
        pos.position.X = sector->box->xmax - margin;

    auto bottom = HeightInfo::fromFloor( sector, pos.position, getLevel()->m_itemNodes ).y - margin;
    auto top = HeightInfo::fromCeiling( sector, pos.position, getLevel()->m_itemNodes ).y + margin;
    if( bottom < top )
        top = bottom = (bottom + top) / 2;

    if( pos.position.Y > bottom )
        return bottom - pos.position.Y;
    if( top > pos.position.Y )
        return top - pos.position.Y;
    return 0_len;
}

bool CameraController::isVerticallyOutsideRoom(const core::TRVec& pos,
                                               const gsl::not_null<const loader::Room*>& room) const
{
    const auto sector = level::Level::findRealFloorSector( pos, room );
    const auto floor = HeightInfo::fromFloor( sector, pos, getLevel()->m_itemNodes ).y;
    const auto ceiling = HeightInfo::fromCeiling( sector, pos, getLevel()->m_itemNodes )
            .y;
    return pos.Y > floor || pos.Y <= ceiling;
}

void CameraController::updatePosition(const core::RoomBoundPosition& eyePositionGoal, const int smoothFactor)
{
    m_eye.position += (eyePositionGoal.position - m_eye.position) / smoothFactor;
    HeightInfo::skipSteepSlants = false;
    m_eye.room = eyePositionGoal.room;
    auto sector = level::Level::findRealFloorSector( m_eye );
    auto floor = HeightInfo::fromFloor( sector, m_eye.position, getLevel()->m_itemNodes )
                         .y - core::QuarterSectorSize;
    if( floor <= m_eye.position.Y && floor <= eyePositionGoal.position.Y )
    {
        clampPosition( m_center, m_eye, *m_level );
        sector = level::Level::findRealFloorSector( m_eye );
        floor = HeightInfo::fromFloor( sector, m_eye.position, getLevel()->m_itemNodes )
                        .y - core::QuarterSectorSize;
    }

    auto ceiling =
            HeightInfo::fromCeiling( sector, m_eye.position, getLevel()->m_itemNodes )
                    .y + core::QuarterSectorSize;
    if( floor < ceiling )
    {
        floor = ceiling = (floor + ceiling) / 2;
    }

    if( m_bounce < 0_len )
    {
        const core::TRVec tmp{
                util::rand15s( m_bounce, core::Length::type() ),
                util::rand15s( m_bounce, core::Length::type() ),
                util::rand15s( m_bounce, core::Length::type() )
        };
        m_eye.position += tmp;
        m_center.position += tmp;
        m_bounce += 5_len;
    }
    else if( m_bounce > 0_len )
    {
        m_eye.position.Y += m_bounce;
        m_center.position.Y += m_bounce;
        m_bounce = 0_len;
    }

    if( m_eye.position.Y > floor )
        m_eyeYOffset = floor - m_eye.position.Y;
    else if( m_eye.position.Y < ceiling )
        m_eyeYOffset = ceiling - m_eye.position.Y;
    else
        m_eyeYOffset = 0_len;

    auto camPos = m_eye.position;
    camPos.Y += m_eyeYOffset;

    // update current room
    level::Level::findRealFloorSector( camPos, &m_eye.room );

    const auto m = lookAt( camPos.toRenderSystem(), m_center.position.toRenderSystem(), {0, 1, 0} );
    m_camera->setViewMatrix( m );
}

void CameraController::chaseItem(const gsl::not_null<std::shared_ptr<const items::ItemNode>>& item)
{
    m_rotationAroundCenter.X += item->m_state.rotation.X;
    if( m_rotationAroundCenter.X > 85_deg )
        m_rotationAroundCenter.X = 85_deg;
    else if( m_rotationAroundCenter.X < -85_deg )
        m_rotationAroundCenter.X = -85_deg;

    const auto dist = util::cos( m_eyeCenterDistance, m_rotationAroundCenter.X );
    m_eyeCenterHorizontalDistanceSq = util::square( dist );

    core::RoomBoundPosition eye( m_eye.room );
    eye.position = {
            0_len,
            util::sin( m_eyeCenterDistance, m_rotationAroundCenter.X ),
            0_len
    };

    core::Angle y = m_rotationAroundCenter.Y + item->m_state.rotation.Y;
    eye.position += m_center.position - util::pitch( dist, y );
    clampBox( eye,
              [this](core::Length& a, core::Length& b, const core::Length c, const core::Length d, const core::Length e,
                     const core::Length f, const core::Length g, const core::Length h) {
                  clampToCorners( m_eyeCenterHorizontalDistanceSq, a, b, c, d, e, f, g, h );
              } );

    updatePosition( eye, m_fixed ? m_smoothness : 12 );
}

void CameraController::handleFreeLook(const items::ItemNode& item)
{
    const auto originalCenter = m_center.position;
    m_center.position.X = item.m_state.position.position.X;
    m_center.position.Z = item.m_state.position.position.Z;
    m_rotationAroundCenter.X = m_level->m_lara->m_torsoRotation.X + m_level->m_lara->m_headRotation.X
                               + item.m_state.rotation.X;
    m_rotationAroundCenter.Y = m_level->m_lara->m_torsoRotation.Y + m_level->m_lara->m_headRotation.Y
                               + item.m_state.rotation.Y;
    m_eyeCenterDistance = core::DefaultCameraLaraDistance;
    m_eyeYOffset = -util::sin( core::SectorSize / 2, m_rotationAroundCenter.Y );
    m_center.position += util::pitch( m_eyeYOffset, item.m_state.rotation.Y );

    if( isVerticallyOutsideRoom( m_center.position, m_eye.room ) )
    {
        m_center.position.X = item.m_state.position.position.X;
        m_center.position.Z = item.m_state.position.position.Z;
    }

    m_center.position.Y += moveIntoGeometry( m_center, core::CameraWallDistance );

    auto center = m_center;
    center.position -= util::pitch( m_eyeCenterDistance, m_rotationAroundCenter.Y );
    center.position.Y += util::sin( m_eyeCenterDistance, m_rotationAroundCenter.X );
    center.room = m_eye.room;

    clampBox( center, &freeLookClamp );

    m_center.position.X = originalCenter.X
                          + (m_center.position.X - originalCenter.X) / m_smoothness;
    m_center.position.Z = originalCenter.Z
                          + (m_center.position.Z - originalCenter.Z) / m_smoothness;

    updatePosition( center, m_smoothness );
}

void CameraController::handleEnemy(const items::ItemNode& item)
{
    m_center.position.X = item.m_state.position.position.X;
    m_center.position.Z = item.m_state.position.position.Z;

    if( m_enemy != nullptr )
    {
        m_rotationAroundCenter.X = m_eyeRotation.X + item.m_state.rotation.X;
        m_rotationAroundCenter.Y = m_eyeRotation.Y + item.m_state.rotation.Y;
    }
    else
    {
        m_rotationAroundCenter.X = m_level->m_lara->m_torsoRotation.X + m_level->m_lara->m_headRotation.X
                                   + item.m_state.rotation.X;
        m_rotationAroundCenter.Y = m_level->m_lara->m_torsoRotation.Y + m_level->m_lara->m_headRotation.Y
                                   + item.m_state.rotation.Y;
    }

    m_eyeCenterDistance = core::CombatCameraLaraDistance;
    auto eye = m_center;
    const auto d = util::cos( m_eyeCenterDistance, m_rotationAroundCenter.X );
    eye.position -= util::pitch( d.retype_as<core::Length>(), m_rotationAroundCenter.Y );
    eye.position.Y += util::sin( m_eyeCenterDistance, m_rotationAroundCenter.X );
    eye.room = m_eye.room;

    clampBox( eye,
              [this](core::Length& a,
                     core::Length& b,
                     const core::Length c,
                     const core::Length d,
                     const core::Length e,
                     const core::Length f,
                     const core::Length g,
                     const core::Length h) {
                  clampToCorners( m_eyeCenterHorizontalDistanceSq, a, b, c, d, e, f, g, h );
              } );
    updatePosition( eye, m_smoothness );
}

void
CameraController::clampBox(core::RoomBoundPosition& eyePositionGoal, const std::function<ClampCallback>& callback) const
{
    clampPosition( m_center, eyePositionGoal, *m_level );
    BOOST_ASSERT( m_center.room->getSectorByAbsolutePosition( m_center.position ) != nullptr );
    auto clampBox = m_center.room->getSectorByAbsolutePosition( m_center.position )->box;
    BOOST_ASSERT( clampBox != nullptr );
    BOOST_ASSERT( eyePositionGoal.room->getSectorByAbsolutePosition( eyePositionGoal.position ) != nullptr );
    if( const auto idealBox = eyePositionGoal.room->getSectorByAbsolutePosition( eyePositionGoal.position )->box )
    {
        if( !clampBox->contains( eyePositionGoal.position.X, eyePositionGoal.position.Z ) )
            clampBox = idealBox;
    }

    core::TRVec testPos = eyePositionGoal.position;
    testPos.Z = (testPos.Z / core::SectorSize) * core::SectorSize - 1_len;
    BOOST_ASSERT( testPos.Z % core::SectorSize == core::SectorSize - 1_len
                  && abs( testPos.Z - eyePositionGoal.position.Z ) <= core::SectorSize );

    auto clampZMin = clampBox->zmin;
    const bool negZVerticallyOutside = isVerticallyOutsideRoom( testPos, eyePositionGoal.room );
    if( !negZVerticallyOutside && level::Level::findRealFloorSector( testPos, eyePositionGoal.room )->box != nullptr )
    {
        const auto testBox = level::Level::findRealFloorSector( testPos, eyePositionGoal.room )->box;
        if( testBox->zmin < clampZMin )
            clampZMin = testBox->zmin;
    }
    clampZMin += core::QuarterSectorSize;

    testPos = eyePositionGoal.position;
    testPos.Z = (testPos.Z / core::SectorSize + 1) * core::SectorSize;
    BOOST_ASSERT( testPos.Z % core::SectorSize == 0_len
                  && abs( testPos.Z - eyePositionGoal.position.Z ) <= core::SectorSize );

    auto clampZMax = clampBox->zmax;
    const bool posZVerticallyOutside = isVerticallyOutsideRoom( testPos, eyePositionGoal.room );
    if( !posZVerticallyOutside && level::Level::findRealFloorSector( testPos, eyePositionGoal.room )->box != nullptr )
    {
        const auto testBox = level::Level::findRealFloorSector( testPos, eyePositionGoal.room )->box;
        if( testBox->zmax > clampZMax )
            clampZMax = testBox->zmax;
    }
    clampZMax -= core::QuarterSectorSize;

    testPos = eyePositionGoal.position;
    testPos.X = (testPos.X / core::SectorSize) * core::SectorSize - 1_len;
    BOOST_ASSERT( testPos.X % core::SectorSize == core::SectorSize - 1_len
                  && abs( testPos.X - eyePositionGoal.position.X ) <= core::SectorSize );

    auto clampXMin = clampBox->xmin;
    const bool negXVerticallyOutside = isVerticallyOutsideRoom( testPos, eyePositionGoal.room );
    if( !negXVerticallyOutside && level::Level::findRealFloorSector( testPos, eyePositionGoal.room )->box != nullptr )
    {
        const auto testBox = level::Level::findRealFloorSector( testPos, eyePositionGoal.room )->box;
        if( testBox->xmin < clampXMin )
            clampXMin = testBox->xmin;
    }
    clampXMin += core::QuarterSectorSize;

    testPos = eyePositionGoal.position;
    testPos.X = (testPos.X / core::SectorSize + 1) * core::SectorSize;
    BOOST_ASSERT( testPos.X % core::SectorSize == 0_len
                  && abs( testPos.X - eyePositionGoal.position.X ) <= core::SectorSize );

    auto clampXMax = clampBox->xmax;
    const bool posXVerticallyOutside = isVerticallyOutsideRoom( testPos, eyePositionGoal.room );
    if( !posXVerticallyOutside && level::Level::findRealFloorSector( testPos, eyePositionGoal.room )->box != nullptr )
    {
        const auto testBox = level::Level::findRealFloorSector( testPos, eyePositionGoal.room )->box;
        if( testBox->xmax > clampXMax )
            clampXMax = testBox->xmax;
    }
    clampXMax -= core::QuarterSectorSize;

    bool skipRoomPatch = true;
    if( negZVerticallyOutside && eyePositionGoal.position.Z < clampZMin )
    {
        skipRoomPatch = false;
        core::Length left = 0_len, right = 0_len;
        if( eyePositionGoal.position.X >= m_center.position.X )
        {
            left = clampXMin;
            right = clampXMax;
        }
        else
        {
            left = clampXMax;
            right = clampXMin;
        }
        callback( eyePositionGoal.position.Z, eyePositionGoal.position.X, m_center.position.Z, m_center.position.X,
                  clampZMin,
                  right, clampZMax, left );
    }
    else if( posZVerticallyOutside && eyePositionGoal.position.Z > clampZMax )
    {
        skipRoomPatch = false;
        core::Length left = 0_len, right = 0_len;
        if( eyePositionGoal.position.X >= m_center.position.X )
        {
            left = clampXMin;
            right = clampXMax;
        }
        else
        {
            left = clampXMax;
            right = clampXMin;
        }
        callback( eyePositionGoal.position.Z, eyePositionGoal.position.X, m_center.position.Z, m_center.position.X,
                  clampZMax,
                  right, clampZMin, left );
    }

    if( !skipRoomPatch )
    {
        // ReSharper disable once CppExpressionWithoutSideEffects
        level::Level::findRealFloorSector( eyePositionGoal );
        return;
    }

    if( negXVerticallyOutside && eyePositionGoal.position.X < clampXMin )
    {
        skipRoomPatch = false;
        core::Length left = 0_len, right = 0_len;
        if( eyePositionGoal.position.Z >= m_center.position.Z )
        {
            left = clampZMin;
            right = clampZMax;
        }
        else
        {
            left = clampZMax;
            right = clampZMin;
        }
        callback( eyePositionGoal.position.X, eyePositionGoal.position.Z, m_center.position.X, m_center.position.Z,
                  clampXMin,
                  right, clampXMax, left );
    }
    else if( posXVerticallyOutside && eyePositionGoal.position.X > clampXMax )
    {
        skipRoomPatch = false;
        core::Length left = 0_len, right = 0_len;
        if( eyePositionGoal.position.Z >= m_center.position.Z )
        {
            left = clampZMin;
            right = clampZMax;
        }
        else
        {
            left = clampZMax;
            right = clampZMin;
        }
        callback( eyePositionGoal.position.X, eyePositionGoal.position.Z, m_center.position.X, m_center.position.Z,
                  clampXMax,
                  right, clampXMin, left );
    }

    if( !skipRoomPatch )
    {
        // ReSharper disable once CppExpressionWithoutSideEffects
        level::Level::findRealFloorSector( eyePositionGoal );
    }
}

void CameraController::freeLookClamp(core::Length& currentFrontBack,
                                     core::Length& currentLeftRight,
                                     const core::Length targetFrontBack,
                                     const core::Length targetLeftRight,
                                     const core::Length back,
                                     const core::Length right,
                                     const core::Length front,
                                     const core::Length left)
{
    if( (front > back) != (targetFrontBack < back) )
    {
        currentFrontBack = back;
        currentLeftRight = targetLeftRight + (currentLeftRight - targetLeftRight) * (back - targetFrontBack)
                                             / (currentFrontBack - targetFrontBack);
    }
    else if( (right < left && targetLeftRight > right && right > currentLeftRight)
             || (right > left && targetLeftRight < right && right < currentLeftRight) )
    {
        currentFrontBack = targetFrontBack + (currentFrontBack - targetFrontBack) * (right - targetLeftRight)
                                             / (currentLeftRight - targetLeftRight);
        currentLeftRight = right;
    }
}

void CameraController::clampToCorners(const core::Area targetHorizontalDistanceSq,
                                      core::Length& currentFrontBack,
                                      core::Length& currentLeftRight,
                                      const core::Length targetFrontBack,
                                      const core::Length targetLeftRight,
                                      const core::Length back,
                                      const core::Length right,
                                      const core::Length front,
                                      const core::Length left)
{
    const auto targetRightDistSq = util::square( targetLeftRight - right );
    const auto targetBackDistSq = util::square( targetFrontBack - back );

    // back right
    const auto backRightDistSq = targetBackDistSq + targetRightDistSq;
    if( backRightDistSq > targetHorizontalDistanceSq )
    {
        //BOOST_LOG_TRIVIAL(debug) << "Clamp back right: " << currentFrontBack << " => " << back;
        //BOOST_LOG_TRIVIAL(debug) << "Left = " << left << ", right = " << right << ", front = " << front << ", back = " << back;
        currentFrontBack = back;
        if( targetHorizontalDistanceSq >= targetBackDistSq )
        {
            auto tmp = sqrt( targetHorizontalDistanceSq - targetBackDistSq );
            if( right < left )
                tmp = -tmp;
            currentLeftRight = tmp + targetLeftRight;
        }
        return;
    }

    if( backRightDistSq > util::square( core::QuarterSectorSize ) )
    {
        currentFrontBack = back;
        currentLeftRight = right;
        return;
    }

    // back left
    const auto targetLeftDistSq = (targetLeftRight - left) * (targetLeftRight - left);
    const auto targetBackLeftDistSq = targetBackDistSq + targetLeftDistSq;
    if( targetBackLeftDistSq > targetHorizontalDistanceSq )
    {
        //BOOST_LOG_TRIVIAL(debug) << "Clamp back left: " << currentFrontBack << " => " << back;
        //BOOST_LOG_TRIVIAL(debug) << "Left = " << left << ", right = " << right << ", front = " << front << ", back = " << back;
        currentFrontBack = back;
        if( targetHorizontalDistanceSq >= targetBackDistSq )
        {
            auto tmp = sqrt( targetHorizontalDistanceSq - targetBackDistSq );
            if( right >= left )
                tmp = -tmp;
            currentLeftRight = tmp + targetLeftRight;
        }
        return;
    }

    if( targetBackLeftDistSq > util::square( core::QuarterSectorSize ) )
    {
        currentFrontBack = back;
        currentLeftRight = left;
        return;
    }

    // front right
    const auto targetFrontDistSq = util::square( targetFrontBack - front );
    const auto targetFrontRightDistSq = targetFrontDistSq + targetRightDistSq;

    if( targetFrontRightDistSq > targetHorizontalDistanceSq )
    {
        //BOOST_LOG_TRIVIAL(debug) << "Clamp front right";
        //BOOST_LOG_TRIVIAL(debug) << "Left = " << left << ", right = " << right << ", front = " << front << ", back = " << back;
        if( targetHorizontalDistanceSq >= targetRightDistSq )
        {
            currentLeftRight = right;
            auto tmp = sqrt( targetHorizontalDistanceSq - targetRightDistSq );
            if( back >= front )
                tmp = -tmp;
            currentFrontBack = tmp + targetFrontBack;
        }
        return;
    }

    //BOOST_LOG_TRIVIAL(debug) << "Clamp front right: " << currentFrontBack << " => " << front << ", " << currentLeftRight << " => " << right;
    //BOOST_LOG_TRIVIAL(debug) << "Left = " << left << ", right = " << right << ", front = " << front << ", back = " << back;
    currentFrontBack = front;
    currentLeftRight = right;
}

void CameraController::updateCinematic(const loader::CinematicFrame& frame, const bool ingame)
{
    if( ingame )
    {
        m_center.position += util::pitch( frame.center, m_cinematicRot.Y );
        m_cinematicPos += util::pitch( frame.eye, m_cinematicRot.Y );

        auto m = lookAt( m_cinematicPos.toRenderSystem(), m_center.position.toRenderSystem(), {0, 1, 0} );
        m = rotate( m, frame.rotZ.toRad(), -glm::vec3{m[2]} );
        m_camera->setViewMatrix( m );
        m_camera->setFieldOfView( frame.fov.toRad() );
        level::Level::findRealFloorSector( m_eye.position, &m_eye.room );
    }
    else
    {
        core::TRVec center = m_eye.position + util::pitch( frame.center, m_eyeRotation.Y );
        core::TRVec eye = m_eye.position + util::pitch( frame.eye, m_eyeRotation.Y );

        auto m = lookAt( eye.toRenderSystem(), center.toRenderSystem(), {0, 1, 0} );
        m = rotate( m, frame.rotZ.toRad(), -glm::vec3{m[2]} );
        m_camera->setViewMatrix( m );
        m_camera->setFieldOfView( frame.fov.toRad() );
    }
}

CameraController::CameraController(gsl::not_null<level::Level*> level,
                                   gsl::not_null<std::shared_ptr<gameplay::Camera>> camera,
                                   bool /*noLaraTag*/)
        : Listener{&level->m_soundEngine}
        , m_camera{std::move( camera )}
        , m_level{level}
        , m_eye{&level->m_rooms[0]}
        , m_center{&level->m_rooms[0]}
{
}

YAML::Node CameraController::save() const
{
    YAML::Node result;
    result["eye"]["position"] = m_eye.position.save();
    result["eye"]["room"] = std::distance( const_cast<const loader::Room*>(&m_level->m_rooms[0]), m_eye.room.get() );
    result["center"]["position"] = m_center.position.save();
    result["center"]["room"] = std::distance( const_cast<const loader::Room*>(&m_level->m_rooms[0]),
                                              m_center.room.get() );
    result["mode"] = toString( m_mode );
    result["modifier"] = toString( m_modifier );
    result["tracking"] = m_fixed;
    if( m_item != nullptr )
    {
        result["item"] = std::find_if( m_level->m_itemNodes.begin(), m_level->m_itemNodes.end(),
                                       [&](const std::pair<uint16_t, std::shared_ptr<items::ItemNode>>& entry) {
                                           return entry.second == m_item;
                                       } )->first;
    }
    if( m_previousItem != nullptr )
    {
        result["lastItem"] = std::find_if( m_level->m_itemNodes.begin(), m_level->m_itemNodes.end(),
                                           [&](const std::pair<uint16_t, std::shared_ptr<items::ItemNode>>& entry) {
                                               return entry.second == m_previousItem;
                                           } )->first;
    }
    if( m_enemy != nullptr )
    {
        result["enemy"] = std::find_if( m_level->m_itemNodes.begin(), m_level->m_itemNodes.end(),
                                        [&](const std::pair<uint16_t, std::shared_ptr<items::ItemNode>>& entry) {
                                            return entry.second == m_enemy;
                                        } )->first;
    }
    result["yOffset"] = m_eyeYOffset;
    result["bounce"] = m_bounce;
    result["eyeCenterDistance"] = m_eyeCenterDistance;
    result["eyeCenterHorizontalDistanceSq"] = m_eyeCenterHorizontalDistanceSq;
    result["eyeRotation"] = m_eyeRotation.save();
    result["currentRotation"] = m_rotationAroundCenter.save();
    result["trackingSmoothness"] = m_smoothness;
    result["fixedCameraId"] = m_fixedCameraId;
    result["currentFixedCameraId"] = m_currentFixedCameraId;
    result["camOverrideTimeout"] = m_camOverrideTimeout;
    result["cinematicFrame"] = m_cinematicFrame;
    result["cinematicPos"] = m_cinematicPos.save();
    result["cinematicRot"] = m_cinematicRot.save();
    return result;
}

void CameraController::load(const YAML::Node& n)
{
    m_eye.position.load( n["eye"]["position"] );
    m_eye.room = &m_level->m_rooms[n["eye"]["room"].as<size_t>()];
    m_center.position.load( n["center"]["position"] );
    m_center.room = &m_level->m_rooms[n["center"]["room"].as<size_t>()];
    m_mode = parseCameraMode( n["mode"].as<std::string>() );
    m_modifier = parseCameraModifier( n["modifier"].as<std::string>() );
    m_fixed = n["tracking"].as<bool>();
    if( !n["item"].IsDefined() )
    {
        m_item = nullptr;
    }
    else
    {
        const auto it = m_level->m_itemNodes.find( n["item"].as<uint16_t>() );
        if( it == m_level->m_itemNodes.end() )
            BOOST_THROW_EXCEPTION( std::domain_error( "Invalid item reference" ) );
        m_item = it->second.get();
    }
    if( !n["lastItem"].IsDefined() )
    {
        m_previousItem = nullptr;
    }
    else
    {
        const auto it = m_level->m_itemNodes.find( n["lastItem"].as<uint16_t>() );
        if( it == m_level->m_itemNodes.end() )
            BOOST_THROW_EXCEPTION( std::domain_error( "Invalid item reference" ) );
        m_previousItem = it->second.get();
    }
    if( !n["enemy"].IsDefined() )
    {
        m_enemy = nullptr;
    }
    else
    {
        const auto it = m_level->m_itemNodes.find( n["enemy"].as<uint16_t>() );
        if( it == m_level->m_itemNodes.end() )
            BOOST_THROW_EXCEPTION( std::domain_error( "Invalid item reference" ) );
        m_enemy = it->second.get();
    }
    m_eyeYOffset = n["yOffset"].as<core::Length>();
    m_bounce = n["bounce"].as<core::Length>();
    m_eyeCenterDistance = n["eyeCenterDistance"].as<core::Length>();
    m_eyeCenterHorizontalDistanceSq = n["eyeCenterHorizontalDistanceSq"].as<core::Area>();
    m_eyeRotation.load( n["eyeRotation"] );
    m_rotationAroundCenter.load( n["currentRotation"] );
    m_smoothness = n["trackingSmoothness"].as<int>();
    m_fixedCameraId = n["fixedCameraId"].as<int>();
    m_currentFixedCameraId = n["currentFixedCameraId"].as<int>();
    m_camOverrideTimeout = n["camOverrideTimeout"].as<core::Frame>();
    m_cinematicFrame = n["cinematicFrame"].as<size_t>();
    m_cinematicPos.load( n["cinematicPos"] );
    m_cinematicRot.load( n["cinematicRot"] );
}
}
