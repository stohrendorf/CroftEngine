#pragma once

#include "level.h"

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
    irr::f32 m_rotateSpeedLara = 0.1f;
    irr::f32 m_maxVerticalAngle = 88;
    irr::f32 m_moveSpeed = 10;
    bool m_forward = false;
    bool m_backward = false;
    bool m_left = false;
    bool m_right = false;
    bool m_moveSlow = false;
    bool m_jump = false;

    const loader::Level* m_level;
    const loader::Room* m_currentRoom;
    LaraStateHandler* m_stateHandler;
    
    const irr::core::vector3df m_relativeTarget{0, 256, 0};
    irr::core::vector3df m_relativePosition{0, 0, -1024};
    
    void setOwnerRoom(const loader::Room* newRoom, irr::scene::IAnimatedMeshSceneNode* lara);

public:
    explicit TRCameraSceneNodeAnimator(irr::gui::ICursorControl* cursorControl, const loader::Level* level, loader::Room* currentRoom, LaraStateHandler* stateHandler);

    //! Animates a scene node.
    /** \param node Node to animate.
    \param timeMs Current time in milli seconds. */
    virtual void animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs) override;

    //! Creates a clone of this animator.
    /** Please note that you will have to drop
    (IReferenceCounted::drop()) the returned pointer after calling this. */
    virtual irr::scene::ISceneNodeAnimator* createClone(irr::scene::ISceneNode* /*node*/, irr::scene::ISceneManager* /*newManager*/ = nullptr) override;

    //! Returns true if this animator receives events.
    /** When attached to an active camera, this animator will be
    able to respond to events such as mouse and keyboard events. */
    virtual bool isEventReceiverEnabled() const override
    {
        return true;
    }

    //! Event receiver, override this function for camera controlling animators
    virtual bool OnEvent(const irr::SEvent& evt) override;
    
    const loader::Room* getCurrentRoom() const noexcept
    {
        return m_currentRoom;
    }

    const loader::Level* getLevel() const noexcept
    {
        return m_level;
    }

private:
    void tracePortals(irr::scene::ICameraSceneNode* camera);

    bool handleFloorData(irr::scene::IAnimatedMeshSceneNode* lara);
    loader::FloorData::const_iterator handleTrigger(loader::FloorData::const_iterator floorDataIt, loader::TriggerType triggerType);
};
