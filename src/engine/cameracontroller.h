#pragma once

#include "core/angle.h"
#include "loader/datatypes.h"
#include "loader/floordata.h"
#include "audio/sourcehandle.h"

namespace engine
{
    namespace items
    {
        class ItemNode;
    }

    class LaraNode;

    enum class CamOverrideType
    {
        None, // 0
        _1,
        FreeLook, // 2
        _3,
        _4,
        _5
    };

    class CameraController final
    {
    private:
        // Internals
        gsl::not_null<std::shared_ptr<gameplay::Camera>> m_camera;

        // For interactions
        level::Level* m_level;
        LaraNode* m_laraController;

        // TR state
        items::ItemNode* m_lookAtItem = nullptr;
        const items::ItemNode* m_lookAtItem2 = nullptr;
        items::ItemNode* m_enemy = nullptr;
        core::TRRotation m_enemyLookRot;
        int m_unknown1 = 0;
        int m_camShakeRadius = 0;
        int m_currentYOffset = 0;
        int m_distanceFromLookAt = 1536;
        int m_smoothFactor = 8;
        int m_camOverrideId = -1;
        int m_activeCamOverrideId = -1;
        std::chrono::microseconds m_camOverrideTimeout{ -1 };
        CamOverrideType m_camOverrideType = CamOverrideType::None;
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
        explicit CameraController(gsl::not_null<level::Level*> level, gsl::not_null<LaraNode*> laraController, const gsl::not_null<std::shared_ptr<gameplay::Camera>>& camera);

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

        void setLookAtItem(items::ItemNode* item)
        {
            if( item == nullptr || (m_camOverrideType != CamOverrideType::_1 && m_camOverrideType != CamOverrideType::_5) )
                return;

            m_lookAtItem = item;
        }

        void findCameraTarget(const loader::FloorData::value_type* floorData);

        void update(const std::chrono::microseconds& deltaTimeMs);

        void setCamOverrideType(CamOverrideType t)
        {
            m_camOverrideType = t;
        }

        CamOverrideType getCamOverrideType() const noexcept
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

        glm::vec3 getPosition() const
        {
            return glm::vec3{ m_camera->getInverseViewMatrix()[3] };
        }

        glm::vec3 getFrontVector() const
        {
            auto rs = m_camera->getInverseViewMatrix();
            rs[3].x = rs[3].y = rs[3].z = 0; // zero out translation component
            return glm::vec3{ rs * glm::vec4{0, 0, -1, 1} };
        }

        glm::vec3 getUpVector() const
        {
            auto rs = m_camera->getInverseViewMatrix();
            rs[3].x = rs[3].y = rs[3].z = 0; // zero out translation component
            return glm::vec3{ rs * glm::vec4{ 0, 1, 0, 1 } };
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

        void handleCamOverride(const std::chrono::microseconds& deltaTimeMs);
        int moveIntoGeometry(core::RoomBoundIntPosition& pos, int margin) const;
        bool isVerticallyOutsideRoom(const core::TRCoordinates& pos, const gsl::not_null<const loader::Room*>& room) const;
        void updatePosition(const ::core::RoomBoundIntPosition& position, int smoothFactor, const std::chrono::microseconds& deltaTimeMs);
        void doUsualMovement(const gsl::not_null<const items::ItemNode*>& item, const std::chrono::microseconds& deltaTimeMs);
        void handleFreeLook(const items::ItemNode& item, const std::chrono::microseconds& deltaTimeMs);
        void handleEnemy(const items::ItemNode& item, const std::chrono::microseconds& deltaTimeMs);

        using ClampCallback = void(long& current1, long& current2, long target1, long target2, long lowLimit1, long lowLimit2, long highLimit1, long highLimit2);

        void clampBox(core::RoomBoundIntPosition& camTargetPos, const std::function<ClampCallback>& callback) const;
        static void freeLookClamp(long& currentFrontBack, long& currentLeftRight, long targetFrontBack, long targetLeftRight, long back, long right, long front, long left);
        static void clampToCorners(const long lookAtDistanceSq, long& currentFrontBack, long& currentLeftRight, long targetFrontBack, long targetLeftRight, long back, long right, long front, long left);
    };
}
