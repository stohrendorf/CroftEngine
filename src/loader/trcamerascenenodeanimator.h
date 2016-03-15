#pragma once

#include "level.h"
#include "render/portaltracer.h"

#include <queue>
#include <set>

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
    
    const irr::core::vector3df m_relativeTarget{0, 256, 0};
    irr::core::vector3df m_relativePosition{0, 0, -1024};

public:
    explicit TRCameraSceneNodeAnimator(irr::gui::ICursorControl* cursorControl, const loader::Level* level)
        : ISceneNodeAnimator(), m_cursorControl(cursorControl), m_level(level)
    {
        BOOST_ASSERT(cursorControl != nullptr);
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

        uint16_t startRoom = std::numeric_limits<uint16_t>::max();
        for(size_t i = 0; i < m_level->m_rooms.size(); ++i)
        {
            const loader::Room& room = m_level->m_rooms[i];
            if(room.node->getTransformedBoundingBox().isPointInside(camera->getAbsolutePosition()))
            {
                startRoom = i;
                room.node->setVisible(true);
            }
            else
            {
                room.node->setVisible(false);
            }
        }

        if(startRoom >= m_level->m_rooms.size())
        {
            for(const loader::Room& room : m_level->m_rooms)
                room.node->setVisible(true);
            return;
        }

        // always process direct neighbours
        for(const loader::Portal& portal : m_level->m_rooms[startRoom].portals)
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
};
