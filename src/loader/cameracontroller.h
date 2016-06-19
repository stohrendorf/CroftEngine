#pragma once

#include "level.h"
#include "inputstate.h"
#include "core/angle.h"

class CameraController final : public irr::scene::ISceneNodeAnimator
{
private:
    // Internals
    bool m_firstUpdate = true;
    irr::gui::ICursorControl* m_cursorControl;
    irr::core::vector2df m_prevCursorPos;
    irr::core::vector2df m_currentCursorPos;
    irr::u32 m_lastAnimationTime = 0;
    bool m_firstInput = true;

    // Input
    bool m_forward = false;
    bool m_backward = false;
    bool m_left = false;
    bool m_right = false;
    bool m_stepLeft = false;
    bool m_stepRight = false;

    InputState m_inputState;

    // For interactions
    const loader::Level* m_level;
    LaraController* m_laraController;
    
    // TR state
    int m_lookAtYOffset = 1024;
    int m_distanceFromLookAt = 1536;
    int m_smoothFactor = 8;
    loader::ExactTRCoordinates m_currentLookAt;
    irr::core::vector3d<core::Angle> m_localRotation;
    loader::ExactTRCoordinates m_currentPosition;
    
    irr::video::IVideoDriver* m_driver;

public:
    explicit CameraController(gsl::not_null<irr::gui::ICursorControl*> cursorControl, gsl::not_null<const loader::Level*> level, gsl::not_null<LaraController*> laraController, gsl::not_null<irr::video::IVideoDriver*> drv);

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
    void tracePortals(gsl::not_null<irr::scene::ICameraSceneNode*>camera);
    bool clampY(const loader::ExactTRCoordinates& lookAt, loader::ExactTRCoordinates& origin, gsl::not_null<const loader::Sector*>sector) const;

    enum class ClampType
    {
        Normal,
        Edge,
        None
    };

    ClampType clampX(const loader::ExactTRCoordinates& lookAt, loader::ExactTRCoordinates& origin) const;
    ClampType clampZ(const loader::ExactTRCoordinates& lookAt, loader::ExactTRCoordinates& origin) const;
    bool clamp(const loader::ExactTRCoordinates& lookAt, loader::ExactTRCoordinates& origin) const;
    void applyPosition(gsl::not_null<irr::scene::ICameraSceneNode*>camera, uint32_t localTime);
};
