#pragma once

#include "level.h"
#include "render/portaltracer.h"

#include <queue>
#include <set>

// Native TR floor data functions

#define TR_FD_FUNC_PORTALSECTOR                 0x01
#define TR_FD_FUNC_FLOORSLANT                   0x02
#define TR_FD_FUNC_CEILINGSLANT                 0x03
#define TR_FD_FUNC_TRIGGER                      0x04
#define TR_FD_FUNC_DEATH                        0x05
#define TR_FD_FUNC_CLIMB                        0x06
#define TR_FD_FUNC_FLOORTRIANGLE_NW             0x07    //  [_\_]
#define TR_FD_FUNC_FLOORTRIANGLE_NE             0x08    //  [_/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NW           0x09    //  [_/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NE           0x0A    //  [_\_]
#define TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_SW   0x0B    //  [P\_]
#define TR_FD_FUNC_FLOORTRIANGLE_NW_PORTAL_NE   0x0C    //  [_\P]
#define TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_SE   0x0D    //  [_/P]
#define TR_FD_FUNC_FLOORTRIANGLE_NE_PORTAL_NW   0x0E    //  [P/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_SW 0x0F    //  [P\_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NW_PORTAL_NE 0x10    //  [_\P]
#define TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_NW 0x11    //  [P/_]
#define TR_FD_FUNC_CEILINGTRIANGLE_NE_PORTAL_SE 0x12    //  [_/P]
#define TR_FD_FUNC_MONKEY                       0x13
#define TR_FD_FUNC_MINECART_LEFT                0x14    // In TR3 only. Function changed in TR4+.
#define TR_FD_FUNC_MINECART_RIGHT               0x15    // In TR3 only. Function changed in TR4+.


// Native TR trigger (TR_FD_FUNC_TRIGGER) types.

#define TR_FD_TRIGTYPE_TRIGGER          0x00    // If Lara is in sector, run (any case).
#define TR_FD_TRIGTYPE_PAD              0x01    // If Lara is in sector, run (land case).
#define TR_FD_TRIGTYPE_SWITCH           0x02    // If item is activated, run, else stop.
#define TR_FD_TRIGTYPE_KEY              0x03    // If item is activated, run.
#define TR_FD_TRIGTYPE_PICKUP           0x04    // If item is picked up, run.
#define TR_FD_TRIGTYPE_HEAVY            0x05    // If item is in sector, run, else stop.
#define TR_FD_TRIGTYPE_ANTIPAD          0x06    // If Lara is in sector, stop (land case).
#define TR_FD_TRIGTYPE_COMBAT           0x07    // If Lara is in combat state, run (any case).
#define TR_FD_TRIGTYPE_DUMMY            0x08    // If Lara is in sector, run (air case).
#define TR_FD_TRIGTYPE_ANTITRIGGER      0x09    // TR2-5 only: If Lara is in sector, stop (any case).
#define TR_FD_TRIGTYPE_HEAVYSWITCH      0x0A    // TR3-5 only: If item is activated by item, run.
#define TR_FD_TRIGTYPE_HEAVYANTITRIGGER 0x0B    // TR3-5 only: If item is activated by item, stop.
#define TR_FD_TRIGTYPE_MONKEY           0x0C    // TR3-5 only: If Lara is monkey-swinging, run.
#define TR_FD_TRIGTYPE_SKELETON         0x0D    // TR5 only: Activated by skeleton only?
#define TR_FD_TRIGTYPE_TIGHTROPE        0x0E    // TR5 only: If Lara is on tightrope, run.
#define TR_FD_TRIGTYPE_CRAWLDUCK        0x0F    // TR5 only: If Lara is crawling, run.
#define TR_FD_TRIGTYPE_CLIMB            0x10    // TR5 only: If Lara is climbing, run.

// Native trigger function types.

#define TR_FD_TRIGFUNC_OBJECT           0x00
#define TR_FD_TRIGFUNC_CAMERATARGET     0x01
#define TR_FD_TRIGFUNC_UWCURRENT        0x02
#define TR_FD_TRIGFUNC_FLIPMAP          0x03
#define TR_FD_TRIGFUNC_FLIPON           0x04
#define TR_FD_TRIGFUNC_FLIPOFF          0x05
#define TR_FD_TRIGFUNC_LOOKAT           0x06
#define TR_FD_TRIGFUNC_ENDLEVEL         0x07
#define TR_FD_TRIGFUNC_PLAYTRACK        0x08
#define TR_FD_TRIGFUNC_FLIPEFFECT       0x09
#define TR_FD_TRIGFUNC_SECRET           0x0A
#define TR_FD_TRIGFUNC_CLEARBODIES      0x0B    // Unused in TR4
#define TR_FD_TRIGFUNC_FLYBY            0x0C
#define TR_FD_TRIGFUNC_CUTSCENE         0x0D

class TRCameraSceneNodeAnimator final : public irr::scene::ISceneNodeAnimator
{
private:
    bool m_firstUpdate = true;
    irr::gui::ICursorControl* m_cursorControl;
    irr::core::vector2df m_prevCursorPos;
    irr::core::vector2df m_currentCursorPos;
    irr::u32 m_lastAnimationTime = 0;
    bool m_firstInput = true;
    irr::f32 m_rotateSpeed = 100;
    irr::f32 m_rotateSpeedLara = 0.1;
    irr::f32 m_maxVerticalAngle = 88;
    irr::f32 m_moveSpeed = 10;
    bool m_forward = false;
    bool m_backward = false;
    bool m_left = false;
    bool m_right = false;

    const loader::Level* m_level;
    const loader::Room* m_currentRoom;
    
    const irr::core::vector3df m_relativeTarget{0, 256, 0};
    irr::core::vector3df m_relativePosition{0, 0, -1024};
    
    void setOwnerRoom(const loader::Room* newRoom, irr::scene::IAnimatedMeshSceneNode* lara)
    {
        if(newRoom == m_currentRoom)
            return;

        BOOST_LOG_TRIVIAL(debug) << "Room switch";
        if(newRoom == nullptr)
        {
            BOOST_LOG_TRIVIAL(fatal) << "No room to switch to!";
            BOOST_LOG_TRIVIAL(fatal) << "Matching rooms by position:";
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
        
        BOOST_LOG_TRIVIAL(debug) << "Old room pos: " << m_currentRoom->node->getAbsolutePosition().X << "/" << m_currentRoom->node->getAbsolutePosition().Y << "/" << m_currentRoom->node->getAbsolutePosition().Z;
        BOOST_LOG_TRIVIAL(debug) << "New room pos: " << newRoom->node->getAbsolutePosition().X << "/" << newRoom->node->getAbsolutePosition().Y << "/" << newRoom->node->getAbsolutePosition().Z;
        BOOST_LOG_TRIVIAL(debug) << "Old lara pos: " << lara->getAbsolutePosition().X << "/" << lara->getAbsolutePosition().Y << "/" << lara->getAbsolutePosition().Z;

        lara->setParent(newRoom->node);
        lara->setPosition(lara->getAbsolutePosition() - newRoom->node->getAbsolutePosition());
        lara->updateAbsolutePosition();

        BOOST_LOG_TRIVIAL(debug) << "New lara pos: " << lara->getAbsolutePosition().X << "/" << lara->getAbsolutePosition().Y << "/" << lara->getAbsolutePosition().Z;
        
        m_currentRoom = newRoom;
    }

public:
    explicit TRCameraSceneNodeAnimator(irr::gui::ICursorControl* cursorControl, const loader::Level* level, loader::Room* currentRoom)
        : ISceneNodeAnimator(), m_cursorControl(cursorControl), m_level(level), m_currentRoom(currentRoom)
    {
        BOOST_ASSERT(cursorControl != nullptr);
        BOOST_ASSERT(currentRoom != nullptr);
    }

    //! Animates a scene node.
    /** \param node Node to animate.
    \param timeMs Current time in milli seconds. */
    virtual void animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs) override
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

        // get time
        irr::f32 timeDiff = timeMs - m_lastAnimationTime;
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

        irr::core::vector3d<irr::f32> forwardDir = {0,0,1};
        forwardDir.rotateXZBy(-lara->getRotation().Y);

        // update position
        auto laraPos = lara->getPosition();

        if(m_forward)
            laraPos += forwardDir * timeDiff * m_moveSpeed;

        if(m_backward)
            laraPos -= forwardDir * timeDiff * m_moveSpeed;

        // strafing

        irr::core::vector3d<irr::f32> rightDir = {1,0,0};
        rightDir.rotateXZBy(lara->getRotation().Y);

#if 0
        if(m_left)
            laraPos -= rightDir * timeDiff * m_moveSpeed;

        if(m_right)
            laraPos += rightDir * timeDiff * m_moveSpeed;
#else
        auto laraRot = lara->getRotation();
        const auto deltaRot = timeDiff * m_rotateSpeedLara;
        if(m_left)
        {
            laraRot.Y -= deltaRot;
        }

        if(m_right)
        {
            laraRot.Y += deltaRot;
        }
        lara->setRotation(laraRot);
#endif

        // write translation
        lara->setPosition(laraPos);

        lara->updateAbsolutePosition();
        camera->setPosition(m_relativePosition + m_relativeTarget);
        camera->updateAbsolutePosition();
        camera->setTarget(lara->getAbsolutePosition() + m_relativeTarget);
        camera->updateAbsolutePosition();
        
        tracePortals(camera);
    }

    //! Creates a clone of this animator.
    /** Please note that you will have to drop
    (IReferenceCounted::drop()) the returned pointer after calling this. */
    virtual irr::scene::ISceneNodeAnimator* createClone(irr::scene::ISceneNode* /*node*/, irr::scene::ISceneManager* /*newManager*/ = nullptr) override
    {
        BOOST_ASSERT(false);
        return nullptr;
    }

    //! Returns true if this animator receives events.
    /** When attached to an active camera, this animator will be
    able to respond to events such as mouse and keyboard events. */
    virtual bool isEventReceiverEnabled() const override
    {
        return true;
    }

    //! Event receiver, override this function for camera controlling animators
    virtual bool OnEvent(const irr::SEvent& evt) override
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
    
private:
    void tracePortals(irr::scene::ICameraSceneNode* camera)
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

    bool handleFloorData(irr::scene::IAnimatedMeshSceneNode* lara)
    {
        const loader::Sector* sector = m_currentRoom->getSectorByPosition(lara->getPosition());
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
            sector = m_currentRoom->getSectorByPosition(lara->getPosition());
            if(sector == nullptr)
            {
                BOOST_LOG_TRIVIAL(error) << "No sector for coordinates: " << lara->getPosition().X << "/" << lara->getPosition().Z;
                return false;
            }
        }
        
        // fix height
        {
            const int globalY = sector->floorHeight * 256;
//            BOOST_LOG_TRIVIAL(debug) << "fh=" << int(sector->floorHeight) << " => " << globalY;
            auto pos = lara->getPosition();
//            BOOST_LOG_TRIVIAL(debug) << "py=" << pos.Y << " ry=" << m_currentRoom->position.Y;
            pos.Y = -globalY - m_currentRoom->position.Y;
            lara->setPosition(pos);
        }
        
        if(sector->floorDataIndex == 0)
            return true;
        
        BOOST_ASSERT(sector->floorDataIndex < m_level->m_floorData.size());
//        BOOST_LOG_TRIVIAL(debug) << "Starting to parse floordata...";
        for(size_t fdi = sector->floorDataIndex; fdi < m_level->m_floorData.size(); /*nop*/)
        {
//            BOOST_LOG_TRIVIAL(debug) << "Parsing floordata @" << fdi;
            const uint16_t* floorData = &m_level->m_floorData[fdi];
            const uint16_t function = *floorData & 0x001F;             // 0b00000000 00011111
//            BOOST_LOG_TRIVIAL(debug) << "Floordata function " << int(function);
            const uint16_t subFunction = (*floorData & 0x7F00) >> 8;        // 0b01111111 00000000
    
            const bool isLastFloordata = ((*floorData & 0x8000) >> 15) != 0;       // 0b10000000 00000000
            
            ++fdi;
            ++floorData;
            
            switch(function)
            {
                case TR_FD_FUNC_PORTALSECTOR:          // PORTAL DATA
                    if(subFunction == 0x00)
                    {
                        if(*floorData < m_level->m_rooms.size())
                        {
                            BOOST_LOG_TRIVIAL(info) << "Switch to room: " << int(*floorData);
                            BOOST_ASSERT(*floorData < m_level->m_rooms.size());
                            setOwnerRoom(&m_level->m_rooms[*floorData], lara);
                            break;
                        }
                        ++floorData;
                        ++fdi;
                    }
                    break;
                case TR_FD_FUNC_FLOORSLANT:          // FLOOR SLANT
                case TR_FD_FUNC_CEILINGSLANT:          // CEILING SLANT
                    if(subFunction == 0)
                    {
                        const int8_t xSlant{ *floorData & 0x00FF };
                        const int8_t zSlant{ (*floorData & 0xFF00) >> 8 };
                        ++floorData;
                        ++fdi;
                        
                        if(function == TR_FD_FUNC_CEILINGSLANT)
                            break;
                        
                        auto laraPos = lara->getPosition();
                        
                        static constexpr irr::f32 QuarterSectorSize = 256;
                        static constexpr irr::f32 SectorSize = 1024;

                        const irr::f32 localX = std::fmod(laraPos.X, SectorSize);
                        const irr::f32 localZ = std::fmod(laraPos.Z, SectorSize);
                        
                        if(zSlant > 0) // lower edge at -Z
                        {
                            auto dist = (SectorSize - localZ) / SectorSize;
                            laraPos.Y -= dist * zSlant * QuarterSectorSize;
                        }
                        else if(zSlant < 0)  // lower edge at +Z
                        {
                            auto dist = localZ / SectorSize;
                            laraPos.Y += dist * zSlant * QuarterSectorSize;
                        }
    
                        if(xSlant > 0) // lower edge at -X
                        {
                            auto dist = (SectorSize - localX) / SectorSize;
                            laraPos.Y -= dist * xSlant * QuarterSectorSize;
                        }
                        else if(xSlant < 0) // lower edge at +X
                        {
                            auto dist = localX / SectorSize;
                            laraPos.Y += dist * xSlant * QuarterSectorSize;
                        }
                        
                        lara->setPosition(laraPos);
                        lara->updateAbsolutePosition();
                    }
                    break;
                case TR_FD_FUNC_TRIGGER:          // TRIGGERS
                    switch(subFunction)
                    {
                        case TR_FD_TRIGTYPE_TRIGGER:
                        case TR_FD_TRIGTYPE_HEAVY:
                        case TR_FD_TRIGTYPE_PAD:
                        case TR_FD_TRIGTYPE_ANTIPAD:
                        case TR_FD_TRIGTYPE_SWITCH:
                        case TR_FD_TRIGTYPE_HEAVYSWITCH:
                        case TR_FD_TRIGTYPE_KEY:
                        case TR_FD_TRIGTYPE_PICKUP:
                        case TR_FD_TRIGTYPE_COMBAT:
                        case TR_FD_TRIGTYPE_DUMMY:
                        case TR_FD_TRIGTYPE_SKELETON:   ///@FIXME: Find the meaning later!!!
                        case TR_FD_TRIGTYPE_ANTITRIGGER:
                        case TR_FD_TRIGTYPE_HEAVYANTITRIGGER:
                        case TR_FD_TRIGTYPE_MONKEY:
                        case TR_FD_TRIGTYPE_CLIMB:
                        case TR_FD_TRIGTYPE_TIGHTROPE:
                        case TR_FD_TRIGTYPE_CRAWLDUCK:
//                            BOOST_LOG_TRIVIAL(debug) << "Trigger subfunction " << int(subFunction);
                            break;
                        default:
                            BOOST_LOG_TRIVIAL(error) << "Unexpected trigger subfunction " << int(subFunction);
                            break;
                    }
                    for(bool contBit = false; contBit && fdi < m_level->m_floorData.size(); /*nop*/)
                    {
                        ++floorData;
                        ++fdi;
                        const uint16_t triggerFunction = (*floorData & 0x7C00) >> 10;
                        BOOST_LOG_TRIVIAL(debug) << "Parsing trigger @" << fdi << " / func=" << int(triggerFunction);
                        switch(triggerFunction)
                        {
                            case TR_FD_TRIGFUNC_OBJECT:         // ACTIVATE / DEACTIVATE object
                                break;
    
                            case TR_FD_TRIGFUNC_CAMERATARGET:
                                ++floorData;
                                ++fdi;
                                contBit = ((*floorData & 0x8000) >> 15) != 0;
                                break;
    
                            case TR_FD_TRIGFUNC_UWCURRENT:
                            case TR_FD_TRIGFUNC_FLIPMAP:
                            case TR_FD_TRIGFUNC_FLIPON:
                            case TR_FD_TRIGFUNC_FLIPOFF:
                            case TR_FD_TRIGFUNC_LOOKAT:
                            case TR_FD_TRIGFUNC_ENDLEVEL:
                            case TR_FD_TRIGFUNC_PLAYTRACK:
                            case TR_FD_TRIGFUNC_FLIPEFFECT:
                            case TR_FD_TRIGFUNC_SECRET:
                            case TR_FD_TRIGFUNC_CLEARBODIES:
                            case TR_FD_TRIGFUNC_FLYBY:
                                ++floorData;
                                ++fdi;
                                contBit = (*floorData & 0x8000) >> 15;
                                break;
    
                            case TR_FD_TRIGFUNC_CUTSCENE:
                                break;
    
                            default: // UNKNOWN!
                                BOOST_LOG_TRIVIAL(warning) << "Unhandled trigger function " << int(triggerFunction);
                                break;
                        };
                    }
                    break;
                case TR_FD_FUNC_DEATH:
                case TR_FD_FUNC_CLIMB:
                case TR_FD_FUNC_MONKEY:
                case TR_FD_FUNC_MINECART_LEFT:
                case TR_FD_FUNC_MINECART_RIGHT:
                    break;
                default:
                    BOOST_LOG_TRIVIAL(error) << "Unhandled floordata function " << int(function) << "; floorData=0x" << std::hex << floorData[-1] << std::dec << " @" << fdi-1;
                    break;
            }
            
            if(isLastFloordata)
                break;
        }
        
        return true;
    }
};
