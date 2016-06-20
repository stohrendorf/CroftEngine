#include "cameracontroller.h"

#include "animationcontroller.h"
#include "render/portaltracer.h"
#include "laracontroller.h"

#include <queue>
#include <set>

CameraController::CameraController(gsl::not_null<irr::gui::ICursorControl*> cursorControl, gsl::not_null<const loader::Level*> level, gsl::not_null<LaraController*> laraController, gsl::not_null<irr::video::IVideoDriver*> drv, gsl::not_null<const loader::Room*> currentRoom, const gsl::not_null<irr::scene::ICameraSceneNode*>& camera)
    : ISceneNodeAnimator()
    , m_cursorControl(cursorControl)
    , m_level(level)
    , m_laraController(laraController)
    , m_driver(drv)
    , m_currentPosition(currentRoom)
    , m_currentLookAt(currentRoom)
    , m_camera(camera)
{
    m_currentLookAt.position = m_laraController->getPosition();
    m_currentLookAt.position.Y -= m_lookAtY;
    m_currentPosition = m_currentLookAt;
    m_currentPosition.position.Z -= 100;
    m_lookAtY = m_laraController->getPosition().Y - 1024;

    update(1000 / core::FrameRate);
}

void CameraController::animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs)
{
    Expects(node == m_camera);
    
    irr::scene::ISceneManager* smgr = node->getSceneManager();
    if(smgr && smgr->getActiveCamera() != m_camera)
        return;
    
    m_inputState.setXAxisMovement(m_left, m_right);
    m_inputState.setZAxisMovement(m_backward, m_forward);
    m_inputState.setStepMovement(m_stepLeft, m_stepRight);
    m_laraController->setInputState(m_inputState);
    
    if(m_firstUpdate)
    {
        m_cursorControl->setPosition(0.5f, 0.5f);
        m_currentCursorPos = m_prevCursorPos = m_cursorControl->getRelativePosition();
        
        m_lastAnimationTime = timeMs;
        
        m_firstUpdate = false;
    }
    
    // If the camera isn't the active camera, and receiving input, then don't process it.
    if(!m_camera->isInputReceiverEnabled())
    {
        m_firstInput = true;
        return;
    }
    
    if(m_firstInput)
    {
        m_left = m_right = m_stepLeft = m_stepRight = m_forward = m_backward = false;
        m_firstInput = false;
    }
    
    const auto localTime = timeMs - m_lastAnimationTime;

    if(localTime <= 1)
        return;

    m_lastAnimationTime = timeMs;
    
    // Update mouse rotation
    if(m_currentCursorPos != m_prevCursorPos)
    {
        static const core::Angle rotationSpeed = 100_deg;
        m_localRotation.Y -= rotationSpeed * (0.5f - m_currentCursorPos.X);
        m_localRotation.X -= rotationSpeed * (0.5f - m_currentCursorPos.Y);
        m_localRotation.X = irr::core::clamp(m_localRotation.X, -85_deg, +85_deg);
        
        // Do the fix as normal, special case below
        // reset cursor position to the centre of the window.
        m_cursorControl->setPosition(0.5f, 0.5f);
        
        // needed to avoid problems when the event receiver is disabled
        m_currentCursorPos = m_prevCursorPos = m_cursorControl->getRelativePosition();
    }
    
    // Special case, mouse is whipped outside of window before it can update.
    irr::video::IVideoDriver* driver = smgr->getVideoDriver();
    irr::core::vector2d<irr::u32> mousepos(m_cursorControl->getPosition().X, m_cursorControl->getPosition().Y);
    irr::core::rect<irr::u32> screenRect(0, 0, driver->getScreenSize().Width, driver->getScreenSize().Height);
    
    // Only if we are moving outside quickly.
    if(!screenRect.isPointInside(mousepos))
    {
        // Force a reset.
        m_cursorControl->setPosition(0.5f, 0.5f);
        m_currentCursorPos = m_prevCursorPos = m_cursorControl->getRelativePosition();
    }
    
    update(localTime);

    tracePortals();
}

irr::scene::ISceneNodeAnimator* CameraController::createClone(irr::scene::ISceneNode*, irr::scene::ISceneManager*)
{
    BOOST_ASSERT(false);
    return nullptr;
}

bool CameraController::OnEvent(const irr::SEvent& evt)
{
    switch(evt.EventType)
    {
        case irr::EET_KEY_INPUT_EVENT:
            switch(evt.KeyInput.Key)
            {
                case irr::KEY_KEY_A:
                    m_left = evt.KeyInput.PressedDown;
                    return true;
                case irr::KEY_KEY_D:
                    m_right = evt.KeyInput.PressedDown;
                    return true;
                case irr::KEY_KEY_Q:
                    m_stepLeft = evt.KeyInput.PressedDown;
                    return true;
                case irr::KEY_KEY_E:
                    m_stepRight = evt.KeyInput.PressedDown;
                    return true;
                case irr::KEY_KEY_W:
                    m_forward = evt.KeyInput.PressedDown;
                    return true;
                case irr::KEY_KEY_S:
                    m_backward = evt.KeyInput.PressedDown;
                    return true;
                case irr::KEY_LSHIFT:
                case irr::KEY_RSHIFT:
                case irr::KEY_SHIFT:
                    m_inputState.moveSlow = evt.KeyInput.PressedDown;
                    return true;
                case irr::KEY_LCONTROL:
                case irr::KEY_RCONTROL:
                case irr::KEY_CONTROL:
                    m_inputState.action = evt.KeyInput.PressedDown;
                    return true;
                case irr::KEY_SPACE:
                    m_inputState.jump = evt.KeyInput.PressedDown;
                    return true;
                case irr::KEY_KEY_X:
                    m_inputState.roll = evt.KeyInput.PressedDown;
                    break;
                default:
                    return false;
            }
            break;
            
        case irr::EET_MOUSE_INPUT_EVENT:
            switch(evt.MouseInput.Event)
            {
                case irr::EMIE_MOUSE_MOVED:
                    m_currentCursorPos = m_cursorControl->getRelativePosition();
                    return true;
                case irr::EMIE_LMOUSE_PRESSED_DOWN:
                    m_inputState.action = true;
                    return true;
                case irr::EMIE_LMOUSE_LEFT_UP:
                    m_inputState.action = false;
                    return true;
                default:
                    return false;
            }
            
        default:
            break;
    }
    
    return false;
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

void CameraController::tracePortals()
{
    bool cameraOutOfGeometry = true;
    for(size_t i = 0; i < m_level->m_rooms.size(); ++i)
    {
        const loader::Room& room = m_level->m_rooms[i];
        if(room.node->getTransformedBoundingBox().isPointInside(m_camera->getAbsolutePosition()))
        {
            cameraOutOfGeometry = false;
            break;
        }
    }
    
    for(const loader::Room& room : m_level->m_rooms)
        room.node->setVisible(cameraOutOfGeometry);

    if(cameraOutOfGeometry)
    {
        return;
    }
    
    // First, find the room the camera is actually in.

    const loader::Room* startRoom = m_laraController->getCurrentRoom();
    {
        std::queue<const loader::Room*> toVisit;
        std::set<const loader::Room*> visited;
        toVisit.push(m_laraController->getCurrentRoom());
        while(!toVisit.empty())
        {
            auto currentRoom = toVisit.front();
            toVisit.pop();
            if(!visited.insert(currentRoom).second)
                continue;

            if(currentRoom->node->getTransformedBoundingBox().isPointInside(m_camera->getAbsolutePosition()))
            {
                startRoom = currentRoom;
                break;
            }
            for(const loader::Portal& portal : currentRoom->portals)
            {
                BOOST_ASSERT(portal.adjoining_room < m_level->m_rooms.size());
                toVisit.push( &m_level->m_rooms[portal.adjoining_room] );
            }
        }
    }

    startRoom->node->setVisible(true);

    // Breadth-first queue
    std::queue<render::PortalTracer> toVisit;

    // always process direct neighbours of the starting room
    for(const loader::Portal& portal : startRoom->portals)
    {
        render::PortalTracer path;
        if(!path.checkVisibility(&portal, *m_camera, m_driver))
            continue;
        
        m_level->m_rooms[portal.adjoining_room].node->setVisible(true);
        
        toVisit.emplace(std::move(path));
    }
    
    // Avoid infinite loops
    std::set<const loader::Portal*> visited;
    while(!toVisit.empty())
    {
        const render::PortalTracer currentPath = std::move(toVisit.front());
        toVisit.pop();
        
        if(!visited.insert(currentPath.getLastPortal()).second)
        {
            continue; // already tested
        }
        
        // iterate through the last room's portals and add the destinations if suitable
        uint16_t destRoom = currentPath.getLastDestinationRoom();
        for(const loader::Portal& srcPortal : m_level->m_rooms[destRoom].portals)
        {
            render::PortalTracer newPath = currentPath;
            if(!newPath.checkVisibility(&srcPortal, *m_camera, m_driver))
                continue;
            
            m_level->m_rooms[srcPortal.adjoining_room].node->setVisible(true);
            toVisit.emplace(std::move(newPath));
        }
    }
}

bool CameraController::clampY(const loader::ExactTRCoordinates& lookAt, loader::ExactTRCoordinates& origin, gsl::not_null<const loader::Sector*> sector) const
{
    const auto d = origin - lookAt;
    const HeightInfo floor = HeightInfo::fromFloor(sector, origin.toInexact(), this);
    if(floor.distance < origin.Y && floor.distance > lookAt.Y)
    {
        origin.Y = floor.distance;
        origin.X = d.X / d.Y * (floor.distance - lookAt.Y) + lookAt.X;
        origin.Z = d.Z / d.Y * (floor.distance - lookAt.Y) + lookAt.Z;
        return false;
    }

    const HeightInfo ceiling = HeightInfo::fromCeiling(sector, origin.toInexact(), this);
    if(ceiling.distance > origin.Y && ceiling.distance < lookAt.Y)
    {
        origin.Y = ceiling.distance;
        origin.X = d.X / d.Y * (ceiling.distance - lookAt.Y) + lookAt.X;
        origin.Z = d.Z / d.Y * (ceiling.distance - lookAt.Y) + lookAt.Z;
        return false;
    }

    return true;
}

CameraController::ClampType CameraController::clampX(const loader::ExactTRCoordinates& lookAt, loader::ExactTRCoordinates& origin) const
{
    if(irr::core::equals(lookAt.X, origin.X, 1.0f))
        return ClampType::None;

    const auto d = origin - lookAt;
    const auto gradientZX = d.Z / d.X;
    const auto gradientYX = d.Y / d.X;

    const int sign = d.X < 0 ? -1 : 1;

    loader::ExactTRCoordinates testPos;
    testPos.X = std::floor(lookAt.X / loader::SectorSize) * loader::SectorSize;
    if(sign > 0)
        testPos.X += loader::SectorSize - 1;

    testPos.Y = lookAt.Y + (testPos.X - lookAt.X) * gradientYX;
    testPos.Z = lookAt.Z + (testPos.X - lookAt.X) * gradientZX;

    loader::ExactTRCoordinates step;
    step.X = sign * loader::SectorSize;
    step.Y = gradientYX * step.X;
    step.Z = gradientZX * step.X;

    while(true)
    {
        if(sign > 0 && testPos.X >= origin.X)
            return ClampType::None;
        else if(sign < 0 && testPos.X <= origin.X)
            return ClampType::None;

        loader::TRCoordinates heightPos = testPos.toInexact();
        auto sector = m_level->findSectorForPosition(heightPos, m_laraController->getCurrentRoom());
        HeightInfo floor = HeightInfo::fromFloor(sector, heightPos, this);
        HeightInfo ceiling = HeightInfo::fromCeiling(sector, heightPos, this);
        if(testPos.Y > floor.distance || testPos.Y < ceiling.distance)
        {
            origin = testPos;
            return ClampType::Normal;
        }

        heightPos.X = testPos.X + 2*sign;
        sector = m_level->findSectorForPosition(heightPos, m_laraController->getCurrentRoom());
        floor = HeightInfo::fromFloor(sector, heightPos, this);
        ceiling = HeightInfo::fromCeiling(sector, heightPos, this);
        if(testPos.Y > floor.distance || testPos.Y < ceiling.distance)
        {
            origin = testPos;
            return ClampType::Edge;
        }

        testPos += step;
    }
}

CameraController::ClampType CameraController::clampZ(const loader::ExactTRCoordinates& lookAt, loader::ExactTRCoordinates& origin) const
{
    if(irr::core::equals(lookAt.Z, origin.Z, 1.0f))
        return ClampType::None;

    const auto d = origin - lookAt;
    const auto gradientXZ = d.X / d.Z;
    const auto gradientYZ = d.Y / d.Z;

    const int sign = d.Z < 0 ? -1 : 1;

    loader::ExactTRCoordinates testPos;
    testPos.Z = std::floor(lookAt.Z / loader::SectorSize) * loader::SectorSize;

    if(sign > 0)
        testPos.Z += loader::SectorSize - 1;

    testPos.X = lookAt.X + (testPos.Z - lookAt.Z) * gradientXZ;
    testPos.Y = lookAt.Y + (testPos.Z - lookAt.Z) * gradientYZ;

    loader::ExactTRCoordinates step;
    step.Z = sign * loader::SectorSize;
    step.X = gradientXZ * step.Z;
    step.Y = gradientYZ * step.Z;

    gsl::not_null<const loader::Room*> room = m_laraController->getCurrentRoom();
    while(true)
    {
        if(sign > 0 && testPos.Z >= origin.Z)
            return ClampType::None;
        else if(sign < 0 && testPos.Z <= origin.Z)
            return ClampType::None;

        loader::TRCoordinates heightPos = testPos.toInexact();
        auto sector = m_level->findSectorForPosition(heightPos, &room);
        HeightInfo floor = HeightInfo::fromFloor(sector, heightPos, this);
        HeightInfo ceiling = HeightInfo::fromCeiling(sector, heightPos, this);
        if(testPos.Y > floor.distance || testPos.Y < ceiling.distance)
        {
            origin = testPos;
            return ClampType::Normal;
        }

        heightPos.Z = testPos.Z + 2*sign;
        sector = m_level->findSectorForPosition(heightPos, &room);
        floor = HeightInfo::fromFloor(sector, heightPos, this);
        ceiling = HeightInfo::fromCeiling(sector, heightPos, this);
        if(testPos.Y > floor.distance || testPos.Y < ceiling.distance)
        {
            origin = testPos;
            return ClampType::Edge;
        }

        testPos += step;
    }
}

bool CameraController::clamp(const loader::ExactTRCoordinates& lookAt, loader::ExactTRCoordinates& origin) const
{
    bool firstUnclamped;
    ClampType secondClamp;
    if(std::abs(origin.Z - lookAt.Z) <= std::abs(origin.X - lookAt.X))
    {
        firstUnclamped = clampZ(lookAt, origin) == ClampType::None;
        secondClamp = clampX(lookAt, origin);
    }
    else
    {
        firstUnclamped = clampX(lookAt, origin) == ClampType::None;
        secondClamp = clampZ(lookAt, origin);
    }

    if(secondClamp == ClampType::Edge)
        return false;

    auto sector = m_level->findSectorForPosition(origin.toInexact(), m_laraController->getCurrentRoom());
    return clampY(lookAt, origin, sector) && firstUnclamped && secondClamp == ClampType::None;
}

void CameraController::update(int deltaTimeMs)
{
    if(m_camOverrideType == 4)
    {
        //! @todo nextCinematicFrame();
        return;
    }

    if(m_unknown1 != 2)
        HeightInfo::skipSteepSlants = true;

    bool lookingAtSomething = m_lookAtItem != nullptr && (m_camOverrideType == 1 || m_camOverrideType == 5);

    ItemController* lookAtItem = m_lookAtItem != nullptr ? m_lookAtItem : m_laraController;
    auto lookAtItemBbox = lookAtItem->getBoundingBox();
    auto lookAtBbox = lookAtItemBbox;
    int lookAtY = lookAtItem->getPosition().Y;
    if(lookingAtSomething)
        lookAtY += (lookAtItemBbox.MinEdge.Y + lookAtItemBbox.MaxEdge.Y) / 2;
    else
        lookAtY += (lookAtItemBbox.MinEdge.Y - lookAtItemBbox.MaxEdge.Y) * 3 / 4 + lookAtItemBbox.MaxEdge.Y;

    if(m_lookAtItem != nullptr && !lookingAtSomething)
    {
        const auto dist = m_lookAtItem->getPosition().distanceTo(lookAtItem->getPosition());
        auto y = core::Angle::fromRad(std::atan2(m_lookAtItem->getPosition().X - lookAtItem->getPosition().X, m_lookAtItem->getPosition().Z - lookAtItem->getPosition().Z)) - lookAtItem->getRotation().Y;
        y *= 0.5;
        lookAtBbox = m_lookAtItem->getBoundingBox();
        auto x = core::Angle::fromRad(std::atan2(dist, lookAtY - (lookAtBbox.MinEdge.Y + lookAtBbox.MaxEdge.Y) / 2 + m_lookAtItem->getPosition().Y));
        x *= 0.5;

        if(y < 50_deg && y > -50_deg && x < 85_deg && x > -85_deg)
        {
            y -= m_headRotation.Y;
            if(y > 4_deg)
                m_headRotation.Y += 4_deg;
            else if(y >= -4_deg)
                m_headRotation.Y += y;
            else
                m_headRotation.Y -= 4_deg;
            m_freeLookRotation.Y = m_headRotation.Y;

            x -= m_headRotation.X;
            if(x > 4_deg)
                m_headRotation.X += 4_deg;
            else if(x >= -4_deg)
                m_headRotation.X += x;
            else
                m_headRotation.X -= 4_deg;
            m_freeLookRotation.X = m_headRotation.X;

            m_camOverrideType = 2;
            //! @todo set item flag 0x40
        }
    }

    if(m_camOverrideType != 2 && m_camOverrideType != 3)
    {
        m_currentLookAt.position.X = lookAtItem->getPosition().X;
        m_currentLookAt.position.Z = lookAtItem->getPosition().Z;

        if(m_unknown1 == 1)
        {
            auto midZ = (lookAtItemBbox.MinEdge.Z + lookAtItemBbox.MaxEdge.Z) / 2;
            m_currentLookAt.position.Z += midZ * lookAtItem->getRotation().Y.cos();
            m_currentLookAt.position.X += midZ * lookAtItem->getRotation().Y.sin();
        }

        m_currentLookAt.room = lookAtItem->getCurrentRoom();
        if(m_lookingAtSomething == lookingAtSomething)
        {
            //! @todo check formula
            m_currentLookAt.position.Y += (lookAtY - m_currentLookAt.position.Y) * 4 / 5;
            m_lookingAtSomething = false;
        }
        else
        {
            m_lookingAtSomething = true;
            m_currentLookAt.position.Y = lookAtY;
            m_smoothFactor = 1;
        }

        auto sector = m_level->findSectorForPosition(m_currentLookAt.position.toInexact(), &m_currentLookAt.room);
        if(HeightInfo::fromFloor(sector, m_currentLookAt.position.toInexact(), this).distance < m_currentLookAt.position.Y)
            HeightInfo::skipSteepSlants = false;

        if(m_camOverrideType != 0 && m_unknown1 != 3)
            handleCamOverride(deltaTimeMs);
        else
            doUsualMovement(lookAtItem, deltaTimeMs);
    }
    else
    {
        m_currentLookAt.room = lookAtItem->getCurrentRoom();
        if(m_lookingAtSomething)
        {
            m_currentLookAt.position.Y = lookAtY - loader::QuarterSectorSize;
            m_smoothFactor = 1;
        }
        else
        {
            m_currentLookAt.position.Y += (lookAtY - loader::QuarterSectorSize - m_currentLookAt.position.Y) / 4;
            if(m_camOverrideType == 2)
                m_smoothFactor = 4;
            else
                m_smoothFactor = 8;
        }
        m_lookingAtSomething = false;
        if(m_camOverrideType == 2)
            handleFreeLook(*lookAtItem, deltaTimeMs);
        else
            handleEnemy(*lookAtItem, deltaTimeMs);
    }

    m_lookingAtSomething = lookingAtSomething;
    m_activeCamOverrideId = m_camOverrideId;
    if(m_camOverrideType != 5 || m_camOverrideTimeout == -1)
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

void CameraController::handleCamOverride(int deltaTimeMs)
{
    Expects(m_camOverrideId >= 0 && m_camOverrideId < m_level->m_cameras.size());
    Expects(m_level->m_cameras[m_camOverrideId].room < m_level->m_rooms.size());

    loader::RoomBoundPosition pos(&m_level->m_rooms[m_level->m_cameras[m_camOverrideId].room]);
    pos.position.X = m_level->m_cameras[m_camOverrideId].x;
    pos.position.Y = m_level->m_cameras[m_camOverrideId].y;
    pos.position.Z = m_level->m_cameras[m_camOverrideId].z;

    if(!clamp(m_currentLookAt.position, pos.position))
        moveIntoGeometry(pos, loader::QuarterSectorSize);

    m_lookingAtSomething = true;
    updatePosition(pos, m_smoothFactor, deltaTimeMs);
    if(m_camOverrideTimeout == 0)
        return;

    --m_camOverrideTimeout;
    if(m_camOverrideTimeout == 0)
        m_camOverrideTimeout = -1;
}

int CameraController::moveIntoGeometry(loader::RoomBoundPosition& pos, int margin) const
{
    auto sector = m_level->findSectorForPosition(pos.position.toInexact(), &pos.room);
    Expects(sector->boxIndex < m_level->m_boxes.size());
    const loader::Box& box = m_level->m_boxes[sector->boxIndex];
    
    if(box.zmin + margin > pos.position.Z && isOutsideRoom(pos.position.toInexact() - loader::TRCoordinates(0, 0, margin), pos.room))
        pos.position.Z = box.zmin + margin;
    else if(box.zmax - margin > pos.position.Z && isOutsideRoom(pos.position.toInexact() + loader::TRCoordinates(0, 0, margin), pos.room))
        pos.position.Z = box.zmax - margin;
    
    if(box.xmin + margin > pos.position.Z && isOutsideRoom(pos.position.toInexact() - loader::TRCoordinates(margin, 0, 0), pos.room))
        pos.position.X = box.xmin + margin;
    else if(box.xmax - margin > pos.position.Z && isOutsideRoom(pos.position.toInexact() + loader::TRCoordinates(margin, 0, 0), pos.room))
        pos.position.X = box.xmax - margin;

    auto ymax = HeightInfo::fromFloor(sector, pos.position.toInexact(), this).distance - margin;
    auto ymin = HeightInfo::fromCeiling(sector, pos.position.toInexact(), this).distance + margin;
    if(ymax > ymin)
        ymin = ymax = (ymax + ymin) / 2;

    if(pos.position.Y > ymax)
        return ymax - pos.position.Y;
    else if(ymin > pos.position.Y)
        return ymin - pos.position.Y;
    else
        return 0;
}

bool CameraController::isOutsideRoom(const loader::TRCoordinates& pos, const gsl::not_null<const loader::Room*>& room) const
{
    auto sector = m_level->findSectorForPosition(pos, room);
    return pos.Y >= HeightInfo::fromFloor(sector, pos, this).distance || pos.Y <= HeightInfo::fromCeiling(sector, pos, this).distance;
}

void CameraController::updatePosition(const loader::RoomBoundPosition& pos, int smoothFactor, int deltaTimeMs)
{
    m_currentPosition.position += (pos.position - m_currentPosition.position) * core::FrameRate * deltaTimeMs / 1000 / smoothFactor;
    HeightInfo::skipSteepSlants = false;
    m_currentPosition.room = pos.room;
    auto sector = m_level->findSectorForPosition(m_currentPosition.position.toInexact(), &m_currentPosition.room);
    auto height = HeightInfo::fromFloor(sector, m_currentPosition.position.toInexact(), this).distance - loader::QuarterSectorSize;
    if(height <= m_currentPosition.position.Y && height <= pos.position.Y)
    {
        clamp(m_currentLookAt.position, m_currentPosition.position);
        sector = m_level->findSectorForPosition(m_currentPosition.position.toInexact(), &m_currentPosition.room);
        height = HeightInfo::fromFloor(sector, m_currentPosition.position.toInexact(), this).distance - loader::QuarterSectorSize;
    }

    auto ceiling = HeightInfo::fromCeiling(sector, m_currentPosition.position.toInexact(), this).distance + loader::QuarterSectorSize;
    if(height < ceiling)
    {
        height = ceiling = (height + ceiling) / 2;
    }

    if(m_camShakeRadius != 0)
    {
        if(m_camShakeRadius < 0)
        {
            //! @todo cam shake
            m_camShakeRadius += 5;
        }
        else
        {
            m_camShakeRadius = 0;
        }
    }

    if(height >= m_currentPosition.position.Y)
    {
        if(ceiling <= m_currentPosition.position.Y)
            m_lookAtY = 0;
        else
            m_lookAtY = ceiling - m_currentPosition.position.Y;
    }
    else
    {
        m_lookAtY = height - m_currentPosition.position.Y;
    }

    auto camPos = m_currentPosition.position;
    camPos.Y += m_lookAtY;

    m_level->findSectorForPosition(camPos.toInexact(), &m_currentPosition.room);

    m_camera->setPosition(camPos.toIrrlicht());
    m_camera->updateAbsolutePosition();
    m_camera->setTarget(m_currentLookAt.position.toIrrlicht());
    m_camera->updateAbsolutePosition();
}

void CameraController::doUsualMovement(const gsl::not_null<ItemController*>& item, int deltaTimeMs)
{
    m_localRotation.X += item->getRotation().X;
    if(m_localRotation.X > 85_deg)
        m_localRotation.X = 85_deg;
    else if(m_localRotation.X < -85_deg)
        m_localRotation.X = -85_deg;

    auto dist = m_localRotation.X.cos() * m_distanceFromLookAt;
    m_horizontalDistSq = dist*dist;

    loader::RoomBoundPosition targetPos(m_currentPosition.room);
    targetPos.position.Y = m_distanceFromLookAt * m_localRotation.X.sin() + m_currentLookAt.position.Y;

    core::Angle y = m_localRotation.Y + item->getRotation().Y;
    targetPos.position.X = m_currentLookAt.position.X - dist*y.sin();
    targetPos.position.Z = m_currentLookAt.position.Z - dist*y.cos();
    //! @todo clampBox(targetPos, &sub_22918);

    if(m_lookingAtSomething)
        updatePosition(targetPos, m_smoothFactor, deltaTimeMs);
    else
        updatePosition(targetPos, 12, deltaTimeMs);
}

void CameraController::handleFreeLook(const ItemController& item, int deltaTimeMs)
{
    const auto origLook = m_currentLookAt.position;
    m_currentLookAt.position.X = item.getPosition().X;
    m_currentLookAt.position.Z = item.getPosition().Z;
    m_localRotation.X = m_freeLookRotation.X + m_headRotation.X + item.getRotation().X;
    m_localRotation.Y = m_freeLookRotation.Y + m_headRotation.Y + item.getRotation().Y;
    m_distanceFromLookAt = 1536;
    m_lookAtY = -2 * loader::QuarterSectorSize * m_localRotation.Y.sin();
    m_currentLookAt.position.X += m_lookAtY * item.getRotation().Y.sin();
    m_currentLookAt.position.Z += m_lookAtY * item.getRotation().Y.cos();

    if(isOutsideRoom(m_currentLookAt.position.toInexact(), m_currentPosition.room))
    {
        m_currentLookAt.position.X = item.getPosition().X;
        m_currentLookAt.position.Z = item.getPosition().Z;
    }

    m_currentLookAt.position.Y += moveIntoGeometry(m_currentLookAt, 306);

    auto tmp = m_currentLookAt;
    tmp.position.X -= m_distanceFromLookAt * m_localRotation.Y.sin();
    tmp.position.Z -= m_distanceFromLookAt * m_localRotation.Y.cos();
    tmp.position.Y += m_distanceFromLookAt * m_localRotation.X.sin();
    tmp.room = m_currentPosition.room;

    //! @todo clampBox(tmp, &freeLookClamp);

    m_currentLookAt.position.X = origLook.X + (m_currentLookAt.position.X - origLook.X) *  core::FrameRate * deltaTimeMs / 1000 / m_smoothFactor;
    m_currentLookAt.position.Z = origLook.Z + (m_currentLookAt.position.Z - origLook.Z) *  core::FrameRate * deltaTimeMs / 1000 / m_smoothFactor;

    updatePosition(tmp, m_smoothFactor, deltaTimeMs);
}

void CameraController::handleEnemy(const ItemController& item, int deltaTimeMs)
{
    m_currentLookAt.position.X = item.getPosition().X;
    m_currentLookAt.position.Z = item.getPosition().Z;

    if(m_enemy != nullptr)
    {
        m_localRotation.X += m_enemyLookRot.X;
        m_localRotation.Y += m_enemyLookRot.Y;
    }
    else
    {
        m_localRotation.X = m_freeLookRotation.X + m_headRotation.X + item.getRotation().X;
        m_localRotation.Y = m_freeLookRotation.Y + m_headRotation.Y + item.getRotation().Y;
    }

    m_distanceFromLookAt = 2560;
    auto tmp = m_currentLookAt;
    auto d = m_distanceFromLookAt * m_localRotation.X.cos();
    tmp.position.X -= d * m_localRotation.Y.sin();
    tmp.position.Z -= d * m_localRotation.Y.cos();
    tmp.position.Y += m_distanceFromLookAt * m_localRotation.X.sin();
    tmp.room = m_currentPosition.room;

    //! @todo clampBox(tmp, &sub_22918);
    updatePosition(tmp, m_smoothFactor, deltaTimeMs);
}
