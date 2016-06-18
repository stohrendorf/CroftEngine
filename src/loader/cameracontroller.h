#pragma once

#include "level.h"
#include "inputstate.h"
#include "core/angle.h"

class CameraController final : public irr::scene::ISceneNodeAnimator
{
private:
    bool m_firstUpdate = true;
    irr::gui::ICursorControl* m_cursorControl;
    irr::core::vector2df m_prevCursorPos;
    irr::core::vector2df m_currentCursorPos;
    irr::u32 m_lastAnimationTime = 0;
    bool m_firstInput = true;
    core::Angle m_rotateSpeed{ 100_deg };

    bool m_forward = false;
    bool m_backward = false;
    bool m_left = false;
    bool m_right = false;
    bool m_stepLeft = false;
    bool m_stepRight = false;

    InputState m_inputState;

    const loader::Level* m_level;
    LaraController* m_laraController;
    
    int m_lookAtYOffset = 1024;
    int m_distanceFromLookAt = 1536;
    int m_smoothFactor = 6; // 12
    irr::core::vector3df m_currentLookAt;
    irr::core::vector3d<core::Angle> m_localRotation;
    irr::core::vector3df m_currentPosition;
    
#ifndef NDEBUG
    irr::video::IVideoDriver* m_driver;
#endif

public:
#ifndef NDEBUG
    explicit CameraController(irr::gui::ICursorControl* cursorControl, const loader::Level* level, LaraController* laraController, irr::video::IVideoDriver* drv);
#else
    explicit TRCameraSceneNodeAnimator(irr::gui::ICursorControl* cursorControl, const loader::Level* level, LaraStateHandler* laraController);
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
    
    const loader::Level* getLevel() const noexcept
    {
        return m_level;
    }

    void setLocalRotation(core::Angle x, core::Angle y);
    void setLocalRotationX(core::Angle x);
    void setLocalRotationY(core::Angle y);

private:
    void tracePortals(irr::scene::ICameraSceneNode* camera);
    bool clampY(const irr::core::vector3df& lookAt, irr::core::vector3df& origin, const loader::Sector* sector) const;

    enum class ClampType
    {
        Outer,
        Inner,
        None
    };

    ClampType clampX(const irr::core::vector3df& lookAt, irr::core::vector3df& origin) const;
    ClampType clampZ(const irr::core::vector3df& lookAt, irr::core::vector3df& origin) const;
    bool clamp(const irr::core::vector3df& lookAt, irr::core::vector3df& origin) const;
};
