#pragma once

#include "core/angle.h"
#include "loader/datatypes.h"
#include "loader/floordata.h"
#include "audio/sourcehandle.h"

namespace engine
{
    class ItemController;
    class LaraController;

    class CameraController final
    {
    private:
        // Internals
        bool m_firstUpdate = true;
        uint32_t m_lastAnimationTime = 0;
        bool m_firstInput = true;
        gsl::not_null<gameplay::Camera*> m_camera;

        // For interactions
        level::Level* m_level;
        LaraController* m_laraController;

        // TR state
        ItemController* m_lookAtItem = nullptr;
        const ItemController* m_lookAtItem2 = nullptr;
        ItemController* m_enemy = nullptr;
        core::TRRotation m_enemyLookRot;
        int m_unknown1 = 0;
        int m_camShakeRadius = 0;
        int m_currentYOffset = 0;
        int m_distanceFromLookAt = 1536;
        int m_smoothFactor = 8;
        int m_camOverrideId = -1;
        int m_activeCamOverrideId = -1;
        int m_camOverrideTimeout = -1;
        int m_camOverrideType = 0;
        core::RoomBoundIntPosition m_currentLookAt;
        core::TRRotation m_localRotation;
        core::RoomBoundIntPosition m_currentPosition;
        bool m_lookingAtSomething = false;
        long m_lookAtDistanceSq = 0;

        // hacks
        core::TRRotation m_headRotation;
        core::TRRotation m_torsoRotation;

        std::shared_ptr<audio::SourceHandle> m_underwaterAmbience;

    public:
        explicit CameraController(gsl::not_null<level::Level*> level, gsl::not_null<LaraController*> laraController, const gsl::not_null<gameplay::Camera*>& camera);

        void animateNode(uint32_t timeMs);

        const level::Level* getLevel() const noexcept
        {
            return m_level;
        }

        void setLocalRotation(core::Angle x, core::Angle y);
        void setLocalRotationX(core::Angle x);
        void setLocalRotationY(core::Angle y);
        void setLocalDistance(int d)
        {
            m_distanceFromLookAt = d;
        }

        void setUnknown1(int k)
        {
            m_unknown1 = k;
        }

        void setCamOverride(uint16_t floorData, uint16_t camId, loader::TriggerType triggerType, bool isDoppelganger, uint16_t triggerArg, bool switchIsOn);

        void setLookAtItem(ItemController* item)
        {
            if( item == nullptr || (m_camOverrideType != 1 && m_camOverrideType != 5) )
                return;

            m_lookAtItem = item;
        }

        void findCameraTarget(const loader::FloorData::value_type* floorData);

        void update(const std::chrono::microseconds& deltaTimeMs);

        void setCamOverrideType(int t)
        {
            m_camOverrideType = t;
        }

        int getCamOverrideType() const noexcept
        {
            return m_camOverrideType;
        }

        void addHeadRotationXY(const core::Angle& x, const core::Angle& y)
        {
            m_headRotation.X += x;
            m_headRotation.Y += y;
        }

        const core::TRRotation& getHeadRotation() const noexcept
        {
            return m_headRotation;
        }

        void setTorsoRotation(const core::TRRotation& r)
        {
            m_torsoRotation = r;
        }

        void setHeadRotation(const core::TRRotation& r)
        {
            m_headRotation = r;
        }

        const core::TRRotation& getTorsoRotation() const noexcept
        {
            return m_torsoRotation;
        }

        gameplay::Vector3 getPosition() const
        {
            gameplay::Vector3 tmp;
            m_camera->getViewMatrix().getTranslation(&tmp);
            return tmp;
        }

        gameplay::Vector3 getFrontVector() const
        {
            gameplay::Vector3 tmp;
            m_camera->getViewMatrix().getForwardVector(&tmp);
            return tmp;
        }

        gameplay::Vector3 getUpVector() const
        {
            gameplay::Vector3 tmp;
            m_camera->getViewMatrix().getUpVector(&tmp);
            return tmp;
        }

        void resetHeadTorsoRotation()
        {
            m_headRotation = { 0_deg, 0_deg, 0_deg };
            m_torsoRotation = { 0_deg, 0_deg, 0_deg };
        }

    private:
        void tracePortals();
        bool clampY(const core::TRCoordinates& lookAt, core::TRCoordinates& origin, gsl::not_null<const loader::Sector*> sector) const;

        enum class ClampType
        {
            Vertical,
            Horizonal,
            None
        };

        ClampType clampAlongX(core::RoomBoundIntPosition& origin) const;
        ClampType clampAlongZ(core::RoomBoundIntPosition& origin) const;
        bool clampPosition(core::RoomBoundIntPosition& origin) const;

        void handleCamOverride(int deltaTimeMs);
        int moveIntoGeometry(core::RoomBoundIntPosition& pos, int margin) const;
        bool isVerticallyOutsideRoom(const core::TRCoordinates& pos, const gsl::not_null<const loader::Room*>& room) const;
        void updatePosition(const core::RoomBoundIntPosition& position, int smoothFactor, int deltaTimeMs);
        void doUsualMovement(const gsl::not_null<const ItemController*>& item, int deltaTimeMs);
        void handleFreeLook(const ItemController& item, int deltaTimeMs);
        void handleEnemy(const ItemController& item, int deltaTimeMs);

        using ClampCallback = void(long& current1, long& current2, long target1, long target2, long lowLimit1, long lowLimit2, long highLimit1, long highLimit2);

        void clampBox(core::RoomBoundIntPosition& camTargetPos, const std::function<ClampCallback>& callback) const;
        static void freeLookClamp(long& currentFrontBack, long& currentLeftRight, long targetFrontBack, long targetLeftRight, long back, long right, long front, long left);
        static void clampToCorners(const long lookAtDistanceSq, long& currentFrontBack, long& currentLeftRight, long targetFrontBack, long targetLeftRight, long back, long right, long front, long left);
    };
}
