#include "cameracontroller.h"

#include "laranode.h"
#include "level/level.h"
#include "render/portaltracer.h"

#include <queue>


namespace engine
{
    CameraController::CameraController(gsl::not_null<level::Level*> level, gsl::not_null<LaraNode*> laraController, const gsl::not_null<std::shared_ptr<gameplay::Camera>>& camera)
        : m_camera{ camera }
        , m_level{ level }
        , m_laraController{ laraController }
        , m_position{ laraController->getCurrentRoom() }
        , m_target{ laraController->getCurrentRoom(), m_laraController->getPosition() }
        , m_cameraYOffset{ laraController->getPosition().Y - loader::SectorSize }
    {
        m_target.position.Y -= m_cameraYOffset;
        m_position = m_target;
        m_position.position.Z -= 100;

        update();
    }


    void CameraController::setCurrentRotation(core::Angle x, core::Angle y)
    {
        setCurrentRotationX(x);
        setCurrentRotationY(y);
    }


    void CameraController::setCurrentRotationX(core::Angle x)
    {
        m_currentRotation.X = x;
    }


    void CameraController::setCurrentRotationY(core::Angle y)
    {
        m_currentRotation.Y = y;
    }


    void CameraController::setCamOverride(const floordata::CameraParameters& camParams, uint16_t camId, floordata::SequenceCondition condition, bool fromHeavy, const floordata::ActivationState& activationRequest, bool switchIsOn)
    {
        Expects(camId < m_level->m_cameras.size());
        if( m_level->m_cameras[camId].isActive() )
            return;

        m_fixedCameraId = camId;
        if( m_mode == CameraMode::FreeLook || m_mode == CameraMode::Combat || condition == floordata::SequenceCondition::LaraInCombatMode )
            return;

        if( condition == floordata::SequenceCondition::ItemActivated && activationRequest.getTimeout() != 0 && switchIsOn )
            return;

        if( condition != floordata::SequenceCondition::ItemActivated && m_fixedCameraId == m_currentFixedCameraId )
            return;

        if( camParams.timeout != 1 )
            m_camOverrideTimeout = camParams.timeout * core::FrameRate;

        if( camParams.oneshot )
            m_level->m_cameras[camId].setActive(true);

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

            if( command.opcode == floordata::CommandOpcode::LookAt && m_mode != CameraMode::FreeLook && m_mode != CameraMode::Combat )
            {
                m_item = m_level->getItemController(command.parameter);
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

        if( type == CameraMode::Chase || (type == CameraMode::FreeLook && m_item != nullptr && m_item->m_flags2_40_alreadyLookedAt && m_item != m_lastItem) )
            m_item = nullptr;
    }


    void CameraController::tracePortals()
    {
        for( const loader::Room& room : m_level->m_rooms )
            room.node->setVisible(false);

        auto startRoom = m_position.room;
        startRoom->node->setVisible(true);

        // Breadth-first queue
        std::queue<render::PortalTracer> toVisit;

        // always process direct neighbours of the starting room
        for( const loader::Portal& portal : startRoom->portals )
        {
            render::PortalTracer path;
            if( !path.checkVisibility(&portal, *m_camera.get()) )
                continue;

            m_level->m_rooms[portal.adjoining_room].node->setVisible(true);

            toVisit.emplace(std::move(path));
        }

        // Avoid infinite loops
        std::set<const loader::Portal*> visited;
        while( !toVisit.empty() )
        {
            const render::PortalTracer currentPath = std::move(toVisit.front());
            toVisit.pop();

            if( !visited.insert(currentPath.getLastPortal()).second )
            {
                continue; // already tested
            }

            // iterate through the last room's portals and add the destinations if suitable
            uint16_t destRoom = currentPath.getLastDestinationRoom();
            for( const loader::Portal& srcPortal : m_level->m_rooms[destRoom].portals )
            {
                render::PortalTracer newPath = currentPath;
                if( !newPath.checkVisibility(&srcPortal, *m_camera.get()) )
                    continue;

                m_level->m_rooms[srcPortal.adjoining_room].node->setVisible(true);
                toVisit.emplace(std::move(newPath));
            }
        }
    }


    bool CameraController::clampY(const core::TRCoordinates& target, core::TRCoordinates& current, gsl::not_null<const loader::Sector*> sector) const
    {
        const auto d = current - target;
        const HeightInfo floor = HeightInfo::fromFloor(sector, current, this);
        const HeightInfo ceiling = HeightInfo::fromCeiling(sector, current, this);
        BOOST_ASSERT(ceiling.distance < floor.distance);

        if( floor.distance < current.Y && floor.distance > target.Y )
        {
            current.Y = floor.distance;
            current.X = (floor.distance - target.Y) * d.X / d.Y + target.X;
            current.Z = (floor.distance - target.Y) * d.Z / d.Y + target.Z;
            return false;
        }

        if( ceiling.distance > current.Y && ceiling.distance < target.Y )
        {
            current.Y = ceiling.distance;
            current.X = (ceiling.distance - target.Y) * d.X / d.Y + target.X;
            current.Z = (ceiling.distance - target.Y) * d.Z / d.Y + target.Z;
            return false;
        }

        return true;
    }


    CameraController::ClampType CameraController::clampAlongX(core::RoomBoundPosition& current) const
    {
        if( m_target.position.X == current.position.X )
        {
            return ClampType::None;
        }

        const auto d = current.position - m_target.position;

        const int sign = d.X < 0 ? -1 : 1;

        core::TRCoordinates testPos;
        testPos.X = (m_target.position.X / loader::SectorSize) * loader::SectorSize;
        if( sign > 0 )
            testPos.X += loader::SectorSize - 1;

        testPos.Y = m_target.position.Y + (testPos.X - m_target.position.X) * d.Y / d.X;
        testPos.Z = m_target.position.Z + (testPos.X - m_target.position.X) * d.Z / d.X;

        core::TRCoordinates step;
        step.X = sign * loader::SectorSize;
        step.Y = step.X * d.Y / d.X;
        step.Z = step.X * d.Z / d.X;

        auto newRoom = m_target.room;

        while( true )
        {
            if( sign > 0 && testPos.X >= current.position.X )
            {
                current.room = newRoom;
                return ClampType::None;
            }
            if( sign < 0 && testPos.X <= current.position.X )
            {
                current.room = newRoom;
                return ClampType::None;
            }

            core::TRCoordinates heightPos = testPos;
            BOOST_ASSERT(heightPos.X % loader::SectorSize == 0 || heightPos.X % loader::SectorSize == loader::SectorSize - 1);
            auto sector = m_level->findRealFloorSector(heightPos, &newRoom);
            if( testPos.Y > HeightInfo::fromFloor(sector, heightPos, this).distance || testPos.Y < HeightInfo::fromCeiling(sector, heightPos, this).distance )
            {
                current.position = testPos;
                current.room = newRoom;
                return ClampType::Vertical;
            }

            heightPos.X = testPos.X + sign;
            BOOST_ASSERT(heightPos.X % loader::SectorSize == 0 || heightPos.X % loader::SectorSize == loader::SectorSize - 1);
            const auto testRoom = newRoom;
            sector = m_level->findRealFloorSector(heightPos, &newRoom);
            if( testPos.Y > HeightInfo::fromFloor(sector, heightPos, this).distance || testPos.Y < HeightInfo::fromCeiling(sector, heightPos, this).distance )
            {
                current.position = testPos;
                current.room = testRoom;
                return ClampType::Horizontal;
            }

            testPos += step;
        }
    }


    CameraController::ClampType CameraController::clampAlongZ(core::RoomBoundPosition& current) const
    {
        if( m_target.position.Z == current.position.Z )
        {
            return ClampType::None;
        }

        const auto d = current.position - m_target.position;

        const int sign = d.Z < 0 ? -1 : 1;

        core::TRCoordinates testPos;
        testPos.Z = (m_target.position.Z / loader::SectorSize) * loader::SectorSize;

        if( sign > 0 )
            testPos.Z += loader::SectorSize - 1;

        testPos.X = m_target.position.X + (testPos.Z - m_target.position.Z) * d.X / d.Z;
        testPos.Y = m_target.position.Y + (testPos.Z - m_target.position.Z) * d.Y / d.Z;

        core::TRCoordinates step;
        step.Z = sign * loader::SectorSize;
        step.X = step.Z * d.X / d.Z;
        step.Y = step.Z * d.Y / d.Z;

        auto newRoom = m_target.room;

        while( true )
        {
            if( sign > 0 && testPos.Z >= current.position.Z )
            {
                current.room = newRoom;
                return ClampType::None;
            }
            if( sign < 0 && testPos.Z <= current.position.Z )
            {
                current.room = newRoom;
                return ClampType::None;
            }

            core::TRCoordinates heightPos = testPos;
            BOOST_ASSERT(heightPos.Z % loader::SectorSize == 0 || heightPos.Z % loader::SectorSize == loader::SectorSize - 1);
            auto sector = m_level->findRealFloorSector(heightPos, &newRoom);
            if( testPos.Y > HeightInfo::fromFloor(sector, heightPos, this).distance || testPos.Y < HeightInfo::fromCeiling(sector, heightPos, this).distance )
            {
                current.position = testPos;
                current.room = newRoom;
                return ClampType::Vertical;
            }

            heightPos.Z = testPos.Z + sign;
            BOOST_ASSERT(heightPos.Z % loader::SectorSize == 0 || heightPos.Z % loader::SectorSize == loader::SectorSize - 1);
            const auto testRoom = newRoom;
            sector = m_level->findRealFloorSector(heightPos, &newRoom);
            if( testPos.Y > HeightInfo::fromFloor(sector, heightPos, this).distance || testPos.Y < HeightInfo::fromCeiling(sector, heightPos, this).distance )
            {
                current.position = testPos;
                current.room = testRoom;
                return ClampType::Horizontal;
            }

            testPos += step;
        }
    }


    bool CameraController::clampPosition(core::RoomBoundPosition& current) const
    {
        //BOOST_ASSERT(m_target.room->isInnerPositionXZ(m_target.goalPosition));

        bool firstUnclamped;
        ClampType secondClamp;
        if( std::abs(current.position.Z - m_target.position.Z) <= std::abs(current.position.X - m_target.position.X) )
        {
            firstUnclamped = clampAlongZ(current) == ClampType::None;
            secondClamp = clampAlongX(current);
        }
        else
        {
            firstUnclamped = clampAlongX(current) == ClampType::None;
            secondClamp = clampAlongZ(current);
        }

        if( secondClamp != ClampType::Horizontal )
        {
            return false;
        }

        auto sector = m_level->findRealFloorSector(current);
        return clampY(m_target.position, current.position, sector) && firstUnclamped && secondClamp == ClampType::None;
    }


    void CameraController::update()
    {
        m_currentRotation.X = util::clamp(m_currentRotation.X, -85_deg, +85_deg);

        if( m_position.room->isWaterRoom() )
        {
            if( m_level->m_cdStream != nullptr )
                m_level->m_cdStream->getSource().setDirectFilter(m_level->m_audioDev.getUnderwaterFilter());
            if( m_underwaterAmbience == nullptr )
            {
                m_underwaterAmbience = m_level->playSound(60, boost::none);
                m_underwaterAmbience->setLooping(true);
            }
        }
        else if( m_underwaterAmbience != nullptr )
        {
            if( m_level->m_cdStream != nullptr )
                m_level->m_cdStream->getSource().setDirectFilter(nullptr);
            m_level->stopSoundEffect(60);
            m_underwaterAmbience.reset();
        }

        if( m_mode == CameraMode::Cinematic )
        {
            //! @todo nextCinematicFrame();
            return;
        }

        if( m_oldMode != CameraMode::FreeLook )
            HeightInfo::skipSteepSlants = true;

        const bool tracking = m_item != nullptr && (m_mode == CameraMode::Fixed || m_mode == CameraMode::Heavy);

        items::ItemNode* trackedItem = tracking ? m_item : m_laraController;
        auto trackedBBox = trackedItem->getNode()->getBoundingBox();
        int trackedY = trackedItem->getPosition().Y;
        if( tracking )
            trackedY += (trackedBBox.minY + trackedBBox.maxY) / 2;
        else
            trackedY += (trackedBBox.minY - trackedBBox.maxY) * 3 / 4 + trackedBBox.maxY;

        if( m_item != nullptr && !tracking )
        {
            BOOST_ASSERT(m_item != trackedItem);
            BOOST_ASSERT(trackedItem);
            const auto distToTarget = m_item->getPosition().distanceTo(trackedItem->getPosition());
            auto trackAngleY = core::Angle::fromAtan(m_item->getPosition().X - trackedItem->getPosition().X, m_item->getPosition().Z - trackedItem->getPosition().Z) - trackedItem->getRotation().Y;
            trackAngleY *= 0.5f;
            trackedBBox = m_item->getNode()->getBoundingBox();
            auto trackAngleX = core::Angle::fromAtan(distToTarget, trackedY - (trackedBBox.minY + trackedBBox.maxY) / 2 + m_item->getPosition().Y);
            trackAngleX *= 0.5f;

            if( trackAngleY < 50_deg && trackAngleY > -50_deg && trackAngleX < 85_deg && trackAngleX > -85_deg )
            {
                trackAngleY -= m_laraController->m_headRotation.Y;
                if( trackAngleY > 4_deg )
                    m_laraController->m_headRotation.Y += 4_deg;
                else if( trackAngleY < -4_deg )
                    m_laraController->m_headRotation.Y -= 4_deg;
                else
                    m_laraController->m_headRotation.Y += trackAngleY;
                m_laraController->m_torsoRotation.Y = m_laraController->m_headRotation.Y;

                trackAngleX -= m_laraController->m_headRotation.X;
                if( trackAngleX > 4_deg )
                    m_laraController->m_headRotation.X += 4_deg;
                else if( trackAngleX < -4_deg )
                    m_laraController->m_headRotation.X -= 4_deg;
                else
                    m_laraController->m_headRotation.X += trackAngleX;
                m_laraController->m_torsoRotation.X = m_laraController->m_headRotation.X;

                m_mode = CameraMode::FreeLook;
                m_item->m_flags2_40_alreadyLookedAt = true;
            }
        }

        m_target.room = trackedItem->getCurrentRoom();

        if( m_mode != CameraMode::FreeLook && m_mode != CameraMode::Combat )
        {
            m_target.position.X = trackedItem->getPosition().X;
            m_target.position.Z = trackedItem->getPosition().Z;

            if( m_oldMode == CameraMode::Fixed )
            {
                const auto midZ = (trackedBBox.minZ + trackedBBox.maxZ) / 2;
                m_target.position.Z += midZ * trackedItem->getRotation().Y.cos();
                m_target.position.X += midZ * trackedItem->getRotation().Y.sin();
            }

            if( m_tracking == tracking )
            {
                m_tracking = false;
                //! @todo check formula
                m_target.position.Y += (trackedY - m_target.position.Y) / 4;
            }
            else
            {
                m_tracking = true;
                m_target.position.Y = trackedY;
                m_trackingSmoothness = 1;
            }

            auto sector = m_level->findRealFloorSector(m_target);
            if( HeightInfo::fromFloor(sector, m_target.position, this).distance < m_target.position.Y )
                HeightInfo::skipSteepSlants = false;

            if( m_mode != CameraMode::Chase && m_oldMode != CameraMode::Combat )
                handleCamOverride();
            else
                doUsualMovement(trackedItem);
        }
        else
        {
            if( m_tracking )
            {
                m_target.position.Y = trackedY - loader::QuarterSectorSize;
                m_trackingSmoothness = 1;
            }
            else
            {
                m_target.position.Y += (trackedY - loader::QuarterSectorSize - m_target.position.Y) / 4;
                if( m_mode == CameraMode::FreeLook )
                    m_trackingSmoothness = 4;
                else
                    m_trackingSmoothness = 8;
            }
            m_tracking = false;
            if( m_mode == CameraMode::FreeLook )
                handleFreeLook(*trackedItem);
            else
                handleEnemy(*trackedItem);
        }

        m_tracking = tracking;
        m_currentFixedCameraId = m_fixedCameraId;
        if( m_mode != CameraMode::Heavy || m_camOverrideTimeout < 0 )
        {
            m_mode = CameraMode::Chase;
            m_lastItem = m_item;
            m_currentRotation.X = m_currentRotation.Y = 0_deg;
            m_targetDistance = 1536;
            m_fixedCameraId = -1;
            m_item = nullptr;
            m_oldMode = CameraMode::Chase;
        }
        HeightInfo::skipSteepSlants = false;

        tracePortals();
    }


    void CameraController::handleCamOverride()
    {
        Expects(m_fixedCameraId >= 0 && gsl::narrow_cast<size_t>(m_fixedCameraId) < m_level->m_cameras.size());
        Expects(m_level->m_cameras[m_fixedCameraId].room < m_level->m_rooms.size());

        core::RoomBoundPosition pos(&m_level->m_rooms[m_level->m_cameras[m_fixedCameraId].room]);
        pos.position = m_level->m_cameras[m_fixedCameraId].position;

        if( !clampPosition(pos) )
            moveIntoGeometry(pos, loader::QuarterSectorSize);

        m_tracking = true;
        updatePosition(pos, m_trackingSmoothness);

        //! @todo Check condition
        if( m_camOverrideTimeout > 1 )
            --m_camOverrideTimeout;
        else
            m_camOverrideTimeout = -1;
    }


    int CameraController::moveIntoGeometry(core::RoomBoundPosition& pos, int margin) const
    {
        auto sector = m_level->findRealFloorSector(pos);
        BOOST_ASSERT(sector->boxIndex < m_level->m_boxes.size());
        const loader::Box& box = m_level->m_boxes[sector->boxIndex];

        if( box.zmin + margin > pos.position.Z && isVerticallyOutsideRoom(pos.position - core::TRCoordinates(0, 0, margin), pos.room) )
            pos.position.Z = box.zmin + margin;
        else if( box.zmax - margin > pos.position.Z && isVerticallyOutsideRoom(pos.position + core::TRCoordinates(0, 0, margin), pos.room) )
            pos.position.Z = box.zmax - margin;

        if( box.xmin + margin > pos.position.X && isVerticallyOutsideRoom(pos.position - core::TRCoordinates(margin, 0, 0), pos.room) )
            pos.position.X = box.xmin + margin;
        else if( box.xmax - margin > pos.position.X && isVerticallyOutsideRoom(pos.position + core::TRCoordinates(margin, 0, 0), pos.room) )
            pos.position.X = box.xmax - margin;

        auto bottom = HeightInfo::fromFloor(sector, pos.position, this).distance - margin;
        auto top = HeightInfo::fromCeiling(sector, pos.position, this).distance + margin;
        if( bottom < top )
            top = bottom = (bottom + top) / 2;

        if( pos.position.Y > bottom )
            return bottom - pos.position.Y;
        if( top > pos.position.Y )
            return top - pos.position.Y;
        return 0;
    }


    bool CameraController::isVerticallyOutsideRoom(const core::TRCoordinates& pos, const gsl::not_null<const loader::Room*>& room) const
    {
        gsl::not_null<const loader::Sector*> sector = m_level->findRealFloorSector(pos, room);
        const auto floor = HeightInfo::fromFloor(sector, pos, this).distance;
        const auto ceiling = HeightInfo::fromCeiling(sector, pos, this).distance;
        return pos.Y > floor || pos.Y <= ceiling;
    }


    void CameraController::updatePosition(const core::RoomBoundPosition& goalPosition, int smoothFactor)
    {
        m_position.position += (goalPosition.position - m_position.position) / smoothFactor;
        HeightInfo::skipSteepSlants = false;
        m_position.room = goalPosition.room;
        auto sector = m_level->findRealFloorSector(m_position);
        auto floor = HeightInfo::fromFloor(sector, m_position.position, this).distance - loader::QuarterSectorSize;
        if( floor <= m_position.position.Y && floor <= goalPosition.position.Y )
        {
            clampPosition(m_position);
            sector = m_level->findRealFloorSector(m_position);
            floor = HeightInfo::fromFloor(sector, m_position.position, this).distance - loader::QuarterSectorSize;
        }

        auto ceiling = HeightInfo::fromCeiling(sector, m_position.position, this).distance + loader::QuarterSectorSize;
        if( floor < ceiling )
        {
            floor = ceiling = (floor + ceiling) / 2;
        }

        if( m_bounce != 0 )
        {
            if( m_bounce < 0 )
            {
                //! @todo cam shake
                m_bounce += 5;
            }
            else
            {
                m_bounce = 0;
            }
        }

        if( m_position.position.Y > floor )
            m_cameraYOffset = floor - m_position.position.Y;
        else if( m_position.position.Y < ceiling )
            m_cameraYOffset = ceiling - m_position.position.Y;
        else
            m_cameraYOffset = 0;

        auto camPos = m_position.position;
        camPos.Y += m_cameraYOffset;

        // update current room
        m_level->findRealFloorSector(camPos, &m_position.room);

        auto m = glm::lookAt(camPos.toRenderSystem(), m_target.position.toRenderSystem(), {0,1,0});
        m_camera->setViewMatrix(m);
    }


    void CameraController::doUsualMovement(const gsl::not_null<const items::ItemNode*>& item)
    {
        m_currentRotation.X += item->getRotation().X;
        if( m_currentRotation.X > 85_deg )
            m_currentRotation.X = 85_deg;
        else if( m_currentRotation.X < -85_deg )
            m_currentRotation.X = -85_deg;

        auto dist = m_currentRotation.X.cos() * m_targetDistance;
        m_targetDistanceSq = gsl::narrow_cast<int>(dist * dist);

        core::RoomBoundPosition targetPos(m_position.room);
        targetPos.position.Y = m_targetDistance * m_currentRotation.X.sin() + m_target.position.Y;

        core::Angle y = m_currentRotation.Y + item->getRotation().Y;
        targetPos.position.X = m_target.position.X - dist * y.sin();
        targetPos.position.Z = m_target.position.Z - dist * y.cos();
        clampBox(targetPos, [this](int& a, int& b, int c, int d, int e, int f, int g, int h)
             {
                 clampToCorners(m_targetDistanceSq, a, b, c, d, e, f, g, h);
             });

        updatePosition(targetPos, m_tracking ? m_trackingSmoothness : 12);
    }


    void CameraController::handleFreeLook(const items::ItemNode& item)
    {
        const auto originalPivotPosition = m_target.position;
        m_target.position.X = item.getPosition().X;
        m_target.position.Z = item.getPosition().Z;
        m_currentRotation.X = m_laraController->m_torsoRotation.X + m_laraController->m_headRotation.X + item.getRotation().X;
        m_currentRotation.Y = m_laraController->m_torsoRotation.Y + m_laraController->m_headRotation.Y + item.getRotation().Y;
        m_targetDistance = 1536;
        m_cameraYOffset = gsl::narrow_cast<int>(-2 * loader::QuarterSectorSize * m_currentRotation.Y.sin());
        m_target.position.X += m_cameraYOffset * item.getRotation().Y.sin();
        m_target.position.Z += m_cameraYOffset * item.getRotation().Y.cos();

        if( isVerticallyOutsideRoom(m_target.position, m_position.room) )
        {
            m_target.position.X = item.getPosition().X;
            m_target.position.Z = item.getPosition().Z;
        }

        m_target.position.Y += moveIntoGeometry(m_target, loader::QuarterSectorSize + 50);

        auto cameraPosition = m_target;
        cameraPosition.position.X -= m_targetDistance * m_currentRotation.Y.sin() * m_currentRotation.X.cos();
        cameraPosition.position.Z -= m_targetDistance * m_currentRotation.Y.cos() * m_currentRotation.X.cos();
        cameraPosition.position.Y += m_targetDistance * m_currentRotation.X.sin();
        cameraPosition.room = m_position.room;

        clampBox(cameraPosition, &freeLookClamp);

        m_target.position.X = originalPivotPosition.X + (m_target.position.X - originalPivotPosition.X) / m_trackingSmoothness;
        m_target.position.Z = originalPivotPosition.Z + (m_target.position.Z - originalPivotPosition.Z) / m_trackingSmoothness;

        updatePosition(cameraPosition, m_trackingSmoothness);
    }


    void CameraController::handleEnemy(const items::ItemNode& item)
    {
        m_target.position.X = item.getPosition().X;
        m_target.position.Z = item.getPosition().Z;

        if( m_enemy != nullptr )
        {
            m_currentRotation.X = m_targetRotation.X + item.getRotation().X;
            m_currentRotation.Y = m_targetRotation.Y + item.getRotation().Y;
        }
        else
        {
            m_currentRotation.X = m_laraController->m_torsoRotation.X + m_laraController->m_headRotation.X + item.getRotation().X;
            m_currentRotation.Y = m_laraController->m_torsoRotation.Y + m_laraController->m_headRotation.Y + item.getRotation().Y;
        }

        m_targetDistance = 2560;
        auto tmp = m_target;
        auto d = m_targetDistance * m_currentRotation.X.cos();
        tmp.position.X -= d * m_currentRotation.Y.sin();
        tmp.position.Z -= d * m_currentRotation.Y.cos();
        tmp.position.Y += m_targetDistance * m_currentRotation.X.sin();
        tmp.room = m_position.room;

        clampBox(tmp, [this](int& a, int& b, int c, int d, int e, int f, int g, int h)
             {
                 clampToCorners(m_targetDistanceSq, a, b, c, d, e, f, g, h);
             });
        updatePosition(tmp, m_trackingSmoothness);
    }


    void CameraController::clampBox(core::RoomBoundPosition& goalPosition, const std::function<ClampCallback>& callback) const
    {
        clampPosition(goalPosition);
        BOOST_ASSERT(m_target.room->getSectorByAbsolutePosition(m_target.position)->boxIndex < m_level->m_boxes.size());
        auto clampBox = &m_level->m_boxes[m_target.room->getSectorByAbsolutePosition(m_target.position)->boxIndex];
        BOOST_ASSERT(goalPosition.room->getSectorByAbsolutePosition(goalPosition.position) != nullptr);
        if( goalPosition.room->getSectorByAbsolutePosition(goalPosition.position)->boxIndex != 0xffff )
        {
            if( goalPosition.position.X < clampBox->xmin || goalPosition.position.X > clampBox->xmax
                || goalPosition.position.Z < clampBox->zmin || goalPosition.position.Z > clampBox->zmax )
                clampBox = &m_level->m_boxes[goalPosition.room->getSectorByAbsolutePosition(goalPosition.position)->boxIndex];
        }

        core::TRCoordinates testPos = goalPosition.position;
        testPos.Z = (testPos.Z / loader::SectorSize) * loader::SectorSize - 1;
        BOOST_ASSERT(testPos.Z % loader::SectorSize == loader::SectorSize - 1 && std::abs(testPos.Z - goalPosition.position.Z) <= loader::SectorSize);

        auto clampZMin = clampBox->zmin;
        const bool negZverticalOutside = isVerticallyOutsideRoom(testPos, goalPosition.room);
        if( !negZverticalOutside && m_level->findRealFloorSector(testPos, goalPosition.room)->boxIndex != 0xffff )
        {
            auto testBox = &m_level->m_boxes[m_level->findRealFloorSector(testPos, goalPosition.room)->boxIndex];
            if( testBox->zmin < clampZMin )
                clampZMin = testBox->zmin;
        }
        clampZMin += loader::QuarterSectorSize;

        testPos = goalPosition.position;
        testPos.Z = (testPos.Z / loader::SectorSize + 1) * loader::SectorSize;
        BOOST_ASSERT(testPos.Z % loader::SectorSize == 0 && std::abs(testPos.Z - goalPosition.position.Z) <= loader::SectorSize);

        auto clampZMax = clampBox->zmax;
        const bool posZverticalOutside = isVerticallyOutsideRoom(testPos, goalPosition.room);
        if( !posZverticalOutside && m_level->findRealFloorSector(testPos, goalPosition.room)->boxIndex != 0xffff )
        {
            auto testBox = &m_level->m_boxes[m_level->findRealFloorSector(testPos, goalPosition.room)->boxIndex];
            if( testBox->zmax > clampZMax )
                clampZMax = testBox->zmax;
        }
        clampZMax -= loader::QuarterSectorSize;

        testPos = goalPosition.position;
        testPos.X = (testPos.X / loader::SectorSize) * loader::SectorSize - 1;
        BOOST_ASSERT(testPos.X % loader::SectorSize == loader::SectorSize - 1 && std::abs(testPos.X - goalPosition.position.X) <= loader::SectorSize);

        auto clampXMin = clampBox->xmin;
        const bool negXverticalOutside = isVerticallyOutsideRoom(testPos, goalPosition.room);
        if( !negXverticalOutside && m_level->findRealFloorSector(testPos, goalPosition.room)->boxIndex != 0xffff )
        {
            auto testBox = &m_level->m_boxes[m_level->findRealFloorSector(testPos, goalPosition.room)->boxIndex];
            if( testBox->xmin < clampXMin )
                clampXMin = testBox->xmin;
        }
        clampXMin += loader::QuarterSectorSize;

        testPos = goalPosition.position;
        testPos.X = (testPos.X / loader::SectorSize + 1) * loader::SectorSize;
        BOOST_ASSERT(testPos.X % loader::SectorSize == 0 && std::abs(testPos.X - goalPosition.position.X) <= loader::SectorSize);

        auto clampXMax = clampBox->xmax;
        const bool posXverticalOutside = isVerticallyOutsideRoom(testPos, goalPosition.room);
        if( !posXverticalOutside && m_level->findRealFloorSector(testPos, goalPosition.room)->boxIndex != 0xffff )
        {
            auto testBox = &m_level->m_boxes[m_level->findRealFloorSector(testPos, goalPosition.room)->boxIndex];
            if( testBox->xmax > clampXMax )
                clampXMax = testBox->xmax;
        }
        clampXMax -= loader::QuarterSectorSize;

        bool skipRoomPatch = true;
        if( negZverticalOutside && goalPosition.position.Z < clampZMin )
        {
            skipRoomPatch = false;
            int left, right;
            if( goalPosition.position.X >= m_target.position.X )
            {
                left = clampXMin;
                right = clampXMax;
            }
            else
            {
                left = clampXMax;
                right = clampXMin;
            }
            callback(goalPosition.position.Z, goalPosition.position.X, m_target.position.Z, m_target.position.X, clampZMin, right, clampZMax, left);
            // BOOST_ASSERT(m_target.goalPosition.distanceTo(goalPosition.goalPosition) <= 2 * m_targetDistance); // sanity check
        }
        else if( posZverticalOutside && goalPosition.position.Z > clampZMax )
        {
            skipRoomPatch = false;
            int left, right;
            if( goalPosition.position.X >= m_target.position.X )
            {
                left = clampXMin;
                right = clampXMax;
            }
            else
            {
                left = clampXMax;
                right = clampXMin;
            }
            callback(goalPosition.position.Z, goalPosition.position.X, m_target.position.Z, m_target.position.X, clampZMax, right, clampZMin, left);
            // BOOST_ASSERT(m_target.goalPosition.distanceTo(goalPosition.goalPosition) <= 2 * m_targetDistance); // sanity check
        }

        if( !skipRoomPatch )
        {
            m_level->findRealFloorSector(goalPosition);
            return;
        }

        if( negXverticalOutside && goalPosition.position.X < clampXMin )
        {
            skipRoomPatch = false;
            int left, right;
            if( goalPosition.position.Z >= m_target.position.Z )
            {
                left = clampZMin;
                right = clampZMax;
            }
            else
            {
                left = clampZMax;
                right = clampZMin;
            }
            callback(goalPosition.position.X, goalPosition.position.Z, m_target.position.X, m_target.position.Z, clampXMin, right, clampXMax, left);
            // BOOST_ASSERT(m_target.goalPosition.distanceTo(goalPosition.goalPosition) <= 2 * m_targetDistance); // sanity check
        }
        else if( posXverticalOutside && goalPosition.position.X > clampXMax )
        {
            skipRoomPatch = false;
            int left, right;
            if( goalPosition.position.Z >= m_target.position.Z )
            {
                left = clampZMin;
                right = clampZMax;
            }
            else
            {
                left = clampZMax;
                right = clampZMin;
            }
            callback(goalPosition.position.X, goalPosition.position.Z, m_target.position.X, m_target.position.Z, clampXMax, right, clampXMin, left);
            // BOOST_ASSERT(m_target.goalPosition.distanceTo(goalPosition.goalPosition) <= 2 * m_targetDistance); // sanity check
        }

        if( !skipRoomPatch )
        {
            m_level->findRealFloorSector(goalPosition);
        }
    }


    void CameraController::freeLookClamp(int& currentFrontBack, int& currentLeftRight, int targetFrontBack, int targetLeftRight, int back, int right, int front, int left)
    {
        if( (front > back) != (targetFrontBack < back) )
        {
            currentFrontBack = back;
            currentLeftRight = targetLeftRight + (currentLeftRight - targetLeftRight) * (back - targetFrontBack) / (currentFrontBack - targetFrontBack);
        }
        else if( (right < left && targetLeftRight > right && right > currentLeftRight) || (right > left && targetLeftRight < right && right < currentLeftRight) )
        {
            currentFrontBack = targetFrontBack + (currentFrontBack - targetFrontBack) * (right - targetLeftRight) / (currentLeftRight - targetLeftRight);
            currentLeftRight = right;
        }
    }


    void CameraController::clampToCorners(const int pivotDistanceSq, int& currentFrontBack, int& currentLeftRight, int targetFrontBack, int targetLeftRight, int back, int right, int front, int left)
    {
        const auto targetRightDistSq = (targetLeftRight - right) * (targetLeftRight - right);
        const auto targetBackDistSq = (targetFrontBack - back) * (targetFrontBack - back);

        // back right
        const auto backRightDistSq = targetBackDistSq + targetRightDistSq;
        if( backRightDistSq > pivotDistanceSq )
        {
            //BOOST_LOG_TRIVIAL(debug) << "Clamp back right: " << currentFrontBack << " => " << back;
            //BOOST_LOG_TRIVIAL(debug) << "Left = " << left << ", right = " << right << ", front = " << front << ", back = " << back;
            currentFrontBack = back;
            if( pivotDistanceSq >= targetBackDistSq )
            {
                auto tmp = std::sqrt(pivotDistanceSq - targetBackDistSq);
                if( right < left )
                    tmp = -tmp;
                currentLeftRight = tmp + targetLeftRight;
            }
            return;
        }

        if( backRightDistSq > loader::QuarterSectorSize * loader::QuarterSectorSize )
        {
            currentFrontBack = back;
            currentLeftRight = right;
            return;
        }

        // back left
        const auto targetLeftDistSq = (targetLeftRight - left) * (targetLeftRight - left);
        const auto targetBackLeftDistSq = targetBackDistSq + targetLeftDistSq;
        if( targetBackLeftDistSq > pivotDistanceSq )
        {
            //BOOST_LOG_TRIVIAL(debug) << "Clamp back left: " << currentFrontBack << " => " << back;
            //BOOST_LOG_TRIVIAL(debug) << "Left = " << left << ", right = " << right << ", front = " << front << ", back = " << back;
            currentFrontBack = back;
            if( pivotDistanceSq >= targetBackDistSq )
            {
                auto tmp = std::sqrt(pivotDistanceSq - targetBackDistSq);
                if( right >= left )
                    tmp = -tmp;
                currentLeftRight = tmp + targetLeftRight;
            }
            return;
        }

        if( targetBackLeftDistSq > loader::QuarterSectorSize * loader::QuarterSectorSize )
        {
            currentFrontBack = back;
            currentLeftRight = left;
            return;
        }

        // front right
        const auto targetFrontDistSq = (targetFrontBack - front) * (targetFrontBack - front);
        const auto targetFrontRightDistSq = targetFrontDistSq + targetRightDistSq;

        if( targetFrontRightDistSq > pivotDistanceSq )
        {
            //BOOST_LOG_TRIVIAL(debug) << "Clamp front right";
            //BOOST_LOG_TRIVIAL(debug) << "Left = " << left << ", right = " << right << ", front = " << front << ", back = " << back;
            if( pivotDistanceSq >= targetRightDistSq )
            {
                currentLeftRight = right;
                auto tmp = std::sqrt(pivotDistanceSq - targetRightDistSq);
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
}
