#include "trcamerascenenodeanimator.h"

#include "defaultanimdispatcher.h"
#include "render/portaltracer.h"
#include "larastatehandler.h"

#include <queue>
#include <set>

void TRCameraSceneNodeAnimator::setCurrentRoom(const loader::Room* newRoom)
{
    if(newRoom == m_currentRoom)
        return;
    
    BOOST_LOG_TRIVIAL(debug) << "Room switch to " << newRoom->node->getName();
    if(newRoom == nullptr)
    {
        BOOST_LOG_TRIVIAL(fatal) << "No room to switch to. Matching rooms by position:";
        for(size_t i = 0; i < m_level->m_rooms.size(); ++i)
        {
            const loader::Room& room = m_level->m_rooms[i];
            if(room.node->getTransformedBoundingBox().isPointInside(m_level->m_lara->getAbsolutePosition()))
            {
                BOOST_LOG_TRIVIAL(fatal) << "  - " << i;
            }
        }
        return;
    }
    
    m_currentRoom = newRoom;
    for(irr::u32 i = 0; i < m_level->m_lara->getMaterialCount(); ++i)
    {
        irr::video::SMaterial& material = m_level->m_lara->getMaterial(i);
        const auto col = m_currentRoom->lightColor.toSColor(m_currentRoom->intensity1 / 8191.0f / 4);
        material.EmissiveColor = col;
        material.AmbientColor = col;
    }
}

TRCameraSceneNodeAnimator::TRCameraSceneNodeAnimator(irr::gui::ICursorControl* cursorControl, const loader::Level* level, loader::Room* currentRoom, LaraStateHandler* stateHandler)
    : ISceneNodeAnimator(), m_cursorControl(cursorControl), m_level(level), m_currentRoom(nullptr), m_stateHandler(stateHandler)
{
    BOOST_ASSERT(cursorControl != nullptr);
    BOOST_ASSERT(currentRoom != nullptr);
    setCurrentRoom(currentRoom);
}

void TRCameraSceneNodeAnimator::animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs)
{
    if(!node || node->getType() != irr::scene::ESNT_CAMERA)
        return;
    
    irr::scene::ICameraSceneNode* camera = static_cast<irr::scene::ICameraSceneNode*>(node);
    irr::scene::ISceneManager* smgr = node->getSceneManager();
    if(smgr && smgr->getActiveCamera() != camera)
        return;
    
    if(!camera->getParent() || camera->getParent()->getType() != irr::scene::ESNT_ANIMATED_MESH)
        return;
    
    irr::scene::IAnimatedMeshSceneNode* lara = static_cast<irr::scene::IAnimatedMeshSceneNode*>(camera->getParent());

    m_inputState.setXAxisMovement(m_left, m_right);
    m_inputState.setZAxisMovement(m_backward, m_forward);
    m_inputState.setStepMovement(m_stepLeft, m_stepRight);
    m_stateHandler->setInputState(m_inputState);
    
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
    
    m_lastAnimationTime = timeMs;
    
    // Update mouse rotation
    if(m_currentCursorPos != m_prevCursorPos)
    {
        auto relativeCameraPosition = m_relativePosition - m_relativeTarget;
        irr::core::vector3df relativeRotation{0,0,0};
        relativeRotation.Y = -(0.5f - m_currentCursorPos.X) * m_rotateSpeed;
        relativeRotation.X = -(0.5f - m_currentCursorPos.Y) * m_rotateSpeed;
        
        // X < MaxVerticalAngle or X > 360-MaxVerticalAngle
        
        if(relativeRotation.X > m_maxVerticalAngle * 2 && relativeRotation.X < 360.0f - m_maxVerticalAngle)
        {
            relativeRotation.X = 360.0f - m_maxVerticalAngle;
        }
        else if(relativeRotation.X > m_maxVerticalAngle && relativeRotation.X < 360.0f - m_maxVerticalAngle)
        {
            relativeRotation.X = m_maxVerticalAngle;
        }
        
        // Do the fix as normal, special case below
        // reset cursor position to the centre of the window.
        m_cursorControl->setPosition(0.5f, 0.5f);
        
        // needed to avoid problems when the event receiver is disabled
        m_currentCursorPos = m_prevCursorPos = m_cursorControl->getRelativePosition();
        irr::core::matrix4 mat(irr::core::IdentityMatrix);
        mat.setRotationDegrees({relativeRotation.X, relativeRotation.Y, 0});
        mat.transformVect(relativeCameraPosition);
        m_relativePosition = m_relativeTarget + relativeCameraPosition;
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
    
    lara->updateAbsolutePosition();
    camera->setPosition(m_relativePosition + m_relativeTarget);
    camera->updateAbsolutePosition();
    camera->setTarget(lara->getJointNode("chest")->getAbsolutePosition() + m_relativeTarget);
    camera->updateAbsolutePosition();
    
    tracePortals(camera);
}

irr::scene::ISceneNodeAnimator*TRCameraSceneNodeAnimator::createClone(irr::scene::ISceneNode*, irr::scene::ISceneManager*)
{
    BOOST_ASSERT(false);
    return nullptr;
}

bool TRCameraSceneNodeAnimator::OnEvent(const irr::SEvent& evt)
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
            if(evt.MouseInput.Event == irr::EMIE_MOUSE_MOVED)
            {
                m_currentCursorPos = m_cursorControl->getRelativePosition();
                return true;
            }
            break;
            
        default:
            break;
    }
    
    return false;
}

void TRCameraSceneNodeAnimator::tracePortals(irr::scene::ICameraSceneNode* camera)
{
    // Breadth-first queue
    std::queue<render::PortalTracer> toVisit;
    
    const loader::Room* cameraRoom = nullptr;
    for(size_t i = 0; i < m_level->m_rooms.size(); ++i)
    {
        const loader::Room& room = m_level->m_rooms[i];
        if(room.node->getTransformedBoundingBox().isPointInside(camera->getAbsolutePosition()))
        {
            cameraRoom = &room;
            room.node->setVisible(true);
        }
        else
        {
            room.node->setVisible(false);
        }
    }
    
    if(cameraRoom == nullptr)
    {
        for(const loader::Room& room : m_level->m_rooms)
            room.node->setVisible(true);
        return;
    }
    
    m_currentRoom->node->setVisible(true);
    
    // always process direct neighbours
    for(const loader::Portal& portal : m_currentRoom->portals)
    {
        render::PortalTracer path;
        if(!path.checkVisibility(&portal, camera->getAbsolutePosition(), *camera->getViewFrustum()))
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
            if(!newPath.checkVisibility(&srcPortal, camera->getAbsolutePosition(), *camera->getViewFrustum()))
                continue;
            
            m_level->m_rooms[srcPortal.adjoining_room].node->setVisible(true);
            toVisit.emplace(std::move(newPath));
        }
    }
}
