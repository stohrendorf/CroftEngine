#include "cameracontroller.h"

#include "laranode.h"
#include "level/level.h"
#include "render/portaltracer.h"

#include <queue>


namespace engine
{
    CameraController::CameraController(gsl::not_null<level::Level*> level, gsl::not_null<LaraNode*> laraController, const gsl::not_null<std::shared_ptr<gameplay::Camera>>& camera)
        : m_camera(camera)
        , m_level(level)
        , m_laraController(laraController)
        , m_cameraYOffset(gsl::narrow_cast<int>(laraController->getPosition().Y - 1024))
        , m_pivot(laraController->getCurrentRoom(), m_laraController->getPosition().toInexact())
        , m_currentPosition(laraController->getCurrentRoom())
    {
        m_pivot.position.Y -= m_cameraYOffset;
        m_currentPosition = m_pivot;
        m_currentPosition.position.Z -= 100;

        update(std::chrono::microseconds(1));
    }


    void CameraController::setLocalRotation(core::Angle x, core::Angle y)
    {
        setLocalRotationX(x);
        setLocalRotationY(y);
    }


    void CameraController::setLocalRotationX(core::Angle x)
    {
        m_globalRotation.X = x;
    }


    void CameraController::setLocalRotationY(core::Angle y)
    {
        m_globalRotation.Y = y;
    }


    void CameraController::setCamOverride(const floordata::CameraParameters& camParams, uint16_t camId, floordata::SequenceCondition condition, bool isNotLara, const floordata::ActivationState& activationRequest, bool switchIsOn)
    {
        Expects(camId < m_level->m_cameras.size());
        if( m_level->m_cameras[camId].isActive() )
            return;

        m_camOverrideId = camId;
        if( m_camOverrideType == CamOverrideType::FreeLook || m_camOverrideType == CamOverrideType::_3 || condition == floordata::SequenceCondition::LaraInCombatMode )
            return;

        if( condition == floordata::SequenceCondition::ItemActivated && activationRequest.getTimeout() != std::chrono::microseconds::zero() && switchIsOn )
            return;

        if( condition != floordata::SequenceCondition::ItemActivated && m_camOverrideId == m_activeCamOverrideId )
            return;

        if( camParams.timeout != 1 )
            m_camOverrideTimeout = std::chrono::seconds(camParams.timeout);

        if( camParams.oneshot )
            m_level->m_cameras[camId].setActive(true);

        m_pivotMovementSmoothness = 1 + (camParams.smoothness * 4);
        if( isNotLara )
            m_camOverrideType = CamOverrideType::NotActivatedByLara;
        else
            m_camOverrideType = CamOverrideType::ActivatedByLara;
    }


    void CameraController::findCameraTarget(const uint16_t* cmdSequence)
    {
        if( m_camOverrideType == CamOverrideType::ActivatedByLara )
            return;

        CamOverrideType type = CamOverrideType::FreeLook;
        while( true )
        {
            const floordata::Command command{*cmdSequence++};

            if( command.opcode == floordata::CommandOpcode::LookAt && m_camOverrideType != CamOverrideType::FreeLook && m_camOverrideType != CamOverrideType::_3 )
            {
                m_itemOfInterest = m_level->getItemController(command.parameter);
            }
            else if( command.opcode == floordata::CommandOpcode::SwitchCamera )
            {
                ++cmdSequence; // skip camera parameters

                if( command.parameter != m_activeCamOverrideId )
                {
                    type = CamOverrideType::None; // new override
                }
                else
                {
                    m_camOverrideId = m_activeCamOverrideId;
                    if( m_camOverrideTimeout >= std::chrono::microseconds::zero() && m_camOverrideType != CamOverrideType::FreeLook && m_camOverrideType != CamOverrideType::_3 )
                    {
                        type = CamOverrideType::NotActivatedByLara;
                        m_camOverrideType = CamOverrideType::NotActivatedByLara;
                    }
                    else
                    {
                        type = CamOverrideType::None;
                        m_camOverrideTimeout = std::chrono::microseconds(-1);
                    }
                }
            }

            if( command.isLast )
                break;
        }

        if( type == CamOverrideType::None || (type == CamOverrideType::FreeLook && m_itemOfInterest != nullptr && m_itemOfInterest->m_flags2_40_alreadyLookedAt && m_itemOfInterest != m_previousItemOfInterest) )
            m_itemOfInterest = nullptr;
    }


    void CameraController::tracePortals()
    {
        for( const loader::Room& room : m_level->m_rooms )
            room.node->setEnabled(false);

        auto startRoom = m_currentPosition.room;
        startRoom->node->setEnabled(true);

        // Breadth-first queue
        std::queue<render::PortalTracer> toVisit;

        // always process direct neighbours of the starting room
        for( const loader::Portal& portal : startRoom->portals )
        {
            render::PortalTracer path;
            if( !path.checkVisibility(&portal, *m_camera.get()) )
                continue;

            m_level->m_rooms[portal.adjoining_room].node->setEnabled(true);

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

                m_level->m_rooms[srcPortal.adjoining_room].node->setEnabled(true);
                toVisit.emplace(std::move(newPath));
            }
        }
    }


    bool CameraController::clampY(const core::TRCoordinates& lookAt, core::TRCoordinates& origin, gsl::not_null<const loader::Sector*> sector) const
    {
        const auto d = origin - lookAt;
        const HeightInfo floor = HeightInfo::fromFloor(sector, origin, this);
        const HeightInfo ceiling = HeightInfo::fromCeiling(sector, origin, this);
        BOOST_ASSERT(ceiling.distance < floor.distance);

        if( floor.distance < origin.Y && floor.distance > lookAt.Y )
        {
            origin.Y = floor.distance;
            origin.X = (floor.distance - lookAt.Y) * d.X / d.Y + lookAt.X;
            origin.Z = (floor.distance - lookAt.Y) * d.Z / d.Y + lookAt.Z;
            return false;
        }

        if( ceiling.distance > origin.Y && ceiling.distance < lookAt.Y )
        {
            origin.Y = ceiling.distance;
            origin.X = (ceiling.distance - lookAt.Y) * d.X / d.Y + lookAt.X;
            origin.Z = (ceiling.distance - lookAt.Y) * d.Z / d.Y + lookAt.Z;
            return false;
        }

        return true;
    }


    CameraController::ClampType CameraController::clampAlongX(core::RoomBoundIntPosition& origin) const
    {
        if( m_pivot.position.X == origin.position.X )
        {
            return ClampType::None;
        }

        const auto d = origin.position - m_pivot.position;

        const int sign = d.X < 0 ? -1 : 1;

        core::TRCoordinates testPos;
        testPos.X = (m_pivot.position.X / loader::SectorSize) * loader::SectorSize;
        if( sign > 0 )
            testPos.X += loader::SectorSize - 1;

        testPos.Y = m_pivot.position.Y + (testPos.X - m_pivot.position.X) * d.Y / d.X;
        testPos.Z = m_pivot.position.Z + (testPos.X - m_pivot.position.X) * d.Z / d.X;

        core::TRCoordinates step;
        step.X = sign * loader::SectorSize;
        step.Y = step.X * d.Y / d.X;
        step.Z = step.X * d.Z / d.X;

        auto newRoom = m_pivot.room;

        while( true )
        {
            if( sign > 0 && testPos.X >= origin.position.X )
            {
                origin.room = newRoom;
                return ClampType::None;
            }
            else if( sign < 0 && testPos.X <= origin.position.X )
            {
                origin.room = newRoom;
                return ClampType::None;
            }

            core::TRCoordinates heightPos = testPos;
            BOOST_ASSERT(heightPos.X % loader::SectorSize == 0 || heightPos.X % loader::SectorSize == loader::SectorSize - 1);
            auto sector = m_level->findRealFloorSector(heightPos, &newRoom);
            if( testPos.Y > HeightInfo::fromFloor(sector, heightPos, this).distance || testPos.Y < HeightInfo::fromCeiling(sector, heightPos, this).distance )
            {
                origin.position = testPos;
                origin.room = newRoom;
                return ClampType::Vertical;
            }

            heightPos.X = testPos.X + sign;
            BOOST_ASSERT(heightPos.X % loader::SectorSize == 0 || heightPos.X % loader::SectorSize == loader::SectorSize - 1);
            const auto testRoom = newRoom;
            sector = m_level->findRealFloorSector(heightPos, &newRoom);
            if( testPos.Y > HeightInfo::fromFloor(sector, heightPos, this).distance || testPos.Y < HeightInfo::fromCeiling(sector, heightPos, this).distance )
            {
                origin.position = testPos;
                origin.room = testRoom;
                return ClampType::Horizontal;
            }

            testPos += step;
        }
    }


    CameraController::ClampType CameraController::clampAlongZ(core::RoomBoundIntPosition& origin) const
    {
        if( m_pivot.position.Z == origin.position.Z )
        {
            return ClampType::None;
        }

        const auto d = origin.position - m_pivot.position;

        const int sign = d.Z < 0 ? -1 : 1;

        core::TRCoordinates testPos;
        testPos.Z = (m_pivot.position.Z / loader::SectorSize) * loader::SectorSize;

        if( sign > 0 )
            testPos.Z += loader::SectorSize - 1;

        testPos.X = m_pivot.position.X + (testPos.Z - m_pivot.position.Z) * d.X / d.Z;
        testPos.Y = m_pivot.position.Y + (testPos.Z - m_pivot.position.Z) * d.Y / d.Z;

        core::TRCoordinates step;
        step.Z = sign * loader::SectorSize;
        step.X = step.Z * d.X / d.Z;
        step.Y = step.Z * d.Y / d.Z;

        auto newRoom = m_pivot.room;

        while( true )
        {
            if( sign > 0 && testPos.Z >= origin.position.Z )
            {
                origin.room = newRoom;
                return ClampType::None;
            }
            else if( sign < 0 && testPos.Z <= origin.position.Z )
            {
                origin.room = newRoom;
                return ClampType::None;
            }

            core::TRCoordinates heightPos = testPos;
            BOOST_ASSERT(heightPos.Z % loader::SectorSize == 0 || heightPos.Z % loader::SectorSize == loader::SectorSize - 1);
            auto sector = m_level->findRealFloorSector(heightPos, &newRoom);
            if( testPos.Y > HeightInfo::fromFloor(sector, heightPos, this).distance || testPos.Y < HeightInfo::fromCeiling(sector, heightPos, this).distance )
            {
                origin.position = testPos;
                origin.room = newRoom;
                return ClampType::Vertical;
            }

            heightPos.Z = testPos.Z + sign;
            BOOST_ASSERT(heightPos.Z % loader::SectorSize == 0 || heightPos.Z % loader::SectorSize == loader::SectorSize - 1);
            const auto testRoom = newRoom;
            sector = m_level->findRealFloorSector(heightPos, &newRoom);
            if( testPos.Y > HeightInfo::fromFloor(sector, heightPos, this).distance || testPos.Y < HeightInfo::fromCeiling(sector, heightPos, this).distance )
            {
                origin.position = testPos;
                origin.room = testRoom;
                return ClampType::Horizontal;
            }

            testPos += step;
        }
    }


    bool CameraController::clampPosition(core::RoomBoundIntPosition& goalPosition) const
    {
        //BOOST_ASSERT(m_pivot.room->isInnerPositionXZ(m_pivot.goalPosition));

        bool firstUnclamped;
        ClampType secondClamp;
        if( std::abs(goalPosition.position.Z - m_pivot.position.Z) <= std::abs(goalPosition.position.X - m_pivot.position.X) )
        {
            firstUnclamped = clampAlongZ(goalPosition) == ClampType::None;
            secondClamp = clampAlongX(goalPosition);
        }
        else
        {
            firstUnclamped = clampAlongX(goalPosition) == ClampType::None;
            secondClamp = clampAlongZ(goalPosition);
        }

        if( secondClamp != ClampType::Horizontal )
        {
            return false;
        }

        auto sector = m_level->findRealFloorSector(goalPosition);
        return clampY(m_pivot.position, goalPosition.position, sector) && firstUnclamped && secondClamp == ClampType::None;
    }


    void CameraController::update(const std::chrono::microseconds& deltaTime)
    {
        m_globalRotation.X = util::clamp(m_globalRotation.X, -85_deg, +85_deg);

        if( m_currentPosition.room->isWaterRoom() )
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

        if( m_camOverrideType == CamOverrideType::Cinematic )
        {
            //! @todo nextCinematicFrame();
            return;
        }

        if( m_unknown1 != CamOverrideType::FreeLook)
            HeightInfo::skipSteepSlants = true;

        const bool lookingAtSomething = m_itemOfInterest != nullptr && (m_camOverrideType == CamOverrideType::NotActivatedByLara || m_camOverrideType == CamOverrideType::ActivatedByLara);

        items::ItemNode* lookAtItem = lookingAtSomething ? m_itemOfInterest : m_laraController;
        auto lookAtBbox = lookAtItem->getBoundingBox();
        int lookAtY = gsl::narrow_cast<int>(lookAtItem->getPosition().Y);
        if( lookingAtSomething )
            lookAtY += (lookAtBbox.min.y + lookAtBbox.max.y) / 2;
        else
            lookAtY += (lookAtBbox.min.y - lookAtBbox.max.y) * 3 / 4 + lookAtBbox.max.y;

        if( m_itemOfInterest != nullptr && !lookingAtSomething )
        {
            BOOST_ASSERT(m_itemOfInterest != lookAtItem);
            BOOST_ASSERT(lookAtItem);
            const auto distToLookAt = m_itemOfInterest->getPosition().distanceTo(lookAtItem->getPosition());
            auto lookAtYAngle = core::Angle::fromAtan(m_itemOfInterest->getPosition().X - lookAtItem->getPosition().X, m_itemOfInterest->getPosition().Z - lookAtItem->getPosition().Z) - lookAtItem->getRotation().Y;
            lookAtYAngle *= 0.5f;
            lookAtBbox = m_itemOfInterest->getBoundingBox();
            auto lookAtXAngle = core::Angle::fromAtan(distToLookAt, lookAtY - (lookAtBbox.min.y + lookAtBbox.max.y) / 2 + m_itemOfInterest->getPosition().Y);
            lookAtXAngle *= 0.5f;

            if( lookAtYAngle < 50_deg && lookAtYAngle > -50_deg && lookAtXAngle < 85_deg && lookAtXAngle > -85_deg )
            {
                lookAtYAngle -= m_headRotation.Y;
                if( lookAtYAngle > 4_deg )
                    m_headRotation.Y += core::makeInterpolatedValue(+4_deg).getScaled(deltaTime);
                else if( lookAtYAngle < -4_deg )
                    m_headRotation.Y -= core::makeInterpolatedValue(+4_deg).getScaled(deltaTime);
                else
                    m_headRotation.Y += core::makeInterpolatedValue(lookAtYAngle).getScaled(deltaTime);
                m_torsoRotation.Y = m_headRotation.Y;

                lookAtXAngle -= m_headRotation.X;
                if( lookAtXAngle > 4_deg )
                    m_headRotation.X += core::makeInterpolatedValue(+4_deg).getScaled(deltaTime);
                else if( lookAtXAngle < -4_deg )
                    m_headRotation.X -= core::makeInterpolatedValue(+4_deg).getScaled(deltaTime);
                else
                    m_headRotation.X += core::makeInterpolatedValue(lookAtXAngle).getScaled(deltaTime);
                m_torsoRotation.X = m_headRotation.X;

                m_camOverrideType = CamOverrideType::FreeLook;
                m_itemOfInterest->m_flags2_40_alreadyLookedAt = true;
            }
        }

        m_pivot.room = lookAtItem->getCurrentRoom();

        if( m_camOverrideType != CamOverrideType::FreeLook && m_camOverrideType != CamOverrideType::_3 )
        {
            m_pivot.position.X = std::lround(lookAtItem->getPosition().X);
            m_pivot.position.Z = std::lround(lookAtItem->getPosition().Z);

            if( m_unknown1 == CamOverrideType::NotActivatedByLara )
            {
                const auto midZ = (lookAtBbox.min.z + lookAtBbox.max.z) / 2;
                m_pivot.position.Z += std::lround(midZ * lookAtItem->getRotation().Y.cos());
                m_pivot.position.X += std::lround(midZ * lookAtItem->getRotation().Y.sin());
            }

            if( m_lookingAtSomething == lookingAtSomething )
            {
                //! @todo check formula
                m_pivot.position.Y += (lookAtY - m_pivot.position.Y) / 4;
                m_lookingAtSomething = false;
            }
            else
            {
                m_lookingAtSomething = true;
                m_pivot.position.Y = lookAtY;
                m_pivotMovementSmoothness = 1;
            }

            auto sector = m_level->findRealFloorSector(m_pivot);
            if( HeightInfo::fromFloor(sector, m_pivot.position, this).distance < m_pivot.position.Y )
                HeightInfo::skipSteepSlants = false;

            if( m_camOverrideType != CamOverrideType::None && m_unknown1 != CamOverrideType::_3)
                handleCamOverride(deltaTime);
            else
                doUsualMovement(lookAtItem, deltaTime);
        }
        else
        {
            if( m_lookingAtSomething )
            {
                m_pivot.position.Y = lookAtY - loader::QuarterSectorSize;
                m_pivotMovementSmoothness = 1;
            }
            else
            {
                m_pivot.position.Y += (lookAtY - loader::QuarterSectorSize - m_pivot.position.Y) / 4;
                if( m_camOverrideType == CamOverrideType::FreeLook )
                    m_pivotMovementSmoothness = 4;
                else
                    m_pivotMovementSmoothness = 8;
            }
            m_lookingAtSomething = false;
            if( m_camOverrideType == CamOverrideType::FreeLook )
                handleFreeLook(*lookAtItem, deltaTime);
            else
                handleEnemy(*lookAtItem, deltaTime);
        }

        m_lookingAtSomething = lookingAtSomething;
        m_activeCamOverrideId = m_camOverrideId;
        if( m_camOverrideType != CamOverrideType::ActivatedByLara || m_camOverrideTimeout < std::chrono::microseconds::zero() )
        {
            m_camOverrideType = CamOverrideType::None;
            m_previousItemOfInterest = m_itemOfInterest;
            m_globalRotation.X = m_globalRotation.Y = 0_deg;
            m_pivotDistance = 1536;
            m_camOverrideId = -1;
            m_itemOfInterest = nullptr;
            m_unknown1 = CamOverrideType::None;
        }
        HeightInfo::skipSteepSlants = false;

        tracePortals();
    }


    void CameraController::handleCamOverride(const std::chrono::microseconds& deltaTime)
    {
        Expects(m_camOverrideId >= 0 && gsl::narrow_cast<size_t>(m_camOverrideId) < m_level->m_cameras.size());
        Expects(m_level->m_cameras[m_camOverrideId].room < m_level->m_rooms.size());

        core::RoomBoundIntPosition pos(&m_level->m_rooms[m_level->m_cameras[m_camOverrideId].room]);
        pos.position.X = m_level->m_cameras[m_camOverrideId].x;
        pos.position.Y = m_level->m_cameras[m_camOverrideId].y;
        pos.position.Z = m_level->m_cameras[m_camOverrideId].z;

        if( !clampPosition(pos) )
            moveIntoGeometry(pos, loader::QuarterSectorSize);

        m_lookingAtSomething = true;
        updatePosition(pos, m_pivotMovementSmoothness, deltaTime);

        if( m_camOverrideTimeout > deltaTime )
            m_camOverrideTimeout -= deltaTime;
        else
            m_camOverrideTimeout = std::chrono::microseconds(-1);
    }


    int CameraController::moveIntoGeometry(core::RoomBoundIntPosition& pos, int margin) const
    {
        auto sector = m_level->findRealFloorSector(pos);
        Expects(sector->boxIndex < m_level->m_boxes.size());
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
        else if( top > pos.position.Y )
            return top - pos.position.Y;
        else
            return 0;
    }


    bool CameraController::isVerticallyOutsideRoom(const core::TRCoordinates& pos, const gsl::not_null<const loader::Room*>& room) const
    {
        gsl::not_null<const loader::Sector*> sector = m_level->findRealFloorSector(pos, room);
        const auto floor = HeightInfo::fromFloor(sector, pos, this).distance;
        const auto ceiling = HeightInfo::fromCeiling(sector, pos, this).distance;
        return pos.Y > floor || pos.Y <= ceiling;
    }


    void CameraController::updatePosition(const core::RoomBoundIntPosition& goalPosition, int smoothFactor, const std::chrono::microseconds& deltaTime)
    {
        m_currentPosition.position.X += (goalPosition.position.X - m_currentPosition.position.X) * core::toFloatFrame(deltaTime) / smoothFactor;
        m_currentPosition.position.Y += (goalPosition.position.Y - m_currentPosition.position.Y) * core::toFloatFrame(deltaTime) / smoothFactor;
        m_currentPosition.position.Z += (goalPosition.position.Z - m_currentPosition.position.Z) * core::toFloatFrame(deltaTime) / smoothFactor;
        HeightInfo::skipSteepSlants = false;
        m_currentPosition.room = goalPosition.room;
        auto sector = m_level->findRealFloorSector(m_currentPosition);
        auto floor = HeightInfo::fromFloor(sector, m_currentPosition.position, this).distance - loader::QuarterSectorSize;
        if( floor <= m_currentPosition.position.Y && floor <= goalPosition.position.Y )
        {
            clampPosition(m_currentPosition);
            sector = m_level->findRealFloorSector(m_currentPosition);
            floor = HeightInfo::fromFloor(sector, m_currentPosition.position, this).distance - loader::QuarterSectorSize;
        }

        auto ceiling = HeightInfo::fromCeiling(sector, m_currentPosition.position, this).distance + loader::QuarterSectorSize;
        if( floor < ceiling )
        {
            floor = ceiling = (floor + ceiling) / 2;
        }

        if( m_camShakeRadius != 0 )
        {
            if( m_camShakeRadius < 0 )
            {
                //! @todo cam shake
                m_camShakeRadius += 5;
            }
            else
            {
                m_camShakeRadius = 0;
            }
        }

        if( m_currentPosition.position.Y > floor )
            m_cameraYOffset = floor - m_currentPosition.position.Y;
        else if( m_currentPosition.position.Y < ceiling )
            m_cameraYOffset = ceiling - m_currentPosition.position.Y;
        else
            m_cameraYOffset = 0;

        auto camPos = m_currentPosition.position;
        camPos.Y += m_cameraYOffset;

        // update current room
        m_level->findRealFloorSector(camPos, &m_currentPosition.room);

        auto m = glm::lookAt(camPos.toRenderSystem(), m_pivot.position.toRenderSystem(), {0,1,0});
        m_camera->setViewMatrix(m);
    }


    void CameraController::doUsualMovement(const gsl::not_null<const items::ItemNode*>& item, const std::chrono::microseconds& deltaTimeMs)
    {
        m_globalRotation.X += item->getRotation().X;
        if( m_globalRotation.X > 85_deg )
            m_globalRotation.X = 85_deg;
        else if( m_globalRotation.X < -85_deg )
            m_globalRotation.X = -85_deg;

        auto dist = m_globalRotation.X.cos() * m_pivotDistance;
        m_flatPivotDistanceSq = gsl::narrow_cast<long>(dist * dist);

        core::RoomBoundIntPosition targetPos(m_currentPosition.room);
        targetPos.position.Y = std::lround(m_pivotDistance * m_globalRotation.X.sin() + m_pivot.position.Y);

        core::Angle y = m_globalRotation.Y + item->getRotation().Y;
        targetPos.position.X = std::lround(m_pivot.position.X - dist * y.sin());
        targetPos.position.Z = std::lround(m_pivot.position.Z - dist * y.cos());
        clampBox(targetPos, [this](long& a, long& b, long c, long d, long e, long f, long g, long h)
                 {
                     clampToCorners(m_flatPivotDistanceSq, a, b, c, d, e, f, g, h);
                 });

        updatePosition(targetPos, m_lookingAtSomething ? m_pivotMovementSmoothness : 12, deltaTimeMs);
    }


    void CameraController::handleFreeLook(const items::ItemNode& item, const std::chrono::microseconds& deltaTime)
    {
        const auto originalPivotPosition = m_pivot.position;
        m_pivot.position.X = std::lround(item.getPosition().X);
        m_pivot.position.Z = std::lround(item.getPosition().Z);
        m_globalRotation.X = m_torsoRotation.X + m_headRotation.X + item.getRotation().X;
        m_globalRotation.Y = m_torsoRotation.Y + m_headRotation.Y + item.getRotation().Y;
        m_pivotDistance = 1536;
        m_cameraYOffset = gsl::narrow_cast<int>(-2 * loader::QuarterSectorSize * m_globalRotation.Y.sin());
        m_pivot.position.X += std::lround(m_cameraYOffset * item.getRotation().Y.sin());
        m_pivot.position.Z += std::lround(m_cameraYOffset * item.getRotation().Y.cos());

        if( isVerticallyOutsideRoom(m_pivot.position, m_currentPosition.room) )
        {
            m_pivot.position.X = std::lround(item.getPosition().X);
            m_pivot.position.Z = std::lround(item.getPosition().Z);
        }

        m_pivot.position.Y += moveIntoGeometry(m_pivot, loader::QuarterSectorSize + 50);

        auto cameraPosition = m_pivot;
        cameraPosition.position.X -= std::lround(m_pivotDistance * m_globalRotation.Y.sin() * m_globalRotation.X.cos());
        cameraPosition.position.Z -= std::lround(m_pivotDistance * m_globalRotation.Y.cos() * m_globalRotation.X.cos());
        cameraPosition.position.Y += std::lround(m_pivotDistance * m_globalRotation.X.sin());
        cameraPosition.room = m_currentPosition.room;

        clampBox(cameraPosition, &freeLookClamp);

        m_pivot.position.X = originalPivotPosition.X + (m_pivot.position.X - originalPivotPosition.X) * core::toFloatFrame(deltaTime) / m_pivotMovementSmoothness;
        m_pivot.position.Z = originalPivotPosition.Z + (m_pivot.position.Z - originalPivotPosition.Z) * core::toFloatFrame(deltaTime) / m_pivotMovementSmoothness;

        updatePosition(cameraPosition, m_pivotMovementSmoothness, deltaTime);
    }


    void CameraController::handleEnemy(const items::ItemNode& item, const std::chrono::microseconds& deltaTime)
    {
        m_pivot.position.X = std::lround(item.getPosition().X);
        m_pivot.position.Z = std::lround(item.getPosition().Z);

        if( m_enemy != nullptr )
        {
            m_globalRotation.X = m_enemyLookRot.X + item.getRotation().X;
            m_globalRotation.Y = m_enemyLookRot.Y + item.getRotation().Y;
        }
        else
        {
            m_globalRotation.X = m_torsoRotation.X + m_headRotation.X + item.getRotation().X;
            m_globalRotation.Y = m_torsoRotation.Y + m_headRotation.Y + item.getRotation().Y;
        }

        m_pivotDistance = 2560;
        auto tmp = m_pivot;
        auto d = m_pivotDistance * m_globalRotation.X.cos();
        tmp.position.X -= std::lround(d * m_globalRotation.Y.sin());
        tmp.position.Z -= std::lround(d * m_globalRotation.Y.cos());
        tmp.position.Y += std::lround(m_pivotDistance * m_globalRotation.X.sin());
        tmp.room = m_currentPosition.room;

        clampBox(tmp, [this](long& a, long& b, long c, long d, long e, long f, long g, long h)
                 {
                     clampToCorners(m_flatPivotDistanceSq, a, b, c, d, e, f, g, h);
                 });
        updatePosition(tmp, m_pivotMovementSmoothness, deltaTime);
    }


    void CameraController::clampBox(core::RoomBoundIntPosition& goalPosition, const std::function<ClampCallback>& callback) const
    {
        clampPosition(goalPosition);
        BOOST_ASSERT(m_pivot.room->getSectorByAbsolutePosition(m_pivot.position)->boxIndex < m_level->m_boxes.size());
        auto clampBox = &m_level->m_boxes[m_pivot.room->getSectorByAbsolutePosition(m_pivot.position)->boxIndex];
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
            long left, right;
            if( goalPosition.position.X >= m_pivot.position.X )
            {
                left = clampXMin;
                right = clampXMax;
            }
            else
            {
                left = clampXMax;
                right = clampXMin;
            }
            callback(goalPosition.position.Z, goalPosition.position.X, m_pivot.position.Z, m_pivot.position.X, clampZMin, right, clampZMax, left);
            // BOOST_ASSERT(m_pivot.goalPosition.distanceTo(goalPosition.goalPosition) <= 2 * m_pivotDistance); // sanity check
        }
        else if( posZverticalOutside && goalPosition.position.Z > clampZMax )
        {
            skipRoomPatch = false;
            long left, right;
            if( goalPosition.position.X >= m_pivot.position.X )
            {
                left = clampXMin;
                right = clampXMax;
            }
            else
            {
                left = clampXMax;
                right = clampXMin;
            }
            callback(goalPosition.position.Z, goalPosition.position.X, m_pivot.position.Z, m_pivot.position.X, clampZMax, right, clampZMin, left);
            // BOOST_ASSERT(m_pivot.goalPosition.distanceTo(goalPosition.goalPosition) <= 2 * m_pivotDistance); // sanity check
        }

        if( !skipRoomPatch )
        {
            m_level->findRealFloorSector(goalPosition);
            return;
        }

        if( negXverticalOutside && goalPosition.position.X < clampXMin )
        {
            skipRoomPatch = false;
            long left, right;
            if( goalPosition.position.Z >= m_pivot.position.Z )
            {
                left = clampZMin;
                right = clampZMax;
            }
            else
            {
                left = clampZMax;
                right = clampZMin;
            }
            callback(goalPosition.position.X, goalPosition.position.Z, m_pivot.position.X, m_pivot.position.Z, clampXMin, right, clampXMax, left);
            // BOOST_ASSERT(m_pivot.goalPosition.distanceTo(goalPosition.goalPosition) <= 2 * m_pivotDistance); // sanity check
        }
        else if( posXverticalOutside && goalPosition.position.X > clampXMax )
        {
            skipRoomPatch = false;
            long left, right;
            if( goalPosition.position.Z >= m_pivot.position.Z )
            {
                left = clampZMin;
                right = clampZMax;
            }
            else
            {
                left = clampZMax;
                right = clampZMin;
            }
            callback(goalPosition.position.X, goalPosition.position.Z, m_pivot.position.X, m_pivot.position.Z, clampXMax, right, clampXMin, left);
            // BOOST_ASSERT(m_pivot.goalPosition.distanceTo(goalPosition.goalPosition) <= 2 * m_pivotDistance); // sanity check
        }

        if( !skipRoomPatch )
        {
            m_level->findRealFloorSector(goalPosition);
        }
    }


    void CameraController::freeLookClamp(long& currentFrontBack, long& currentLeftRight, long targetFrontBack, long targetLeftRight, long back, long right, long front, long left)
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


    void CameraController::clampToCorners(const long pivotDistanceSq, long& currentFrontBack, long& currentLeftRight, long targetFrontBack, long targetLeftRight, long back, long right, long front, long left)
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
                auto tmp = std::lround(std::sqrt(pivotDistanceSq - targetBackDistSq));
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
                auto tmp = std::lround(std::sqrt(pivotDistanceSq - targetBackDistSq));
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
                auto tmp = std::lround(std::sqrt(pivotDistanceSq - targetRightDistSq));
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
