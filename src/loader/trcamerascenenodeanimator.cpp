#include "trcamerascenenodeanimator.h"

#include "defaultanimdispatcher.h"
#include "render/portaltracer.h"
#include "larastatehandler.h"

#include <queue>
#include <set>

void TRCameraSceneNodeAnimator::setOwnerRoom(const loader::Room* newRoom, irr::scene::IAnimatedMeshSceneNode* lara)
{
    if(newRoom == m_currentRoom)
        return;
    
    BOOST_LOG_TRIVIAL(debug) << "Room switch";
    if(newRoom == nullptr)
    {
        BOOST_LOG_TRIVIAL(fatal) << "No room to switch to. Matching rooms by position:";
        for(size_t i = 0; i < m_level->m_rooms.size(); ++i)
        {
            const loader::Room& room = m_level->m_rooms[i];
            if(room.node->getTransformedBoundingBox().isPointInside(lara->getAbsolutePosition()))
            {
                BOOST_LOG_TRIVIAL(fatal) << "  - " << i;
            }
        }
        return;
    }
    
    m_currentRoom = newRoom;
    for(irr::u32 i = 0; i < lara->getMaterialCount(); ++i)
    {
        irr::video::SMaterial& material = lara->getMaterial(i);
        const auto col = m_currentRoom->lightColor.toSColor(m_currentRoom->intensity1 / 8191.0f / 4);
        material.EmissiveColor = col;
        material.AmbientColor = col;
    }
}

TRCameraSceneNodeAnimator::TRCameraSceneNodeAnimator(irr::gui::ICursorControl* cursorControl, const loader::Level* level, loader::Room* currentRoom, irr::scene::IAnimatedMeshSceneNode* lara, LaraStateHandler* stateHandler)
    : ISceneNodeAnimator(), m_cursorControl(cursorControl), m_level(level), m_currentRoom(nullptr), m_stateHandler(stateHandler)
{
    BOOST_ASSERT(cursorControl != nullptr);
    BOOST_ASSERT(currentRoom != nullptr);
    setOwnerRoom(currentRoom, lara);
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
    m_stateHandler->setInputState(m_inputState);
    
    handleFloorData(lara);
    lara->updateAbsolutePosition();
    
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
        m_left = m_right = m_forward = m_backward = false;
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

bool TRCameraSceneNodeAnimator::handleFloorData(irr::scene::IAnimatedMeshSceneNode* lara)
{
    const loader::Sector* sector = m_currentRoom->getSectorByAbsolutePosition(loader::TRCoordinates(lara->getAbsolutePosition()));
    if(sector == nullptr)
    {
        BOOST_LOG_TRIVIAL(error) << "No sector for coordinates: " << lara->getPosition().X << "/" << lara->getPosition().Z;
        return false;
    }
    
    // check "room below"
    while(sector->roomBelow != 0xff)
    {
        BOOST_ASSERT(sector->roomBelow < m_level->m_rooms.size());
        setOwnerRoom(&m_level->m_rooms[sector->roomBelow], lara);
        sector = m_currentRoom->getSectorByAbsolutePosition(loader::TRCoordinates(lara->getAbsolutePosition()));
        if(sector == nullptr)
        {
            BOOST_LOG_TRIVIAL(error) << "No sector for coordinates: " << lara->getPosition().X << "/" << lara->getPosition().Z;
            return false;
        }
    }

    if(sector->floorDataIndex == 0 || sector->floorDataIndex >= m_level->m_floorData.size())
        return true;
    
    BOOST_ASSERT(sector->floorDataIndex < m_level->m_floorData.size());
    //        BOOST_LOG_TRIVIAL(debug) << "Starting to parse floordata...";

    using loader::FDFunction;
    for(auto floorDataIt = std::next(m_level->m_floorData.begin(), sector->floorDataIndex); floorDataIt < m_level->m_floorData.end(); /*nop*/)
    {
        //            BOOST_LOG_TRIVIAL(debug) << "Parsing floordata @" << fdi;
        const FDFunction function = static_cast<FDFunction>(*floorDataIt & 0x001F);             // 0b00000000 00011111
        //            BOOST_LOG_TRIVIAL(debug) << "Floordata function " << int(function);
        const uint16_t subFunction = (*floorDataIt & 0x7F00) >> 8;        // 0b01111111 00000000
        
        const bool isLastFloordata = ((*floorDataIt & 0x8000) >> 15) != 0;       // 0b10000000 00000000
        
        ++floorDataIt;
        
        switch(function)
        {
            case FDFunction::PortalSector:          // PORTAL DATA
                if(*floorDataIt < m_level->m_rooms.size())
                {
                    BOOST_LOG_TRIVIAL(info) << "Switch to room: " << int(*floorDataIt);
                    BOOST_ASSERT(*floorDataIt < m_level->m_rooms.size());
                    setOwnerRoom(&m_level->m_rooms[*floorDataIt], lara);
                }
                ++floorDataIt;
                break;
            case FDFunction::FloorSlant:          // FLOOR SLANT
            case FDFunction::CeilingSlant:          // CEILING SLANT
                ++floorDataIt;
                break;
            case FDFunction::Trigger:          // TRIGGERS
                floorDataIt = handleTrigger(floorDataIt, static_cast<loader::TriggerType>(subFunction));
                break;
            case FDFunction::Death:
            case FDFunction::Climb:
            case FDFunction::Monkey:
            case FDFunction::MinecartLeft:
            case FDFunction::MinecartRight:
                break;
            default:
                BOOST_LOG_TRIVIAL(error) << "Unhandled floordata function " << int(function) << "; floorData=0x" << std::hex << *std::prev(floorDataIt) << std::dec;
                break;
        }
        
        if(isLastFloordata)
            break;
    }
    
    return true;
}

loader::FloorData::const_iterator TRCameraSceneNodeAnimator::handleTrigger(loader::FloorData::const_iterator floorDataIt, loader::TriggerType triggerType)
{
    const int8_t timer = static_cast<int8_t>(*floorDataIt & 0x00FF);
    const uint8_t mask = (*floorDataIt & 0x3E00) >> 9;
    const bool onlyOnce = ((*floorDataIt & 0x0100) >> 8) != 0;
    
    //BOOST_LOG_TRIVIAL(debug) << "Trigger: timer=" << int(timer) << ", mask=0x" << std::hex << int(mask) << std::dec << ", onlyOnce=" << (onlyOnce ? "true" : "false");
    
    using loader::TriggerType;
    using loader::TriggerFunction;

    switch(triggerType)
    {
        case TriggerType::Trigger:
        case TriggerType::Heavy:
        case TriggerType::Pad:
        case TriggerType::AntiPad:
        case TriggerType::Switch:
        case TriggerType::HeavySwitch:
        case TriggerType::Key:
        case TriggerType::Pickup:
        case TriggerType::Combat:
        case TriggerType::Dummy:
        case TriggerType::Skeleton:   ///@FIXME: Find the meaning later!!!
        case TriggerType::AntiTrigger:
        case TriggerType::HeavyAntiTrigger:
        case TriggerType::Monkey:
        case TriggerType::Climb:
        case TriggerType::TightRope:
        case TriggerType::CrawlDuck:
            //BOOST_LOG_TRIVIAL(debug) << "Trigger type " << int(triggerType);
            break;
        default:
            BOOST_LOG_TRIVIAL(error) << "Unexpected trigger type " << int(triggerType);
            break;
    }
    loader::AbstractTriggerHandler* const laraHandler = m_level->findHandler(0);
    for(bool breakBit = false; !breakBit && floorDataIt < m_level->m_floorData.end(); /*nop*/)
    {
        ++floorDataIt;
        breakBit = (*floorDataIt & 0x8000) != 0;
        const TriggerFunction triggerFunction = static_cast<TriggerFunction>((*floorDataIt & 0x7C00) >> 10);
        const uint16_t operand = *floorDataIt & 0x03FF;
        //BOOST_LOG_TRIVIAL(debug) << "  - func=" << int(triggerFunction) << " op=" << int(operand);
        loader::AbstractTriggerHandler* const handler = m_level->findHandler(operand);
        switch(triggerFunction)
        {
            case TriggerFunction::Object:
                if(handler)
                {
                    switch(triggerType)
                    {
                        case TriggerType::AntiPad:
                        case TriggerType::AntiTrigger:
                        case TriggerType::HeavyAntiTrigger:
                            handler->deactivate(laraHandler, onlyOnce);
                            break;
                        default:
                            handler->activate(laraHandler, mask, loader::TriggerOp::Or, onlyOnce, timer);
                    }

                }
                break;
                
            case TriggerFunction::CameraTarget:
                ++floorDataIt;
                breakBit = (*floorDataIt & 0x8000) != 0;
                break;
                
            case TriggerFunction::FlipMap:
            case TriggerFunction::FlipOn:
            case TriggerFunction::FlipOff:
            case TriggerFunction::LookAt:
            case TriggerFunction::EndLevel:
            case TriggerFunction::PlayTrack:
            case TriggerFunction::FlipEffect:
            case TriggerFunction::Secret:
            case TriggerFunction::ClearBodies:
            case TriggerFunction::FlyBy:
                ++floorDataIt;
                breakBit = (*floorDataIt & 0x8000) != 0;
                break;
                
            case TriggerFunction::CutScene:
            case TriggerFunction::UnderwaterCurrent:
                break;
                
            default: // UNKNOWN!
                BOOST_LOG_TRIVIAL(warning) << "  - Unhandled trigger function " << int(triggerFunction);
                break;
        };
    }
    return floorDataIt;
}
