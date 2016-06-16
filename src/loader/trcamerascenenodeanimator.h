#pragma once

#include "level.h"
#include "inputstate.h"

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

    bool m_forward = false;
    bool m_backward = false;
    bool m_left = false;
    bool m_right = false;
    bool m_stepLeft = false;
    bool m_stepRight = false;

    InputState m_inputState;

    const loader::Level* m_level;
    const loader::Room* m_currentRoom;
    LaraStateHandler* m_stateHandler;
    
    int m_lookAtYOffset = 1024;
    int m_distanceFromLookAt = 1536;
    int m_smoothFactor = 3; // 12
    irr::core::vector3df m_currentLookAt;
    irr::core::vector3df m_localRotation;
    irr::core::vector3df m_currentPosition;
    
#ifndef NDEBUG
    irr::video::IVideoDriver* m_driver;
#endif

public:
#ifndef NDEBUG
    explicit TRCameraSceneNodeAnimator(irr::gui::ICursorControl* cursorControl, const loader::Level* level, loader::Room* currentRoom, LaraStateHandler* stateHandler, irr::video::IVideoDriver* drv);
#else
    explicit TRCameraSceneNodeAnimator(irr::gui::ICursorControl* cursorControl, const loader::Level* level, loader::Room* currentRoom, LaraStateHandler* stateHandler);
#endif

    //! Animates a scene node.
    /** \param node Node to animate.
    \param timeMs Current time in milli seconds. */
    void animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs) override;

    //! Creates a clone of this animator.
    /** Please note that you will have to drop
    (IReferenceCounted::drop()) the returned pointer after calling this. */
    irr::scene::ISceneNodeAnimator* createClone(irr::scene::ISceneNode* /*node*/, irr::scene::ISceneManager* /*newManager*/ = nullptr) override;

    //! Returns true if this animator receives events.
    /** When attached to an active camera, this animator will be
    able to respond to events such as mouse and keyboard events. */
    bool isEventReceiverEnabled() const override
    {
        return true;
    }

    //! Event receiver, override this function for camera controlling animators
    bool OnEvent(const irr::SEvent& evt) override;
    
    const loader::Room* getCurrentRoom() const noexcept
    {
        return m_currentRoom;
    }

    void setCurrentRoom(const loader::Room* newRoom);

    const loader::Level* getLevel() const noexcept
    {
        return m_level;
    }

    void setLocalRotation(int16_t x, int16_t y);
    void setLocalRotationX(int16_t x);
    void setLocalRotationY(int16_t y);

private:
    void tracePortals(irr::scene::ICameraSceneNode* camera);
    bool moveIntoRoomGeometry(const irr::core::vector3df& lookAt, irr::core::vector3df& origin, const loader::Sector* sector) const;
    int moveX(const irr::core::vector3df& lookAt, irr::core::vector3df& origin) const;
    int moveZ(const irr::core::vector3df& lookAt, irr::core::vector3df& origin) const;
    bool tryLookAt(const irr::core::vector3df& lookAt, irr::core::vector3df& origin) const;
};
