#include "cameracontroller.h"

#include "animationcontroller.h"
#include "render/portaltracer.h"
#include "laracontroller.h"

#include <queue>
#include <set>
#include <chrono>


namespace engine
{
    CameraController::CameraController(gsl::not_null<level::Level*> level, gsl::not_null<LaraController*> laraController, const gsl::not_null<std::shared_ptr<gameplay::Camera>>& camera)
        : m_camera(camera)
        , m_level(level)
        , m_laraController(laraController)
        , m_currentYOffset(gsl::narrow_cast<int>(laraController->getPosition().Y - 1024))
        , m_currentLookAt(laraController->getCurrentRoom(), m_laraController->getPosition().toInexact())
        , m_currentPosition(laraController->getCurrentRoom())
    {
        m_currentLookAt.position.Y -= m_currentYOffset;
        m_currentPosition = m_currentLookAt;
        m_currentPosition.position.Z -= 100;

        update(std::chrono::microseconds(1));
    }

    void CameraController::animateNode(uint32_t timeMs)
    {
        if( m_firstUpdate )
        {
            m_lastAnimationTime = timeMs;
            m_firstUpdate = false;
        }

        if( m_firstInput )
        {
            m_firstInput = false;
        }

        const auto localTime = timeMs - m_lastAnimationTime;

        if( localTime <= 1 )
            return;

        m_lastAnimationTime = timeMs;

        m_localRotation.X = util::clamp(m_localRotation.X, -85_deg, +85_deg);

        tracePortals();
    }

    void CameraController::setLocalRotation(core::Angle x, core::Angle y)
    {
        setLocalRotationX(x);
        setLocalRotationY(y);
    }

    void CameraController::setLocalRotationX(core::Angle x)
    {
        m_localRotation.X = x;
    }

    void CameraController::setLocalRotationY(core::Angle y)
    {
        m_localRotation.Y = y;
    }

    void CameraController::setCamOverride(uint16_t floorData, uint16_t camId, loader::TriggerType triggerType, bool isDoppelganger, uint16_t triggerArg, bool switchIsOn)
    {
        Expects(camId < m_level->m_cameras.size());
        if( m_level->m_cameras[camId].isActive() )
            return;

        const auto timeout = floorData & 0xff;
        const auto flags = (floorData >> 8) & 0xff;

        m_camOverrideId = camId;
        if( m_camOverrideType == 2 || m_camOverrideType == 3 || triggerType == loader::TriggerType::Combat )
            return;

        if( triggerType == loader::TriggerType::Switch && triggerArg != 0 && switchIsOn )
            return;

        if( triggerType != loader::TriggerType::Switch && m_camOverrideId == m_activeCamOverrideId )
            return;

        if( timeout != 1 )
            m_camOverrideTimeout = std::chrono::seconds(timeout);

        if( (flags & 1) != 0 )
            m_level->m_cameras[camId].setActive(true);

        m_smoothFactor = 1 + ((flags & 0x3e00) >> 6);
        if( isDoppelganger )
            m_camOverrideType = 1;
        else
            m_camOverrideType = 5;
    }

    void CameraController::findCameraTarget(const loader::FloorData::value_type* floorData)
    {
        if( m_camOverrideType == 5 )
            return;

        int type = 2;
        while( true )
        {
            const bool isLast = loader::isLastFloordataEntry(*floorData);
            const auto triggerFunc = loader::extractTriggerFunction(*floorData);
            const auto param = loader::extractTriggerFunctionParam(*floorData);

            ++floorData;

            if( triggerFunc == loader::TriggerFunction::LookAt && m_camOverrideType != 2 && m_camOverrideType != 3 )
            {
                m_lookAtItem = m_level->getItemController(param);
            }
            else if( triggerFunc == loader::TriggerFunction::CameraTarget )
            {
                ++floorData;

                if( param != m_activeCamOverrideId )
                {
                    type = 0;
                }
                else
                {
                    m_camOverrideId = m_activeCamOverrideId;
                    if( m_camOverrideTimeout >= std::chrono::microseconds::zero() && m_camOverrideType != 2 && m_camOverrideType != 3 )
                    {
                        type = 1;
                        m_camOverrideType = 1;
                    }
                    else
                    {
                        type = 0;
                        m_camOverrideTimeout = std::chrono::microseconds(-1);
                    }
                }
            }

            if( isLast )
                break;
        }

        if( type == 0 || (type == 2 && m_lookAtItem != nullptr && m_lookAtItem->m_flags2_40 && m_lookAtItem != m_lookAtItem2) )
            m_lookAtItem = nullptr;
    }

    void CameraController::tracePortals()
    {
        for(const loader::Room& room : m_level->m_rooms)
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
        if(m_currentLookAt.position.X == origin.position.X)
        {
            return ClampType::None;
        }

        const auto d = origin.position - m_currentLookAt.position;

        const int sign = d.X < 0 ? -1 : 1;

        core::TRCoordinates testPos;
        testPos.X = (m_currentLookAt.position.X / loader::SectorSize) * loader::SectorSize;
        if( sign > 0 )
            testPos.X += loader::SectorSize - 1;

        testPos.Y = m_currentLookAt.position.Y + (testPos.X - m_currentLookAt.position.X) * d.Y / d.X;
        testPos.Z = m_currentLookAt.position.Z + (testPos.X - m_currentLookAt.position.X) * d.Z / d.X;

        core::TRCoordinates step;
        step.X = sign * loader::SectorSize;
        step.Y = step.X * d.Y / d.X;
        step.Z = step.X * d.Z / d.X;

        auto newRoom = m_currentLookAt.room;

        while( true )
        {
            if(sign > 0 && testPos.X >= origin.position.X)
            {
                origin.room = newRoom;
                return ClampType::None;
            }
            else if(sign < 0 && testPos.X <= origin.position.X)
            {
                origin.room = newRoom;
                return ClampType::None;
            }

            core::TRCoordinates heightPos = testPos;
            BOOST_ASSERT(heightPos.X % loader::SectorSize == 0 || heightPos.X % loader::SectorSize == loader::SectorSize - 1);
            auto sector = m_level->findFloorSectorWithClampedPosition(heightPos, &newRoom);
            if( testPos.Y > HeightInfo::fromFloor(sector, heightPos, this).distance || testPos.Y < HeightInfo::fromCeiling(sector, heightPos, this).distance )
            {
                origin.position = testPos;
                origin.room = newRoom;
                return ClampType::Vertical;
            }

            heightPos.X = testPos.X + sign;
            BOOST_ASSERT(heightPos.X % loader::SectorSize == 0 || heightPos.X % loader::SectorSize == loader::SectorSize - 1);
            const auto testRoom = newRoom;
            sector = m_level->findFloorSectorWithClampedPosition(heightPos, &newRoom);
            if( testPos.Y > HeightInfo::fromFloor(sector, heightPos, this).distance || testPos.Y < HeightInfo::fromCeiling(sector, heightPos, this).distance )
            {
                origin.position = testPos;
                origin.room = testRoom;
                return ClampType::Horizonal;
            }

            testPos += step;
        }
    }

    CameraController::ClampType CameraController::clampAlongZ(core::RoomBoundIntPosition& origin) const
    {
        if(m_currentLookAt.position.Z == origin.position.Z)
        {
            return ClampType::None;
        }

        const auto d = origin.position - m_currentLookAt.position;

        const int sign = d.Z < 0 ? -1 : 1;

        core::TRCoordinates testPos;
        testPos.Z = (m_currentLookAt.position.Z / loader::SectorSize) * loader::SectorSize;

        if( sign > 0 )
            testPos.Z += loader::SectorSize - 1;

        testPos.X = m_currentLookAt.position.X + (testPos.Z - m_currentLookAt.position.Z) * d.X / d.Z;
        testPos.Y = m_currentLookAt.position.Y + (testPos.Z - m_currentLookAt.position.Z) * d.Y / d.Z;

        core::TRCoordinates step;
        step.Z = sign * loader::SectorSize;
        step.X = step.Z * d.X / d.Z;
        step.Y = step.Z * d.Y / d.Z;

        auto newRoom = m_currentLookAt.room;

        while( true )
        {
            if(sign > 0 && testPos.Z >= origin.position.Z)
            {
                origin.room = newRoom;
                return ClampType::None;
            }
            else if(sign < 0 && testPos.Z <= origin.position.Z)
            {
                origin.room = newRoom;
                return ClampType::None;
            }

            core::TRCoordinates heightPos = testPos;
            BOOST_ASSERT(heightPos.Z % loader::SectorSize == 0 || heightPos.Z % loader::SectorSize == loader::SectorSize - 1);
            auto sector = m_level->findFloorSectorWithClampedPosition(heightPos, &newRoom);
            if( testPos.Y > HeightInfo::fromFloor(sector, heightPos, this).distance || testPos.Y < HeightInfo::fromCeiling(sector, heightPos, this).distance )
            {
                origin.position = testPos;
                origin.room = newRoom;
                return ClampType::Vertical;
            }

            heightPos.Z = testPos.Z + sign;
            BOOST_ASSERT(heightPos.Z % loader::SectorSize == 0 || heightPos.Z % loader::SectorSize == loader::SectorSize - 1);
            const auto testRoom = newRoom;
            sector = m_level->findFloorSectorWithClampedPosition(heightPos, &newRoom);
            if( testPos.Y > HeightInfo::fromFloor(sector, heightPos, this).distance || testPos.Y < HeightInfo::fromCeiling(sector, heightPos, this).distance )
            {
                origin.position = testPos;
                origin.room = testRoom;
                return ClampType::Horizonal;
            }

            testPos += step;
        }
    }

    bool CameraController::clampPosition(core::RoomBoundIntPosition& origin) const
    {
        BOOST_ASSERT(m_currentLookAt.room->isInnerPositionXZ(m_currentLookAt.position));

        bool firstUnclamped;
        ClampType secondClamp;
        if( std::abs(origin.position.Z - m_currentLookAt.position.Z) <= std::abs(origin.position.X - m_currentLookAt.position.X) )
        {
            firstUnclamped = clampAlongZ(origin) == ClampType::None;
            secondClamp = clampAlongX(origin);
        }
        else
        {
            firstUnclamped = clampAlongX(origin) == ClampType::None;
            secondClamp = clampAlongZ(origin);
        }

        if(secondClamp != ClampType::Horizonal)
        {
            return false;
        }

        auto sector = m_level->findFloorSectorWithClampedPosition(origin);
        return clampY(m_currentLookAt.position, origin.position, sector) && firstUnclamped && secondClamp == ClampType::None;
    }

    void CameraController::update(const std::chrono::microseconds& deltaTimeMs)
    {
        if(m_currentPosition.room->isWaterRoom())
        {
            if(m_level->m_cdStream != nullptr)
                m_level->m_cdStream->getSource().setDirectFilter(m_level->m_audioDev.getUnderwaterFilter());
            if(m_underwaterAmbience == nullptr)
            {
                m_underwaterAmbience = m_level->playSound(60, boost::none);
                m_underwaterAmbience->setLooping(true);
            }
        }
        else if(m_underwaterAmbience != nullptr)
        {
            if(m_level->m_cdStream != nullptr)
                m_level->m_cdStream->getSource().setDirectFilter(nullptr);
            m_level->stopSoundEffect(60);
            m_underwaterAmbience.reset();
        }

        if( m_camOverrideType == 4 )
        {
            //! @todo nextCinematicFrame();
            return;
        }

        if( m_unknown1 != 2 )
            HeightInfo::skipSteepSlants = true;

        const bool lookingAtSomething = m_lookAtItem != nullptr && (m_camOverrideType == 1 || m_camOverrideType == 5);

        ItemController* lookAtItem = lookingAtSomething ? m_lookAtItem : m_laraController;
        auto lookAtBbox = lookAtItem->getBoundingBox();
        int lookAtY = gsl::narrow_cast<int>(lookAtItem->getPosition().Y);
        if( lookingAtSomething )
            lookAtY += (lookAtBbox.min.y + lookAtBbox.max.y) / 2;
        else
            lookAtY += (lookAtBbox.min.y - lookAtBbox.max.y) * 3 / 4 + lookAtBbox.max.y;

        if( m_lookAtItem != nullptr && !lookingAtSomething )
        {
            BOOST_ASSERT(m_lookAtItem != lookAtItem);
            const auto distToLookAt = m_lookAtItem->getPosition().distanceTo(lookAtItem->getPosition());
            auto lookAtYAngle = -core::Angle::fromRad(std::atan2(m_lookAtItem->getPosition().X - lookAtItem->getPosition().X, m_lookAtItem->getPosition().Z - lookAtItem->getPosition().Z)) - lookAtItem->getRotation().Y;
            lookAtYAngle *= 0.5f;
            lookAtBbox = m_lookAtItem->getBoundingBox();
            auto lookAtXAngle = -core::Angle::fromRad(std::atan2(distToLookAt, lookAtY - (lookAtBbox.min.y + lookAtBbox.max.y) / 2 + m_lookAtItem->getPosition().Y));
            lookAtXAngle *= 0.5f;

            if( lookAtYAngle < 50_deg && lookAtYAngle > -50_deg && lookAtXAngle < 85_deg && lookAtXAngle > -85_deg )
            {
                lookAtYAngle -= m_headRotation.Y;
                if( lookAtYAngle > 4_deg )
                    m_headRotation.Y += core::makeInterpolatedValue(+4_deg).getScaled(deltaTimeMs);
                else if( lookAtYAngle < -4_deg )
                    m_headRotation.Y -= core::makeInterpolatedValue(+4_deg).getScaled(deltaTimeMs);
                else
                    m_headRotation.Y += core::makeInterpolatedValue(lookAtYAngle).getScaled(deltaTimeMs);
                m_torsoRotation.Y = m_headRotation.Y;

                lookAtXAngle -= m_headRotation.X;
                if( lookAtXAngle > 4_deg )
                    m_headRotation.X += core::makeInterpolatedValue(+4_deg).getScaled(deltaTimeMs);
                else if( lookAtXAngle < -4_deg )
                    m_headRotation.X -= core::makeInterpolatedValue(+4_deg).getScaled(deltaTimeMs);
                else
                    m_headRotation.X += core::makeInterpolatedValue(lookAtXAngle).getScaled(deltaTimeMs);
                m_torsoRotation.X = m_headRotation.X;

                m_camOverrideType = 2;
                m_lookAtItem->m_flags2_40 = true;
            }
        }

        m_currentLookAt.room = lookAtItem->getCurrentRoom();

        if( m_camOverrideType != 2 && m_camOverrideType != 3 )
        {
            m_currentLookAt.position.X = std::lround(lookAtItem->getPosition().X);
            m_currentLookAt.position.Z = std::lround(lookAtItem->getPosition().Z);

            if( m_unknown1 == 1 )
            {
                const auto midZ = (lookAtBbox.min.z + lookAtBbox.max.z) / 2;
                m_currentLookAt.position.Z += std::lround(midZ * lookAtItem->getRotation().Y.cos());
                m_currentLookAt.position.X += std::lround(midZ * lookAtItem->getRotation().Y.sin());
            }

            if( m_lookingAtSomething == lookingAtSomething )
            {
                //! @todo check formula
                m_currentLookAt.position.Y += (lookAtY - m_currentLookAt.position.Y) / 4;
                m_lookingAtSomething = false;
            }
            else
            {
                m_lookingAtSomething = true;
                m_currentLookAt.position.Y = lookAtY;
                m_smoothFactor = 1;
            }

            auto sector = m_level->findFloorSectorWithClampedPosition(m_currentLookAt);
            if( HeightInfo::fromFloor(sector, m_currentLookAt.position, this).distance < m_currentLookAt.position.Y )
                HeightInfo::skipSteepSlants = false;

            if( m_camOverrideType != 0 && m_unknown1 != 3 )
                handleCamOverride(deltaTimeMs);
            else
                doUsualMovement(lookAtItem, deltaTimeMs);
        }
        else
        {
            if( m_lookingAtSomething )
            {
                m_currentLookAt.position.Y = lookAtY - loader::QuarterSectorSize;
                m_smoothFactor = 1;
            }
            else
            {
                m_currentLookAt.position.Y += (lookAtY - loader::QuarterSectorSize - m_currentLookAt.position.Y) / 4;
                if( m_camOverrideType == 2 )
                    m_smoothFactor = 4;
                else
                    m_smoothFactor = 8;
            }
            m_lookingAtSomething = false;
            if( m_camOverrideType == 2 )
                handleFreeLook(*lookAtItem, deltaTimeMs);
            else
                handleEnemy(*lookAtItem, deltaTimeMs);
        }

        m_lookingAtSomething = lookingAtSomething;
        m_activeCamOverrideId = m_camOverrideId;
        if( m_camOverrideType != 5 || m_camOverrideTimeout < std::chrono::microseconds::zero() )
        {
            m_camOverrideType = 0;
            m_lookAtItem2 = m_lookAtItem;
            m_localRotation.X = m_localRotation.Y = 0_deg;
            m_distanceFromLookAt = 1536;
            m_camOverrideId = -1;
            m_lookAtItem = nullptr;
            m_unknown1 = 0;
        }
        HeightInfo::skipSteepSlants = false;
    }

    void CameraController::handleCamOverride(const std::chrono::microseconds& deltaTimeMs)
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
        updatePosition(pos, m_smoothFactor, deltaTimeMs);

        if( m_camOverrideTimeout > deltaTimeMs )
            m_camOverrideTimeout -= deltaTimeMs;
        else
            m_camOverrideTimeout = std::chrono::microseconds(-1);
    }

    int CameraController::moveIntoGeometry(core::RoomBoundIntPosition& pos, int margin) const
    {
        auto sector = m_level->findFloorSectorWithClampedPosition(pos);
        Expects(sector->boxIndex < m_level->m_boxes.size());
        const loader::Box& box = m_level->m_boxes[sector->boxIndex];

        if( box.zmin + margin > pos.position.Z && isVerticallyOutsideRoom(pos.position - core::TRCoordinates(0, 0, margin), pos.room) )
            pos.position.Z = box.zmin + margin;
        else if( box.zmax - margin > pos.position.Z && isVerticallyOutsideRoom(pos.position + core::TRCoordinates(0, 0, margin), pos.room) )
            pos.position.Z = box.zmax - margin;

        if( box.xmin + margin > pos.position.Z && isVerticallyOutsideRoom(pos.position - core::TRCoordinates(margin, 0, 0), pos.room) )
            pos.position.X = box.xmin + margin;
        else if( box.xmax - margin > pos.position.Z && isVerticallyOutsideRoom(pos.position + core::TRCoordinates(margin, 0, 0), pos.room) )
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
        gsl::not_null<const loader::Sector*> sector = m_level->findFloorSectorWithClampedPosition(pos, room);
        const auto floor = HeightInfo::fromFloor(sector, pos, this).distance;
        const auto ceiling = HeightInfo::fromCeiling(sector, pos, this).distance;
        return pos.Y > floor || pos.Y <= ceiling;
    }

    void CameraController::updatePosition(const core::RoomBoundIntPosition& position, int smoothFactor, const std::chrono::microseconds& deltaTimeMs)
    {
        m_currentPosition.position.X += (position.position.X - m_currentPosition.position.X) * deltaTimeMs / core::FrameTime / smoothFactor;
        m_currentPosition.position.Y += (position.position.Y - m_currentPosition.position.Y) * deltaTimeMs / core::FrameTime / smoothFactor;
        m_currentPosition.position.Z += (position.position.Z - m_currentPosition.position.Z) * deltaTimeMs / core::FrameTime / smoothFactor;
        HeightInfo::skipSteepSlants = false;
        m_currentPosition.room = position.room;
        auto sector = m_level->findFloorSectorWithClampedPosition(m_currentPosition);
        auto floor = HeightInfo::fromFloor(sector, m_currentPosition.position, this).distance - loader::QuarterSectorSize;
        if( floor <= m_currentPosition.position.Y && floor <= position.position.Y )
        {
            clampPosition(m_currentPosition);
            sector = m_level->findFloorSectorWithClampedPosition(m_currentPosition);
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
            m_currentYOffset = floor - m_currentPosition.position.Y;
        else if( m_currentPosition.position.Y < ceiling )
            m_currentYOffset = ceiling - m_currentPosition.position.Y;
        else
            m_currentYOffset = 0;

        auto camPos = m_currentPosition.position;
        camPos.Y += m_currentYOffset;

        // update current room
        m_level->findFloorSectorWithClampedPosition(camPos, &m_currentPosition.room);

        auto m = glm::lookAt(camPos.toRenderSystem(), m_currentLookAt.position.toRenderSystem(), { 0,1,0 });
        m_camera->setViewMatrix(m);
    }

    void CameraController::doUsualMovement(const gsl::not_null<const ItemController*>& item, const std::chrono::microseconds& deltaTimeMs)
    {
        m_localRotation.X += item->getRotation().X;
        if( m_localRotation.X > 85_deg )
            m_localRotation.X = 85_deg;
        else if( m_localRotation.X < -85_deg )
            m_localRotation.X = -85_deg;

        auto dist = m_localRotation.X.cos() * m_distanceFromLookAt;
        m_lookAtDistanceSq = gsl::narrow_cast<long>(dist * dist);

        core::RoomBoundIntPosition targetPos(m_currentPosition.room);
        targetPos.position.Y = std::lround(m_distanceFromLookAt * m_localRotation.X.sin() + m_currentLookAt.position.Y);

        core::Angle y = m_localRotation.Y + item->getRotation().Y;
        targetPos.position.X = std::lround(m_currentLookAt.position.X - dist * y.sin());
        targetPos.position.Z = std::lround(m_currentLookAt.position.Z - dist * y.cos());
        clampBox(targetPos, [this](long& a, long& b, long c, long d, long e, long f, long g, long h)
                 {
                     clampToCorners(m_lookAtDistanceSq, a, b, c, d, e, f, g, h);
                 });

        updatePosition(targetPos, m_lookingAtSomething ? m_smoothFactor : 12, deltaTimeMs);
    }

    void CameraController::handleFreeLook(const ItemController& item, const std::chrono::microseconds& deltaTimeMs)
    {
        const auto origLook = m_currentLookAt.position;
        m_currentLookAt.position.X = std::lround(item.getPosition().X);
        m_currentLookAt.position.Z = std::lround(item.getPosition().Z);
        m_localRotation.X = m_torsoRotation.X + m_headRotation.X + item.getRotation().X;
        m_localRotation.Y = m_torsoRotation.Y + m_headRotation.Y + item.getRotation().Y;
        m_distanceFromLookAt = 1536;
        m_currentYOffset = gsl::narrow_cast<int>(-2 * loader::QuarterSectorSize * m_localRotation.Y.sin());
        m_currentLookAt.position.X += std::lround(m_currentYOffset * item.getRotation().Y.sin());
        m_currentLookAt.position.Z += std::lround(m_currentYOffset * item.getRotation().Y.cos());

        if( isVerticallyOutsideRoom(m_currentLookAt.position, m_currentPosition.room) )
        {
            m_currentLookAt.position.X = std::lround(item.getPosition().X);
            m_currentLookAt.position.Z = std::lround(item.getPosition().Z);
        }

        m_currentLookAt.position.Y += moveIntoGeometry(m_currentLookAt, loader::QuarterSectorSize + 50);

        auto tmp = m_currentLookAt;
        tmp.position.X -= std::lround(m_distanceFromLookAt * m_localRotation.Y.sin() * m_localRotation.X.cos());
        tmp.position.Z -= std::lround(m_distanceFromLookAt * m_localRotation.Y.cos() * m_localRotation.X.cos());
        tmp.position.Y += std::lround(m_distanceFromLookAt * m_localRotation.X.sin());
        tmp.room = m_currentPosition.room;

        clampBox(tmp, &freeLookClamp);

        m_currentLookAt.position.X = origLook.X + (m_currentLookAt.position.X - origLook.X) * deltaTimeMs / m_smoothFactor / core::FrameTime;
        m_currentLookAt.position.Z = origLook.Z + (m_currentLookAt.position.Z - origLook.Z) * deltaTimeMs / m_smoothFactor / core::FrameTime;

        updatePosition(tmp, m_smoothFactor, deltaTimeMs);
    }

    void CameraController::handleEnemy(const ItemController& item, const std::chrono::microseconds& deltaTimeMs)
    {
        m_currentLookAt.position.X = std::lround(item.getPosition().X);
        m_currentLookAt.position.Z = std::lround(item.getPosition().Z);

        if( m_enemy != nullptr )
        {
            m_localRotation.X = m_enemyLookRot.X + item.getRotation().X;
            m_localRotation.Y = m_enemyLookRot.Y + item.getRotation().Y;
        }
        else
        {
            m_localRotation.X = m_torsoRotation.X + m_headRotation.X + item.getRotation().X;
            m_localRotation.Y = m_torsoRotation.Y + m_headRotation.Y + item.getRotation().Y;
        }

        m_distanceFromLookAt = 2560;
        auto tmp = m_currentLookAt;
        auto d = m_distanceFromLookAt * m_localRotation.X.cos();
        tmp.position.X -= std::lround(d * m_localRotation.Y.sin());
        tmp.position.Z -= std::lround(d * m_localRotation.Y.cos());
        tmp.position.Y += std::lround(m_distanceFromLookAt * m_localRotation.X.sin());
        tmp.room = m_currentPosition.room;

        clampBox(tmp, [this](long& a, long& b, long c, long d, long e, long f, long g, long h)
                 {
                     clampToCorners(m_lookAtDistanceSq, a, b, c, d, e, f, g, h);
                 });
        updatePosition(tmp, m_smoothFactor, deltaTimeMs);
    }

    void CameraController::clampBox(core::RoomBoundIntPosition& camTargetPos, const std::function<ClampCallback>& callback) const
    {
        clampPosition(camTargetPos);
        Expects(m_currentLookAt.room->getSectorByAbsolutePosition(m_currentLookAt.position)->boxIndex < m_level->m_boxes.size());
        auto clampBox = &m_level->m_boxes[m_currentLookAt.room->getSectorByAbsolutePosition(m_currentLookAt.position)->boxIndex];
        Expects(camTargetPos.room->getSectorByAbsolutePosition(camTargetPos.position) != nullptr);
        if( camTargetPos.room->getSectorByAbsolutePosition(camTargetPos.position)->boxIndex != 0xffff )
        {
            if(    camTargetPos.position.X < clampBox->xmin || camTargetPos.position.X > clampBox->xmax
                || camTargetPos.position.Z < clampBox->zmin || camTargetPos.position.Z > clampBox->zmax )
                clampBox = &m_level->m_boxes[camTargetPos.room->getSectorByAbsolutePosition(camTargetPos.position)->boxIndex];
        }

        core::TRCoordinates testPos = camTargetPos.position;
        testPos.Z = (testPos.Z / loader::SectorSize) * loader::SectorSize - 1;
        BOOST_ASSERT(testPos.Z % loader::SectorSize == loader::SectorSize - 1 && std::abs(testPos.Z - camTargetPos.position.Z) <= loader::SectorSize);

        auto clampZMin = clampBox->zmin;
        const bool negZverticalOutside = isVerticallyOutsideRoom(testPos, camTargetPos.room);
        if( !negZverticalOutside && m_level->findFloorSectorWithClampedPosition(testPos, camTargetPos.room)->boxIndex != 0xffff )
        {
            auto testBox = &m_level->m_boxes[m_level->findFloorSectorWithClampedPosition(testPos, camTargetPos.room)->boxIndex];
            if( testBox->zmin < clampZMin )
                clampZMin = testBox->zmin;
        }
        clampZMin += loader::QuarterSectorSize;

        testPos = camTargetPos.position;
        testPos.Z = (testPos.Z / loader::SectorSize + 1) * loader::SectorSize;
        BOOST_ASSERT(testPos.Z % loader::SectorSize == 0 && std::abs(testPos.Z - camTargetPos.position.Z) <= loader::SectorSize);

        auto clampZMax = clampBox->zmax;
        const bool posZverticalOutside = isVerticallyOutsideRoom(testPos, camTargetPos.room);
        if( !posZverticalOutside && m_level->findFloorSectorWithClampedPosition(testPos, camTargetPos.room)->boxIndex != 0xffff )
        {
            auto testBox = &m_level->m_boxes[m_level->findFloorSectorWithClampedPosition(testPos, camTargetPos.room)->boxIndex];
            if( testBox->zmax > clampZMax )
                clampZMax = testBox->zmax;
        }
        clampZMax -= loader::QuarterSectorSize;

        testPos = camTargetPos.position;
        testPos.X = (testPos.X / loader::SectorSize) * loader::SectorSize - 1;
        BOOST_ASSERT(testPos.X % loader::SectorSize == loader::SectorSize - 1 && std::abs(testPos.X - camTargetPos.position.X) <= loader::SectorSize);

        auto clampXMin = clampBox->xmin;
        const bool negXverticalOutside = isVerticallyOutsideRoom(testPos, camTargetPos.room);
        if( !negXverticalOutside && m_level->findFloorSectorWithClampedPosition(testPos, camTargetPos.room)->boxIndex != 0xffff )
        {
            auto testBox = &m_level->m_boxes[m_level->findFloorSectorWithClampedPosition(testPos, camTargetPos.room)->boxIndex];
            if( testBox->xmin < clampXMin )
                clampXMin = testBox->xmin;
        }
        clampXMin += loader::QuarterSectorSize;

        testPos = camTargetPos.position;
        testPos.X = (testPos.X / loader::SectorSize + 1) * loader::SectorSize;
        BOOST_ASSERT(testPos.X % loader::SectorSize == 0 && std::abs(testPos.X - camTargetPos.position.X) <= loader::SectorSize);

        auto clampXMax = clampBox->xmax;
        const bool posXverticalOutside = isVerticallyOutsideRoom(testPos, camTargetPos.room);
        if( !posXverticalOutside && m_level->findFloorSectorWithClampedPosition(testPos, camTargetPos.room)->boxIndex != 0xffff )
        {
            auto testBox = &m_level->m_boxes[m_level->findFloorSectorWithClampedPosition(testPos, camTargetPos.room)->boxIndex];
            if( testBox->xmax > clampXMax )
                clampXMax = testBox->xmax;
        }
        clampXMax -= loader::QuarterSectorSize;

        bool skipRoomPatch = true;
        if( negZverticalOutside && camTargetPos.position.Z < clampZMin )
        {
            skipRoomPatch = false;
            long left, right;
            if( camTargetPos.position.X >= m_currentLookAt.position.X )
            {
                left = clampXMin;
                right = clampXMax;
            }
            else
            {
                left = clampXMax;
                right = clampXMin;
            }
            callback(camTargetPos.position.Z, camTargetPos.position.X, m_currentLookAt.position.Z, m_currentLookAt.position.X, clampZMin, right, clampZMax, left);
            // BOOST_ASSERT(m_currentLookAt.position.distanceTo(camTargetPos.position) <= 2 * m_distanceFromLookAt); // sanity check
        }
        else if( posZverticalOutside && camTargetPos.position.Z > clampZMax )
        {
            skipRoomPatch = false;
            long left, right;
            if( camTargetPos.position.X >= m_currentLookAt.position.X )
            {
                left = clampXMin;
                right = clampXMax;
            }
            else
            {
                left = clampXMax;
                right = clampXMin;
            }
            callback(camTargetPos.position.Z, camTargetPos.position.X, m_currentLookAt.position.Z, m_currentLookAt.position.X, clampZMax, right, clampZMin, left);
            // BOOST_ASSERT(m_currentLookAt.position.distanceTo(camTargetPos.position) <= 2 * m_distanceFromLookAt); // sanity check
        }

        if( !skipRoomPatch )
        {
            m_level->findFloorSectorWithClampedPosition(camTargetPos);
            return;
        }

        if( negXverticalOutside && camTargetPos.position.X < clampXMin )
        {
            skipRoomPatch = false;
            long left, right;
            if( camTargetPos.position.Z >= m_currentLookAt.position.Z )
            {
                left = clampZMin;
                right = clampZMax;
            }
            else
            {
                left = clampZMax;
                right = clampZMin;
            }
            callback(camTargetPos.position.X, camTargetPos.position.Z, m_currentLookAt.position.X, m_currentLookAt.position.Z, clampXMin, right, clampXMax, left);
            // BOOST_ASSERT(m_currentLookAt.position.distanceTo(camTargetPos.position) <= 2 * m_distanceFromLookAt); // sanity check
        }
        else if( posXverticalOutside && camTargetPos.position.X > clampXMax )
        {
            skipRoomPatch = false;
            long left, right;
            if( camTargetPos.position.Z >= m_currentLookAt.position.Z )
            {
                left = clampZMin;
                right = clampZMax;
            }
            else
            {
                left = clampZMax;
                right = clampZMin;
            }
            callback(camTargetPos.position.X, camTargetPos.position.Z, m_currentLookAt.position.X, m_currentLookAt.position.Z, clampXMax, right, clampXMin, left);
            // BOOST_ASSERT(m_currentLookAt.position.distanceTo(camTargetPos.position) <= 2 * m_distanceFromLookAt); // sanity check
        }

        if( !skipRoomPatch )
        {
            m_level->findFloorSectorWithClampedPosition(camTargetPos);
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

    void CameraController::clampToCorners(const long lookAtDistanceSq, long& currentFrontBack, long& currentLeftRight, long targetFrontBack, long targetLeftRight, long back, long right, long front, long left)
    {
        const auto targetRightDistSq = (targetLeftRight - right) * (targetLeftRight - right);
        const auto targetBackDistSq = (targetFrontBack - back) * (targetFrontBack - back);

        // back right
        const auto backRightDistSq = targetBackDistSq + targetRightDistSq;
        if( backRightDistSq > lookAtDistanceSq )
        {
            //BOOST_LOG_TRIVIAL(debug) << "Clamp back right: " << currentFrontBack << " => " << back;
            //BOOST_LOG_TRIVIAL(debug) << "Left = " << left << ", right = " << right << ", front = " << front << ", back = " << back;
            currentFrontBack = back;
            if( lookAtDistanceSq >= targetBackDistSq )
            {
                auto tmp = std::lround(std::sqrt(lookAtDistanceSq - targetBackDistSq));
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
        if( targetBackLeftDistSq > lookAtDistanceSq )
        {
            //BOOST_LOG_TRIVIAL(debug) << "Clamp back left: " << currentFrontBack << " => " << back;
            //BOOST_LOG_TRIVIAL(debug) << "Left = " << left << ", right = " << right << ", front = " << front << ", back = " << back;
            currentFrontBack = back;
            if( lookAtDistanceSq >= targetBackDistSq )
            {
                auto tmp = std::lround(std::sqrt(lookAtDistanceSq - targetBackDistSq));
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

        if( targetFrontRightDistSq > lookAtDistanceSq )
        {
            //BOOST_LOG_TRIVIAL(debug) << "Clamp front right";
            //BOOST_LOG_TRIVIAL(debug) << "Left = " << left << ", right = " << right << ", front = " << front << ", back = " << back;
            if( lookAtDistanceSq >= targetRightDistSq )
            {
                currentLeftRight = right;
                auto tmp = std::lround(std::sqrt(lookAtDistanceSq - targetRightDistSq));
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
