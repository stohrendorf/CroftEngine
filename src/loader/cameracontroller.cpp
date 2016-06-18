#include "cameracontroller.h"

#include "animationcontroller.h"
#include "render/portaltracer.h"
#include "laracontroller.h"

#include <queue>
#include <set>

#ifndef NDEBUG
CameraController::CameraController(irr::gui::ICursorControl* cursorControl, const loader::Level* level, LaraController* laraController, irr::video::IVideoDriver* drv)
    : ISceneNodeAnimator(), m_cursorControl(cursorControl), m_level(level), m_laraController(laraController), m_driver(drv)
#else
TRCameraSceneNodeAnimator::TRCameraSceneNodeAnimator(irr::gui::ICursorControl* cursorControl, const loader::Level* level, LaraStateHandler* laraController)
    : ISceneNodeAnimator(), m_cursorControl(cursorControl), m_level(level), m_laraController(laraController)
#endif
{
    BOOST_ASSERT(cursorControl != nullptr);
    m_currentLookAt = m_level->m_lara->getSceneNode()->getAbsolutePosition();
    m_currentLookAt.Y += m_lookAtYOffset;
    m_currentPosition = m_currentLookAt;
    m_currentPosition.Z -= 100;
}

void CameraController::animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs)
{
    if(!node || node->getType() != irr::scene::ESNT_CAMERA)
        return;
    
    irr::scene::ICameraSceneNode* camera = static_cast<irr::scene::ICameraSceneNode*>(node);
    irr::scene::ISceneManager* smgr = node->getSceneManager();
    if(smgr && smgr->getActiveCamera() != camera)
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
    if(!camera->isInputReceiverEnabled())
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
        m_localRotation.Y += m_rotateSpeed * -(0.5f - m_currentCursorPos.X);
        m_localRotation.X += m_rotateSpeed *  (0.5f - m_currentCursorPos.Y);
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
    
    auto targetLookAt = m_laraController->getPosition().toIrrlicht();
    targetLookAt.Y += m_lookAtYOffset;
    m_currentLookAt += (targetLookAt - m_currentLookAt) * 30 / m_smoothFactor * localTime / 1000;

    const irr::core::vector3d<core::Angle> totalRotation = m_localRotation + m_laraController->getRotation();

    irr::core::vector3df targetPos = m_currentLookAt;
    const auto localDistance = m_distanceFromLookAt * totalRotation.X.cos();
    targetPos.X -= totalRotation.Y.sin() * localDistance;
    targetPos.Z -= totalRotation.Y.cos() * localDistance;
    targetPos.Y -= totalRotation.X.sin() * m_distanceFromLookAt;

    const auto d = targetPos - m_currentPosition;
    const auto bias = 30.0f / m_smoothFactor * localTime / 1000;
    m_currentPosition += d * bias;

    auto pos = m_currentPosition;
    clamp(m_currentLookAt, pos);

    camera->setPosition(pos);
    camera->updateAbsolutePosition();
    camera->setTarget(m_currentLookAt);
    camera->updateAbsolutePosition();
    
    tracePortals(camera);
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
            break;
            
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

void CameraController::tracePortals(irr::scene::ICameraSceneNode* camera)
{
    // Breadth-first queue
    std::queue<render::PortalTracer> toVisit;
    
    bool cameraOutOfGeometry = true;
    for(size_t i = 0; i < m_level->m_rooms.size(); ++i)
    {
        const loader::Room& room = m_level->m_rooms[i];
        if(room.node->getTransformedBoundingBox().isPointInside(camera->getAbsolutePosition()))
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
        toVisit.push(m_laraController->getCurrentRoom());
        while(!toVisit.empty())
        {
            auto currentRoom = toVisit.front();
            toVisit.pop();
            if(currentRoom->node->getTransformedBoundingBox().isPointInside(camera->getAbsolutePosition()))
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

    // always process direct neighbours of the starting room
    for(const loader::Portal& portal : startRoom->portals)
    {
        render::PortalTracer path;
#ifndef NDEBUG
        if(!path.checkVisibility(&portal, *camera, m_driver))
            continue;
#else
        if(!path.checkVisibility(&portal, *camera))
            continue;
#endif
        
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
#ifndef NDEBUG
            if(!newPath.checkVisibility(&srcPortal, *camera, m_driver))
                continue;
#else
            if(!newPath.checkVisibility(&srcPortal, *camera))
                continue;
#endif
            
            m_level->m_rooms[srcPortal.adjoining_room].node->setVisible(true);
            toVisit.emplace(std::move(newPath));
        }
    }
}

bool CameraController::clampY(const irr::core::vector3df& lookAt, irr::core::vector3df& origin, const loader::Sector* sector) const
{
    BOOST_ASSERT(sector != nullptr);

    const auto d = origin - lookAt;
    const HeightInfo floor = HeightInfo::fromFloor(sector, loader::TRCoordinates(origin), this);
    if(floor.distance < -origin.Y && floor.distance > -lookAt.Y)
    {
        origin.Y = -floor.distance;
        origin.X = d.X / d.Y * (-floor.distance - lookAt.Y) + lookAt.X;
        origin.Z = d.Z / d.Y * (-floor.distance - lookAt.Y) + lookAt.Z;
        return false;
    }

    const HeightInfo ceiling = HeightInfo::fromCeiling(sector, loader::TRCoordinates(origin), this);
    if(ceiling.distance > -origin.Y && ceiling.distance < -lookAt.Y)
    {
        origin.Y = -ceiling.distance;
        origin.X = d.X / d.Y * (-ceiling.distance - lookAt.Y) + lookAt.X;
        origin.Z = d.Z / d.Y * (-ceiling.distance - lookAt.Y) + lookAt.Z;
        return false;
    }

    return true;
}

CameraController::ClampType CameraController::clampX(const irr::core::vector3df& lookAt, irr::core::vector3df& origin) const
{
    if(irr::core::equals(lookAt.X, origin.X, 1.0f))
        return ClampType::None;

    const auto d = origin - lookAt;
    const auto gradientZX = d.Z / d.X;
    const auto gradientYX = d.Y / d.X;

    if(d.X < 0)
    {
        auto sectorBoundary = std::floor(lookAt.X / loader::SectorSize) * loader::SectorSize;
        if(sectorBoundary <= origin.X)
            return ClampType::None;

        irr::core::vector3df localPos;
        localPos.X = sectorBoundary - lookAt.X;
        localPos.Y = lookAt.Y + localPos.X * gradientYX;
        localPos.Z = lookAt.Z + localPos.X * gradientZX;

        auto innerBoundary = sectorBoundary - 1;
        while(true)
        {
            loader::TRCoordinates pos(sectorBoundary, localPos.Y, localPos.Z);
            auto sector = m_level->findSectorForPosition(pos, m_laraController->getCurrentRoom());
            HeightInfo floor = HeightInfo::fromFloor(sector, pos, this);
            HeightInfo ceiling = HeightInfo::fromCeiling(sector, pos, this);
            if(-localPos.Y > floor.distance || -localPos.Y < ceiling.distance)
            {
                origin.X = sectorBoundary;
                origin.Y = localPos.Y;
                origin.Z = localPos.Z;
                return ClampType::Outer;
            }

            pos.X = innerBoundary;
            sector = m_level->findSectorForPosition(pos, m_laraController->getCurrentRoom());
            floor = HeightInfo::fromFloor(sector, pos, this);
            ceiling = HeightInfo::fromCeiling(sector, pos, this);
            if(-localPos.Y > floor.distance || -localPos.Y < ceiling.distance)
            {
                origin.X = sectorBoundary;
                origin.Y = localPos.Y;
                origin.Z = localPos.Z;
                return ClampType::Inner;
            }

            sectorBoundary -= loader::SectorSize;
            innerBoundary -= loader::SectorSize;
            localPos.Y -= gradientYX * loader::SectorSize;
            localPos.Z -= gradientZX * loader::SectorSize;

            if(sectorBoundary <= origin.X)
                return ClampType::None;
        }
    }

    auto sectorBoundary = std::floor(lookAt.X / loader::SectorSize) * loader::SectorSize + loader::SectorSize - 1;

    if(sectorBoundary >= origin.X)
        return ClampType::None;

    irr::core::vector3df localPos;
    localPos.X = sectorBoundary - lookAt.X;
    localPos.Y = lookAt.Y + localPos.X * gradientYX;
    localPos.Z = lookAt.Z + localPos.X * gradientZX;

    auto innerBoundary = sectorBoundary + 1;
    while(true)
    {
        loader::TRCoordinates pos(sectorBoundary, localPos.Y, localPos.Z);
        auto sector = m_level->findSectorForPosition(pos, m_laraController->getCurrentRoom());
        HeightInfo floor = HeightInfo::fromFloor(sector, pos, this);
        HeightInfo ceiling = HeightInfo::fromCeiling(sector, pos, this);
        if(-localPos.Y > floor.distance || -localPos.Y < ceiling.distance)
        {
            origin.X = sectorBoundary;
            origin.Y = localPos.Y;
            origin.Z = localPos.Z;
            return ClampType::Outer;
        }

        pos.X = innerBoundary;
        sector = m_level->findSectorForPosition(pos, m_laraController->getCurrentRoom());
        floor = HeightInfo::fromFloor(sector, pos, this);
        ceiling = HeightInfo::fromCeiling(sector, pos, this);
        if(-localPos.Y > floor.distance || -localPos.Y < ceiling.distance)
        {
            origin.X = innerBoundary;
            origin.Y = localPos.Y;
            origin.Z = localPos.Z;
            return ClampType::Inner;
        }

        sectorBoundary += loader::SectorSize;
        innerBoundary += loader::SectorSize;
        localPos.Y += gradientYX * loader::SectorSize;
        localPos.Z += gradientZX * loader::SectorSize;

        if(sectorBoundary >= origin.X)
            return ClampType::None;
    }
}

CameraController::ClampType CameraController::clampZ(const irr::core::vector3df& lookAt, irr::core::vector3df& origin) const
{
    if(irr::core::equals(lookAt.Z, origin.Z, 1.0f))
        return ClampType::None;

    const auto d = origin - lookAt;
    const auto gradientXZ = d.X / d.Z;
    const auto gradientYZ = d.Y / d.Z;

    if(d.Z < 0)
    {
        auto sectorBoundary = std::floor(lookAt.Z / loader::SectorSize) * loader::SectorSize;
        if(sectorBoundary <= origin.Z)
            return ClampType::None;

        irr::core::vector3df localPos;
        localPos.Z = sectorBoundary - lookAt.Z;
        localPos.X = lookAt.X + localPos.Z * gradientXZ;
        localPos.Y = lookAt.Y + localPos.Z * gradientYZ;

        auto innerBoundary = sectorBoundary - 1;
        auto room = m_laraController->getCurrentRoom();
        while(true)
        {
            loader::TRCoordinates pos(localPos.X, localPos.Y, sectorBoundary);
            auto sector = m_level->findSectorForPosition(pos, &room);
            HeightInfo floor = HeightInfo::fromFloor(sector, pos, this);
            HeightInfo ceiling = HeightInfo::fromCeiling(sector, pos, this);
            if(-localPos.Y > floor.distance || -localPos.Y < ceiling.distance)
            {
                origin.X = localPos.X;
                origin.Y = localPos.Y;
                origin.Z = sectorBoundary;
                return ClampType::Outer;
            }

            pos.Z = innerBoundary;
            sector = m_level->findSectorForPosition(pos, &room);
            floor = HeightInfo::fromFloor(sector, pos, this);
            ceiling = HeightInfo::fromCeiling(sector, pos, this);
            if(-localPos.Y > floor.distance || -localPos.Y < ceiling.distance)
            {
                origin.X = localPos.X;
                origin.Y = localPos.Y;
                origin.Z = innerBoundary;
                return ClampType::Inner;
            }

            sectorBoundary -= loader::SectorSize;
            innerBoundary -= loader::SectorSize;
            localPos.X -= gradientXZ * loader::SectorSize;
            localPos.Y -= gradientYZ * loader::SectorSize;

            if(sectorBoundary <= origin.Z)
                return ClampType::None;
        }
    }

    auto sectorBoundary = std::floor(lookAt.Z / loader::SectorSize) * loader::SectorSize + loader::SectorSize - 1;

    if(sectorBoundary >= origin.Z)
        return ClampType::None;

    irr::core::vector3df localPos;
    localPos.Z = sectorBoundary - lookAt.Z;
    localPos.X = lookAt.X + localPos.Z * gradientXZ;
    localPos.Y = lookAt.Y + localPos.Z * gradientYZ;

    auto innerBoundary = sectorBoundary + 1;
    auto room = m_laraController->getCurrentRoom();
    while(true)
    {
        loader::TRCoordinates pos(localPos.X, localPos.Y, sectorBoundary);
        auto sector = m_level->findSectorForPosition(pos, &room);
        HeightInfo floor = HeightInfo::fromFloor(sector, pos, this);
        HeightInfo ceiling = HeightInfo::fromCeiling(sector, pos, this);
        if(-localPos.Y > floor.distance || -localPos.Y < ceiling.distance)
        {
            origin.X = localPos.X;
            origin.Y = localPos.Y;
            origin.Z = sectorBoundary;
            return ClampType::Outer;
        }

        pos.Z = innerBoundary;
        sector = m_level->findSectorForPosition(pos, &room);
        floor = HeightInfo::fromFloor(sector, pos, this);
        ceiling = HeightInfo::fromCeiling(sector, pos, this);
        if(-localPos.Y > floor.distance || -localPos.Y < ceiling.distance)
        {
            origin.X = localPos.X;
            origin.Y = localPos.Y;
            origin.Z = innerBoundary;
            return ClampType::Inner;
        }

        sectorBoundary += loader::SectorSize;
        innerBoundary += loader::SectorSize;
        localPos.X += gradientXZ * loader::SectorSize;
        localPos.Y += gradientYZ * loader::SectorSize;

        if(sectorBoundary >= origin.Z)
            return ClampType::None;
    }
}

bool CameraController::clamp(const irr::core::vector3df& lookAt, irr::core::vector3df& origin) const
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

    if(secondClamp == ClampType::Inner)
        return false;

    auto sector = m_level->findSectorForPosition(loader::TRCoordinates(origin), m_laraController->getCurrentRoom());
    return clampY(lookAt, origin, sector) && firstUnclamped && secondClamp == ClampType::None;
}
