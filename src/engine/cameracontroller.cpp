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
}

CameraController::CameraController(const gsl::not_null<level::Level*>& level,
                                   gsl::not_null<std::shared_ptr<gameplay::Camera>> camera)
        : m_camera{std::move( camera )}
        , m_level{level}
        , m_eye{level->m_lara->m_state.position.room}
        , m_center{level->m_lara->m_state.position.room, level->m_lara->m_state.position.position}
        , m_cameraYOffset{level->m_lara->m_state.position.position.Y - loader::SectorSize}
{
    Expects( level->m_lara != nullptr );

    m_center.position.Y -= m_cameraYOffset;
    m_eye = m_center;
    m_eye.position.Z -= 100;

    update();
}

void CameraController::setCurrentRotation(const core::Angle x, const core::Angle y)
{
    setCurrentRotationX( x );
    setCurrentRotationY( y );
}

void CameraController::setCurrentRotationX(const core::Angle x)
{
    m_currentRotation.X = x;
}

void CameraController::setCurrentRotationY(const core::Angle y)
{
    m_currentRotation.Y = y;
}

void CameraController::setCamOverride(const floordata::CameraParameters& camParams,
                                      const uint16_t camId,
                                      const floordata::SequenceCondition condition,
                                      const bool fromHeavy,
                                      const uint16_t activationRequest,
                                      const bool switchIsOn)
{
    Expects( camId < m_level->m_cameras.size() );
    if( m_level->m_cameras[camId].isActive() )
        return;

    m_fixedCameraId = camId;
    if( m_mode == CameraMode::FreeLook || m_mode == CameraMode::Combat
        || condition == floordata::SequenceCondition::LaraInCombatMode )
        return;

    if( condition == floordata::SequenceCondition::ItemActivated
        && floordata::ActivationState::extractTimeout( activationRequest ) != 0 && switchIsOn )
        return;

    if( condition != floordata::SequenceCondition::ItemActivated && m_fixedCameraId == m_currentFixedCameraId )
        return;

    if( camParams.timeout != 1 )
        m_camOverrideTimeout = camParams.timeout * core::FrameRate;

    if( camParams.oneshot )
        m_level->m_cameras[camId].setActive( true );

    m_trackingSmoothness = 1 + (camParams.smoothness * 4);
    if( fromHeavy )
        m_mode = CameraMode::Heavy;
    else
        m_mode = CameraMode::Fixed;
}

void CameraController::findItem(const uint16_t* cmdSequence)
{
    if( m_mode == CameraMode::Heavy )
        return;

    CameraMode type = CameraMode::FreeLook;
    while( true )
    {
        const floordata::Command command{*cmdSequence++};

        if( command.opcode == floordata::CommandOpcode::LookAt && m_mode != CameraMode::FreeLook
            && m_mode != CameraMode::Combat )
        {
            m_item = m_level->getItem( command.parameter );
        }
        else if( command.opcode == floordata::CommandOpcode::SwitchCamera )
        {
            ++cmdSequence; // skip camera parameters

            if( command.parameter != m_currentFixedCameraId )
            {
                type = CameraMode::Chase; // new override
            }
            else
            {
                m_fixedCameraId = m_currentFixedCameraId;
                if( m_camOverrideTimeout >= 0 && m_mode != CameraMode::FreeLook && m_mode != CameraMode::Combat )
                {
                    type = CameraMode::Fixed;
                    m_mode = CameraMode::Fixed;
                }
                else
                {
                    type = CameraMode::Chase;
                    m_camOverrideTimeout = -1;
                }
            }
        }

        if( command.isLast )
            break;
    }

    if( type == CameraMode::Chase
        || (type == CameraMode::FreeLook && m_item != nullptr && m_item->m_state.already_looked_at
            && m_item != m_lastItem) )
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

        m_level->m_rooms[portal.adjoining_room].node->setVisible( true );

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
        const uint16_t destRoom = currentPath.getLastDestinationRoom();
        for( const loader::Portal& srcPortal : m_level->m_rooms[destRoom].portals )
        {
            render::PortalTracer newPath = currentPath;
            if( !newPath.checkVisibility( &srcPortal, *m_camera.get() ) )
                continue;

            m_level->m_rooms[srcPortal.adjoining_room].node->setVisible( true );
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

    const int sign = d.X < 0 ? -1 : 1;

    core::TRVec testPos;
    testPos.X = (start.position.X / loader::SectorSize) * loader::SectorSize;
    if( sign > 0 )
        testPos.X += loader::SectorSize - 1;

    testPos.Y = start.position.Y + (testPos.X - start.position.X) * d.Y / d.X;
    testPos.Z = start.position.Z + (testPos.X - start.position.X) * d.Z / d.X;

    core::TRVec step;
    step.X = sign * loader::SectorSize;
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
        auto sector = gsl::make_not_null( level::Level::findRealFloorSector( heightPos, make_not_null( &room ) ) );
        if( testPos.Y > HeightInfo::fromFloor( sector, heightPos, level.m_itemNodes ).y
            || testPos.Y < HeightInfo::fromCeiling( sector, heightPos, level.m_itemNodes ).y )
        {
            end.position = testPos;
            end.room = room;
            return ClampType::Ceiling;
        }

        heightPos.X = testPos.X + sign;
        sector = gsl::make_not_null( level.findRealFloorSector( heightPos, room ) );
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

    const int sign = d.Z < 0 ? -1 : 1;

    core::TRVec testPos;
    testPos.Z = (start.position.Z / loader::SectorSize) * loader::SectorSize;

    if( sign > 0 )
        testPos.Z += loader::SectorSize - 1;

    testPos.X = start.position.X + (testPos.Z - start.position.Z) * d.X / d.Z;
    testPos.Y = start.position.Y + (testPos.Z - start.position.Z) * d.Y / d.Z;

    core::TRVec step;
    step.Z = sign * loader::SectorSize;
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
        auto sector = gsl::make_not_null( level::Level::findRealFloorSector( heightPos, make_not_null( &room ) ) );
        if( testPos.Y > HeightInfo::fromFloor( sector, heightPos, level.m_itemNodes ).y
            || testPos.Y < HeightInfo::fromCeiling( sector, heightPos, level.m_itemNodes ).y )
        {
            end.position = testPos;
            end.room = room;
            return ClampType::Ceiling;
        }

        heightPos.Z = testPos.Z + sign;
        sector = gsl::make_not_null( level.findRealFloorSector( heightPos, room ) );
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
    if( std::abs( end.position.Z - start.position.Z ) <= std::abs( end.position.X - start.position.X ) )
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

    const auto sector = gsl::make_not_null( level.findRealFloorSector( end ) );
    return clampY( start.position, end.position, sector, level ) && firstUnclamped && secondClamp == ClampType::None;
}

void CameraController::update()
{
    m_currentRotation.X = util::clamp( m_currentRotation.X, -85_deg, +85_deg );

    if( m_eye.room->isWaterRoom() )
    {
        if( isPlaying( m_level->m_ambientStream ) )
            m_level->m_ambientStream.lock()
                   ->getSource().lock()->setDirectFilter( m_level->m_audioDev.getUnderwaterFilter() );

        if( isPlaying( m_level->m_interceptStream ) )
            m_level->m_interceptStream.lock()
                   ->getSource().lock()->setDirectFilter( m_level->m_audioDev.getUnderwaterFilter() );

        if( m_underwaterAmbience.expired() )
        {
            m_underwaterAmbience = m_level->playSound( TR1SoundId::UnderwaterAmbience, boost::none );
            m_underwaterAmbience.lock()->setLooping( true );
        }
    }
    else if( !m_underwaterAmbience.expired() )
    {
        if( !m_level->m_ambientStream.expired() )
            m_level->m_ambientStream.lock()->getSource().lock()->setDirectFilter( nullptr );

        if( isPlaying( m_level->m_interceptStream ) )
            m_level->m_interceptStream.lock()->getSource().lock()->setDirectFilter( nullptr );

        m_level->stopSound( TR1SoundId::UnderwaterAmbience );
        m_underwaterAmbience.reset();
    }

    if( !isPlaying( m_level->m_interceptStream ) )
    {
        if( const auto str = m_level->m_ambientStream.lock() )
            str->play();
    }

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

    if( m_oldMode != CameraMode::FreeLook )
        HeightInfo::skipSteepSlants = true;

    const bool tracking = m_item != nullptr && (m_mode == CameraMode::Fixed || m_mode == CameraMode::Heavy);

    std::shared_ptr<items::ItemNode> trackedItem = tracking ? m_item : m_level->m_lara;
    auto trackedBBox = trackedItem->getBoundingBox();
    int trackedY = trackedItem->m_state.position.position.Y;
    if( tracking )
        trackedY += (trackedBBox.minY + trackedBBox.maxY) / 2;
    else
        trackedY += (trackedBBox.minY - trackedBBox.maxY) * 3 / 4 + trackedBBox.maxY;

    if( m_item != nullptr && !tracking )
    {
        BOOST_ASSERT( m_item != trackedItem );
        BOOST_ASSERT( trackedItem );
        const auto distToTarget = m_item->m_state.position.position
                                        .distanceTo( trackedItem->m_state.position.position );
        auto trackAngleY =
                core::Angle::fromAtan( m_item->m_state.position.position.X - trackedItem->m_state.position.position.X,
                                       m_item->m_state.position.position.Z - trackedItem->m_state.position.position.Z )
                - trackedItem->m_state.rotation.Y;
        trackAngleY *= 0.5f;
        trackedBBox = m_item->getBoundingBox();
        auto trackAngleX = core::Angle::fromAtan( distToTarget, trackedY - (trackedBBox.minY + trackedBBox.maxY) / 2.0f
                                                                + m_item->m_state.position.position.Y );
        trackAngleX *= 0.5f;

        if( trackAngleY < 50_deg && trackAngleY > -50_deg && trackAngleX < 85_deg && trackAngleX > -85_deg )
        {
            trackAngleY -= m_level->m_lara->m_headRotation.Y;
            if( trackAngleY > 4_deg )
                m_level->m_lara->m_headRotation.Y += 4_deg;
            else if( trackAngleY < -4_deg )
                m_level->m_lara->m_headRotation.Y -= 4_deg;
            else
                m_level->m_lara->m_headRotation.Y += trackAngleY;
            m_level->m_lara->m_torsoRotation.Y = m_level->m_lara->m_headRotation.Y;

            trackAngleX -= m_level->m_lara->m_headRotation.X;
            if( trackAngleX > 4_deg )
                m_level->m_lara->m_headRotation.X += 4_deg;
            else if( trackAngleX < -4_deg )
                m_level->m_lara->m_headRotation.X -= 4_deg;
            else
                m_level->m_lara->m_headRotation.X += trackAngleX;
            m_level->m_lara->m_torsoRotation.X = m_level->m_lara->m_headRotation.X;

            m_mode = CameraMode::FreeLook;
            m_item->m_state.already_looked_at = true;
        }
    }

    m_center.room = trackedItem->m_state.position.room;

    if( m_mode != CameraMode::FreeLook && m_mode != CameraMode::Combat )
    {
        m_center.position.X = trackedItem->m_state.position.position.X;
        m_center.position.Z = trackedItem->m_state.position.position.Z;

        if( m_oldMode == CameraMode::Fixed )
        {
            const auto midZ = (trackedBBox.minZ + trackedBBox.maxZ) / 2;
            m_center.position.Z += midZ * trackedItem->m_state.rotation.Y.cos();
            m_center.position.X += midZ * trackedItem->m_state.rotation.Y.sin();
        }

        if( m_tracking == tracking )
        {
            m_tracking = false;
            m_center.position.Y += (trackedY - m_center.position.Y) / 4;
        }
        else
        {
            m_tracking = true;
            m_center.position.Y = trackedY;
            m_trackingSmoothness = 1;
        }

        const auto sector = gsl::make_not_null( m_level->findRealFloorSector( m_center ) );
        if( HeightInfo::fromFloor( sector, m_center.position, getLevel()->m_itemNodes ).y < m_center.position.Y )
            HeightInfo::skipSteepSlants = false;

        if( m_mode == CameraMode::Chase || m_oldMode == CameraMode::Combat )
            doUsualMovement( gsl::make_not_null( trackedItem ) );
        else
            handleCamOverride();
    }
    else
    {
        if( m_tracking )
        {
            m_center.position.Y = trackedY - loader::QuarterSectorSize;
            m_trackingSmoothness = 1;
        }
        else
        {
            m_center.position.Y += (trackedY - loader::QuarterSectorSize - m_center.position.Y) / 4;
            if( m_mode == CameraMode::FreeLook )
                m_trackingSmoothness = 4;
            else
                m_trackingSmoothness = 8;
        }
        m_tracking = false;
        if( m_mode == CameraMode::FreeLook )
            handleFreeLook( *trackedItem );
        else
            handleEnemy( *trackedItem );
    }

    m_tracking = tracking;
    m_currentFixedCameraId = m_fixedCameraId;
    if( m_mode != CameraMode::Heavy || m_camOverrideTimeout < 0 )
    {
        m_mode = CameraMode::Chase;
        m_lastItem = m_item;
        m_currentRotation.X = m_currentRotation.Y = 0_deg;
        m_eyeCenterDistance = 1536;
        m_fixedCameraId = -1;
        m_item = nullptr;
        m_oldMode = CameraMode::Chase;
    }
    HeightInfo::skipSteepSlants = false;

    tracePortals();
}

void CameraController::handleCamOverride()
{
    Expects( m_fixedCameraId >= 0 && gsl::narrow_cast<size_t>( m_fixedCameraId ) < m_level->m_cameras.size() );
    Expects( m_level->m_cameras[m_fixedCameraId].room < m_level->m_rooms.size() );

    core::RoomBoundPosition pos( gsl::make_not_null( &m_level->m_rooms[m_level->m_cameras[m_fixedCameraId].room] ) );
    pos.position = m_level->m_cameras[m_fixedCameraId].position;

    if( !clampPosition( m_center, pos, *m_level ) )
    {
        // ReSharper disable once CppExpressionWithoutSideEffects
        moveIntoGeometry( pos, loader::QuarterSectorSize );
    }

    m_tracking = true;
    updatePosition( pos, m_trackingSmoothness );

    if( m_camOverrideTimeout > 0 )
        --m_camOverrideTimeout;
    else
        m_camOverrideTimeout = -1;
}

int CameraController::moveIntoGeometry(core::RoomBoundPosition& pos, const int margin) const
{
    const auto sector = gsl::make_not_null( m_level->findRealFloorSector( pos ) );
    BOOST_ASSERT( sector->box != nullptr );

    const auto room = pos.room;

    if( sector->box->zmin + margin > pos.position.Z
        && isVerticallyOutsideRoom( pos.position - core::TRVec( 0, 0, margin ), room ) )
        pos.position.Z = sector->box->zmin + margin;
    else if( sector->box->zmax - margin > pos.position.Z
             && isVerticallyOutsideRoom( pos.position + core::TRVec( 0, 0, margin ), room ) )
        pos.position.Z = sector->box->zmax - margin;

    if( sector->box->xmin + margin > pos.position.X
        && isVerticallyOutsideRoom( pos.position - core::TRVec( margin, 0, 0 ), room ) )
        pos.position.X = sector->box->xmin + margin;
    else if( sector->box->xmax - margin > pos.position.X
             && isVerticallyOutsideRoom( pos.position + core::TRVec( margin, 0, 0 ), room ) )
        pos.position.X = sector->box->xmax - margin;

    auto bottom = HeightInfo::fromFloor( sector, pos.position, getLevel()->m_itemNodes ).y - margin;
    auto top = HeightInfo::fromCeiling( sector, pos.position, getLevel()->m_itemNodes ).y + margin;
    if( bottom < top )
        top = bottom = (bottom + top) / 2;

    if( pos.position.Y > bottom )
        return bottom - pos.position.Y;
    if( top > pos.position.Y )
        return top - pos.position.Y;
    return 0;
}

bool CameraController::isVerticallyOutsideRoom(const core::TRVec& pos,
                                               const gsl::not_null<const loader::Room*>& room) const
{
    const auto sector = gsl::make_not_null( m_level->findRealFloorSector( pos, room ) );
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
    auto sector = gsl::make_not_null( m_level->findRealFloorSector( m_eye ) );
    auto floor = HeightInfo::fromFloor( sector, m_eye.position, getLevel()->m_itemNodes )
                         .y - loader::QuarterSectorSize;
    if( floor <= m_eye.position.Y && floor <= eyePositionGoal.position.Y )
    {
        clampPosition( m_center, m_eye, *m_level );
        sector = gsl::make_not_null( m_level->findRealFloorSector( m_eye ) );
        floor = HeightInfo::fromFloor( sector, m_eye.position, getLevel()->m_itemNodes )
                        .y - loader::QuarterSectorSize;
    }

    auto ceiling =
            HeightInfo::fromCeiling( sector, m_eye.position, getLevel()->m_itemNodes )
                    .y + loader::QuarterSectorSize;
    if( floor < ceiling )
    {
        floor = ceiling = (floor + ceiling) / 2;
    }

    if( m_bounce < 0 )
    {
        auto tmp = util::rand15s( m_bounce );
        m_eye.position.X += tmp;
        m_center.position.X += tmp;
        tmp = util::rand15s( m_bounce );
        m_eye.position.Y += tmp;
        m_center.position.Y += tmp;
        tmp = util::rand15s( m_bounce );
        m_eye.position.Z += tmp;
        m_center.position.Z += tmp;
        m_bounce += 5;
    }
    else if( m_bounce > 0 )
    {
        m_eye.position.Y += m_bounce;
        m_center.position.Y += m_bounce;
        m_bounce = 0;
    }

    if( m_eye.position.Y > floor )
        m_cameraYOffset = floor - m_eye.position.Y;
    else if( m_eye.position.Y < ceiling )
        m_cameraYOffset = ceiling - m_eye.position.Y;
    else
        m_cameraYOffset = 0;

    auto camPos = m_eye.position;
    camPos.Y += m_cameraYOffset;

    // update current room
    level::Level::findRealFloorSector( camPos, make_not_null( &m_eye.room ) );

    const auto m = lookAt( camPos.toRenderSystem(), m_center.position.toRenderSystem(), {0, 1, 0} );
    m_camera->setViewMatrix( m );
}

void CameraController::doUsualMovement(const gsl::not_null<std::shared_ptr<const items::ItemNode>>& item)
{
    m_currentRotation.X += item->m_state.rotation.X;
    if( m_currentRotation.X > 85_deg )
        m_currentRotation.X = 85_deg;
    else if( m_currentRotation.X < -85_deg )
        m_currentRotation.X = -85_deg;

    const auto dist = m_currentRotation.X.cos() * m_eyeCenterDistance;
    m_eyeCenterHorizontalDistanceSq = gsl::narrow_cast<int>( util::square( dist ) );

    core::RoomBoundPosition eye( m_eye.room );
    eye.position.Y = m_eyeCenterDistance * m_currentRotation.X.sin() + m_center.position.Y;

    core::Angle y = m_currentRotation.Y + item->m_state.rotation.Y;
    eye.position.X = m_center.position.X - dist * y.sin();
    eye.position.Z = m_center.position.Z - dist * y.cos();
    clampBox( eye,
              [this](int& a, int& b, const int c, const int d, const int e, const int f, const int g, const int h) {
                  clampToCorners( m_eyeCenterHorizontalDistanceSq, a, b, c, d, e, f, g, h );
              } );

    updatePosition( eye, m_tracking ? m_trackingSmoothness : 12 );
}

void CameraController::handleFreeLook(const items::ItemNode& item)
{
    const auto originalCenter = m_center.position;
    m_center.position.X = item.m_state.position.position.X;
    m_center.position.Z = item.m_state.position.position.Z;
    m_currentRotation.X = m_level->m_lara->m_torsoRotation.X + m_level->m_lara->m_headRotation.X
                          + item.m_state.rotation.X;
    m_currentRotation.Y = m_level->m_lara->m_torsoRotation.Y + m_level->m_lara->m_headRotation.Y
                          + item.m_state.rotation.Y;
    m_eyeCenterDistance = 1536;
    m_cameraYOffset = gsl::narrow_cast<int>( -2 * loader::QuarterSectorSize * m_currentRotation.Y.sin() );
    m_center.position.X += m_cameraYOffset * item.m_state.rotation.Y.sin();
    m_center.position.Z += m_cameraYOffset * item.m_state.rotation.Y.cos();

    if( isVerticallyOutsideRoom( m_center.position, m_eye.room ) )
    {
        m_center.position.X = item.m_state.position.position.X;
        m_center.position.Z = item.m_state.position.position.Z;
    }

    m_center.position.Y += moveIntoGeometry( m_center, loader::QuarterSectorSize + 50 );

    auto center = m_center;
    center.position.X -= m_eyeCenterDistance * m_currentRotation.Y.sin() * m_currentRotation.X.cos();
    center.position.Z -= m_eyeCenterDistance * m_currentRotation.Y.cos() * m_currentRotation.X.cos();
    center.position.Y += m_eyeCenterDistance * m_currentRotation.X.sin();
    center.room = m_eye.room;

    clampBox( center, &freeLookClamp );

    m_center.position.X = originalCenter.X
                          + (m_center.position.X - originalCenter.X) / m_trackingSmoothness;
    m_center.position.Z = originalCenter.Z
                          + (m_center.position.Z - originalCenter.Z) / m_trackingSmoothness;

    updatePosition( center, m_trackingSmoothness );
}

void CameraController::handleEnemy(const items::ItemNode& item)
{
    m_center.position.X = item.m_state.position.position.X;
    m_center.position.Z = item.m_state.position.position.Z;

    if( m_enemy != nullptr )
    {
        m_currentRotation.X = m_eyeRotation.X + item.m_state.rotation.X;
        m_currentRotation.Y = m_eyeRotation.Y + item.m_state.rotation.Y;
    }
    else
    {
        m_currentRotation.X = m_level->m_lara->m_torsoRotation.X + m_level->m_lara->m_headRotation.X
                              + item.m_state.rotation.X;
        m_currentRotation.Y = m_level->m_lara->m_torsoRotation.Y + m_level->m_lara->m_headRotation.Y
                              + item.m_state.rotation.Y;
    }

    m_eyeCenterDistance = 2560;
    auto eye = m_center;
    const auto d = m_eyeCenterDistance * m_currentRotation.X.cos();
    eye.position.X -= d * m_currentRotation.Y.sin();
    eye.position.Z -= d * m_currentRotation.Y.cos();
    eye.position.Y += m_eyeCenterDistance * m_currentRotation.X.sin();
    eye.room = m_eye.room;

    clampBox( eye,
              [this](int& a, int& b, const int c, const int d, const int e, const int f, const int g, const int h) {
                  clampToCorners( m_eyeCenterHorizontalDistanceSq, a, b, c, d, e, f, g, h );
              } );
    updatePosition( eye, m_trackingSmoothness );
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
    testPos.Z = (testPos.Z / loader::SectorSize) * loader::SectorSize - 1;
    BOOST_ASSERT( testPos.Z % loader::SectorSize == loader::SectorSize - 1
                  && std::abs( testPos.Z - eyePositionGoal.position.Z ) <= loader::SectorSize );

    auto clampZMin = clampBox->zmin;
    const bool negZVerticallyOutside = isVerticallyOutsideRoom( testPos, eyePositionGoal.room );
    if( !negZVerticallyOutside && m_level->findRealFloorSector( testPos, eyePositionGoal.room )->box != nullptr )
    {
        const auto testBox = m_level->findRealFloorSector( testPos, eyePositionGoal.room )->box;
        if( testBox->zmin < clampZMin )
            clampZMin = testBox->zmin;
    }
    clampZMin += loader::QuarterSectorSize;

    testPos = eyePositionGoal.position;
    testPos.Z = (testPos.Z / loader::SectorSize + 1) * loader::SectorSize;
    BOOST_ASSERT( testPos.Z % loader::SectorSize == 0
                  && std::abs( testPos.Z - eyePositionGoal.position.Z ) <= loader::SectorSize );

    auto clampZMax = clampBox->zmax;
    const bool posZVerticallyOutside = isVerticallyOutsideRoom( testPos, eyePositionGoal.room );
    if( !posZVerticallyOutside && m_level->findRealFloorSector( testPos, eyePositionGoal.room )->box != nullptr )
    {
        const auto testBox = m_level->findRealFloorSector( testPos, eyePositionGoal.room )->box;
        if( testBox->zmax > clampZMax )
            clampZMax = testBox->zmax;
    }
    clampZMax -= loader::QuarterSectorSize;

    testPos = eyePositionGoal.position;
    testPos.X = (testPos.X / loader::SectorSize) * loader::SectorSize - 1;
    BOOST_ASSERT( testPos.X % loader::SectorSize == loader::SectorSize - 1
                  && std::abs( testPos.X - eyePositionGoal.position.X ) <= loader::SectorSize );

    auto clampXMin = clampBox->xmin;
    const bool negXVerticallyOutside = isVerticallyOutsideRoom( testPos, eyePositionGoal.room );
    if( !negXVerticallyOutside && m_level->findRealFloorSector( testPos, eyePositionGoal.room )->box != nullptr )
    {
        const auto testBox = m_level->findRealFloorSector( testPos, eyePositionGoal.room )->box;
        if( testBox->xmin < clampXMin )
            clampXMin = testBox->xmin;
    }
    clampXMin += loader::QuarterSectorSize;

    testPos = eyePositionGoal.position;
    testPos.X = (testPos.X / loader::SectorSize + 1) * loader::SectorSize;
    BOOST_ASSERT( testPos.X % loader::SectorSize == 0
                  && std::abs( testPos.X - eyePositionGoal.position.X ) <= loader::SectorSize );

    auto clampXMax = clampBox->xmax;
    const bool posXVerticallyOutside = isVerticallyOutsideRoom( testPos, eyePositionGoal.room );
    if( !posXVerticallyOutside && m_level->findRealFloorSector( testPos, eyePositionGoal.room )->box != nullptr )
    {
        const auto testBox = m_level->findRealFloorSector( testPos, eyePositionGoal.room )->box;
        if( testBox->xmax > clampXMax )
            clampXMax = testBox->xmax;
    }
    clampXMax -= loader::QuarterSectorSize;

    bool skipRoomPatch = true;
    if( negZVerticallyOutside && eyePositionGoal.position.Z < clampZMin )
    {
        skipRoomPatch = false;
        int left, right;
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
        int left, right;
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
        m_level->findRealFloorSector( eyePositionGoal );
        return;
    }

    if( negXVerticallyOutside && eyePositionGoal.position.X < clampXMin )
    {
        skipRoomPatch = false;
        int left, right;
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
        int left, right;
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
        m_level->findRealFloorSector( eyePositionGoal );
    }
}

void CameraController::freeLookClamp(int& currentFrontBack,
                                     int& currentLeftRight,
                                     const int targetFrontBack,
                                     const int targetLeftRight,
                                     const int back,
                                     const int right,
                                     const int front,
                                     const int left)
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

void CameraController::clampToCorners(const int targetHorizontalDistanceSq,
                                      int& currentFrontBack,
                                      int& currentLeftRight,
                                      const int targetFrontBack,
                                      const int targetLeftRight,
                                      const int back,
                                      const int right,
                                      const int front,
                                      const int left)
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
            auto tmp = std::sqrt( targetHorizontalDistanceSq - targetBackDistSq );
            if( right < left )
                tmp = -tmp;
            currentLeftRight = tmp + targetLeftRight;
        }
        return;
    }

    if( backRightDistSq > util::square( loader::QuarterSectorSize ) )
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
            auto tmp = std::sqrt( targetHorizontalDistanceSq - targetBackDistSq );
            if( right >= left )
                tmp = -tmp;
            currentLeftRight = tmp + targetLeftRight;
        }
        return;
    }

    if( targetBackLeftDistSq > util::square( loader::QuarterSectorSize ) )
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
            auto tmp = std::sqrt( targetHorizontalDistanceSq - targetRightDistSq );
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
        const auto c = m_cinematicRot.Y.cos();
        const auto s = m_cinematicRot.Y.sin();

        core::TRVec center = m_cinematicPos;
        center.X += (s * frame.center.Z + c * frame.center.X);
        center.Y += frame.center.Y;
        center.Z += (c * frame.center.Z - s * frame.center.X);
        m_center.position = center;

        core::TRVec eye = m_cinematicPos;
        eye.X += (s * frame.eye.Z + c * frame.eye.X);
        eye.Y += frame.eye.Y;
        eye.Z += (c * frame.eye.Z - s * frame.eye.X);
        m_eye.position = eye;

        auto m = lookAt( eye.toRenderSystem(), center.toRenderSystem(), {0, 1, 0} );
        m = rotate( m, frame.rotZ.toRad(), -glm::vec3{m[2]} );
        m_camera->setViewMatrix( m );
        m_camera->setFieldOfView( frame.fov.toRad() );
        level::Level::findRealFloorSector( m_eye.position, make_not_null( &m_eye.room ) );
    }
    else
    {
        const auto c = m_eyeRotation.Y.cos();
        const auto s = m_eyeRotation.Y.sin();

        core::TRVec center = m_eye.position;
        center.X += (s * frame.center.Z + c * frame.center.X);
        center.Y += frame.center.Y;
        center.Z += (c * frame.center.Z - s * frame.center.X);

        core::TRVec eye = m_eye.position;
        eye.X += (s * frame.eye.Z + c * frame.eye.X);
        eye.Y += frame.eye.Y;
        eye.Z += (c * frame.eye.Z - s * frame.eye.X);

        auto m = lookAt( eye.toRenderSystem(), center.toRenderSystem(), {0, 1, 0} );
        m = rotate( m, frame.rotZ.toRad(), -glm::vec3{m[2]} );
        m_camera->setViewMatrix( m );
        m_camera->setFieldOfView( frame.fov.toRad() );
    }
}

CameraController::CameraController(gsl::not_null<level::Level*> level,
                                   gsl::not_null<std::shared_ptr<gameplay::Camera>> camera,
                                   bool /*noLaraTag*/)
        : m_camera{std::move( camera )}
        , m_level{level}
        , m_eye{gsl::make_not_null( &level->m_rooms[0] )}
        , m_center{gsl::make_not_null( &level->m_rooms[0] )}
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
    result["oldMode"] = toString( m_oldMode );
    result["tracking"] = m_tracking;
    if( m_item != nullptr )
    {
        result["item"] = std::find_if( m_level->m_itemNodes.begin(), m_level->m_itemNodes.end(),
                                       [&](const std::pair<uint16_t, std::shared_ptr<items::ItemNode>>& entry) {
                                           return entry.second == m_item;
                                       } )->first;
    }
    if( m_lastItem != nullptr )
    {
        result["lastItem"] = std::find_if( m_level->m_itemNodes.begin(), m_level->m_itemNodes.end(),
                                           [&](const std::pair<uint16_t, std::shared_ptr<items::ItemNode>>& entry) {
                                               return entry.second == m_lastItem;
                                           } )->first;
    }
    if( m_enemy != nullptr )
    {
        result["enemy"] = std::find_if( m_level->m_itemNodes.begin(), m_level->m_itemNodes.end(),
                                        [&](const std::pair<uint16_t, std::shared_ptr<items::ItemNode>>& entry) {
                                            return entry.second == m_enemy;
                                        } )->first;
    }
    result["yOffset"] = m_cameraYOffset;
    result["bounce"] = m_bounce;
    result["eyeCenterDistance"] = m_eyeCenterDistance;
    result["eyeCenterHorizontalDistanceSq"] = m_eyeCenterHorizontalDistanceSq;
    result["eyeRotation"] = m_eyeRotation.save();
    result["currentRotation"] = m_currentRotation.save();
    result["trackingSmoothness"] = m_trackingSmoothness;
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
    m_eye.room = gsl::make_not_null( &m_level->m_rooms[n["eye"]["room"].as<size_t>()] );
    m_center.position.load( n["center"]["position"] );
    m_center.room = gsl::make_not_null( &m_level->m_rooms[n["center"]["room"].as<size_t>()] );
    m_mode = parseCameraMode( n["mode"].as<std::string>() );
    m_oldMode = parseCameraMode( n["oldMode"].as<std::string>() );
    m_tracking = n["tracking"].as<bool>();
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
        m_lastItem = nullptr;
    }
    else
    {
        const auto it = m_level->m_itemNodes.find( n["lastItem"].as<uint16_t>() );
        if( it == m_level->m_itemNodes.end() )
            BOOST_THROW_EXCEPTION( std::domain_error( "Invalid item reference" ) );
        m_lastItem = it->second.get();
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
    m_cameraYOffset = n["yOffset"].as<int>();
    m_bounce = n["bounce"].as<int>();
    m_eyeCenterDistance = n["eyeCenterDistance"].as<int>();
    m_eyeCenterHorizontalDistanceSq = n["eyeCenterHorizontalDistanceSq"].as<int>();
    m_eyeRotation.load( n["eyeRotation"] );
    m_currentRotation.load( n["currentRotation"] );
    m_trackingSmoothness = n["trackingSmoothness"].as<int>();
    m_fixedCameraId = n["fixedCameraId"].as<int>();
    m_currentFixedCameraId = n["currentFixedCameraId"].as<int>();
    m_camOverrideTimeout = n["camOverrideTimeout"].as<int>();
    m_cinematicFrame = n["cinematicFrame"].as<size_t>();
    m_cinematicPos.load( n["cinematicPos"] );
    m_cinematicRot.load( n["cinematicRot"] );
}
}
