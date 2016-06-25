#pragma once

#include "level.h"
#include "inputstate.h"
#include "core/angle.h"

class ItemController;

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
    gsl::not_null<irr::scene::ICameraSceneNode*> m_camera;

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
    const ItemController* m_lookAtItem = nullptr;
    const ItemController* m_lookAtItem2 = nullptr;
    ItemController* m_enemy = nullptr;
    irr::core::vector3d<core::Angle> m_enemyLookRot;
    int m_unknown1 = 0;
    int m_camShakeRadius = 0;
    int m_lookAtY = 0;
    int m_distanceFromLookAt = 1536;
    int m_smoothFactor = 8;
    int m_camOverrideId = -1;
    int m_activeCamOverrideId = -1;
    int m_camOverrideTimeout = -1;
    int m_camOverrideType = 0;
    loader::RoomBoundPosition m_currentLookAt;
    irr::core::vector3d<core::Angle> m_localRotation;
    loader::RoomBoundPosition m_currentPosition;
    bool m_lookingAtSomething = false;
    float m_lookAtDistanceSq = 0;
    
    // hacks
    irr::core::vector3d<core::Angle> m_headRotation;
    irr::core::vector3d<core::Angle> m_freeLookRotation;

    irr::video::IVideoDriver* m_driver;

public:
    explicit CameraController(gsl::not_null<irr::gui::ICursorControl*> cursorControl, gsl::not_null<const loader::Level*> level, gsl::not_null<LaraController*> laraController, gsl::not_null<irr::video::IVideoDriver*> drv, const gsl::not_null<irr::scene::ICameraSceneNode*>& camera);

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

    void setCamOverride(uint16_t floorData, uint16_t camId, loader::TriggerType triggerType, bool isDoppelganger, uint16_t triggerArg, bool switchIsOn);
    void setLookAtItem(const ItemController* item)
    {
        if(item == nullptr || (m_camOverrideType != 1 && m_camOverrideType != 5))
            return;

        m_lookAtItem = item;
    }

    void findCameraTarget(loader::TriggerType triggerType, const loader::FloorData::value_type* floorData)
    {
        if(m_camOverrideType == 5)
            return;

        int type = 2;
        while(true)
        {
            const bool isLast = loader::isLastFloordataEntry(*floorData);
            const auto triggerFunc = loader::extractTriggerFunction(*floorData);
            const auto param = loader::extractTriggerFunctionParam(*floorData);

            ++floorData;

            
            if(triggerFunc == loader::TriggerFunction::LookAt && m_camOverrideType != 2 && m_camOverrideType != 3)
            {
                m_lookAtItem = m_level->getItemController(param);
            }
            else if(triggerFunc == loader::TriggerFunction::CameraTarget)
            {
                ++floorData;

                if(param != m_activeCamOverrideId)
                {
                    type = 0;
                }
                else
                {
                    m_camOverrideId = m_activeCamOverrideId;
                    if(m_camOverrideTimeout >= 0 && m_camOverrideType != 2 && m_camOverrideType != 3)
                    {
                        type = 1;
                        m_camOverrideType = 1;
                    }
                    else
                    {
                        type = 0;
                        m_camOverrideTimeout = -1;
                    }
                }
            }

            if(isLast)
                break;
        }

        if(type == 0 || (type == 2 /** @todo && lookAtItem->flags2 & 0x40 */ && m_lookAtItem != m_lookAtItem2))
            m_lookAtItem = nullptr;
    }

private:
    void tracePortals();
    bool clampY(const loader::ExactTRCoordinates& lookAt, loader::ExactTRCoordinates& origin, gsl::not_null<const loader::Sector*>sector) const;

    enum class ClampType
    {
        Ceiling,
        Wall,
        None
    };

    ClampType clampX(loader::RoomBoundPosition& origin) const;
    ClampType clampZ(loader::RoomBoundPosition& origin) const;
    bool clamp(loader::RoomBoundPosition& origin) const;

    void update(int deltaTimeMs);
    void handleCamOverride(int deltaTimeMs);
    int moveIntoGeometry(loader::RoomBoundPosition& pos, int margin) const;
    bool isVerticallyOutsideRoom(const loader::TRCoordinates& pos, const gsl::not_null<const loader::Room*>& room) const;
    void updatePosition(const loader::RoomBoundPosition& pos, int smoothFactor, int deltaTimeMs);
    void doUsualMovement(const gsl::not_null<const ItemController*>& item, int deltaTimeMs);
    void handleFreeLook(const ItemController& item, int deltaTimeMs);
    void handleEnemy(const ItemController& item, int deltaTimeMs);

    using ClampCallback = void(float& current1, float& current2, float target1, float target2, float lowLimit1, float lowLimit2, float highLimit1, float highLimit2);

    void clampBox(loader::RoomBoundPosition& pos, const std::function<ClampCallback>& callback) const;
    static void freeLookClamp(float& currentFrontBack, float& currentLeftRight, float targetFrontBack, float targetLeftRight, float back, float right, float front, float left);
    static void clampToCorners(const float lookAtDistanceSq, float& currentFrontBack, float& currentLeftRight, float targetFrontBack, float targetLeftRight, float back, float right, float front, float left);
};
