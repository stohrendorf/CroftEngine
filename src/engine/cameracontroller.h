#pragma once

#include "core/angle.h"
#include "loader/datatypes.h"
#include "audio/sourcehandle.h"


namespace engine
{
    namespace floordata
    {
        enum class SequenceCondition;
        class ActivationState;
        struct CameraParameters;
    }


    namespace items
    {
        class ItemNode;
    }


    class LaraNode;


    enum class CamOverrideType
    {
        None, // 0
        NotActivatedByLara,
        FreeLook, // 2
        _3,
        Cinematic,
        ActivatedByLara
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
        //! @brief An item to point the camera to.
        //! @note Also modifies Lara's head and torso rotation.
        items::ItemNode* m_itemOfInterest = nullptr;
        const items::ItemNode* m_previousItemOfInterest = nullptr;
        items::ItemNode* m_enemy = nullptr;
        core::TRRotation m_enemyLookRot;
        CamOverrideType m_unknown1 = CamOverrideType::None;
        int m_camShakeRadius = 0;
        //! @brief Additional height of the camera above the real position.
        int m_cameraYOffset = 0;
        //! @brief Goal distance between the pivot point and the camera.
        int m_pivotDistance = 1536;
        //! @brief Movement smothness for adjusting the pivot position.
        int m_pivotMovementSmoothness = 8;
        int m_camOverrideId = -1;
        int m_activeCamOverrideId = -1;
        int m_camOverrideTimeout{-1};
        CamOverrideType m_camOverrideType = CamOverrideType::None;
        //! @brief The point the camera moves around.
        core::RoomBoundPosition m_pivot;
        //! @brief Global camera rotation.
        core::TRRotation m_globalRotation;
        //! @brief Global camera position.
        core::RoomBoundPosition m_currentPosition;
        bool m_lookingAtSomething = false;
        //! @brief Floor-projected pivot distance, squared.
        long m_flatPivotDistanceSq = 0;

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
            m_pivotDistance = d;
        }


        void setUnknown1(CamOverrideType k)
        {
            m_unknown1 = k;
        }


        void setCamOverride(const floordata::CameraParameters& camParams, uint16_t camId, floordata::SequenceCondition condition, bool isDoppelganger, const floordata::ActivationState& activationRequest, bool switchIsOn);


        void setLookAtItem(items::ItemNode* item)
        {
            if( item == nullptr || (m_camOverrideType != CamOverrideType::NotActivatedByLara && m_camOverrideType != CamOverrideType::ActivatedByLara) )
                return;

            m_itemOfInterest = item;
        }


        void findCameraTarget(const uint16_t* floorData);

        void update();


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
            return glm::vec3{m_camera->getInverseViewMatrix()[3]};
        }


        glm::vec3 getFrontVector() const
        {
            auto rs = m_camera->getInverseViewMatrix();
            rs[3].x = rs[3].y = rs[3].z = 0; // zero out translation component
            return glm::vec3{rs * glm::vec4{0, 0, -1, 1}};
        }


        glm::vec3 getUpVector() const
        {
            auto rs = m_camera->getInverseViewMatrix();
            rs[3].x = rs[3].y = rs[3].z = 0; // zero out translation component
            return glm::vec3{rs * glm::vec4{0, 1, 0, 1}};
        }


        void resetHeadTorsoRotation()
        {
            m_headRotation = {0_deg, 0_deg, 0_deg};
            m_torsoRotation = {0_deg, 0_deg, 0_deg};
        }


        const loader::Room* getCurrentRoom() const
        {
            return m_currentPosition.room;
        }


    private:
        void tracePortals();
        bool clampY(const core::TRCoordinates& lookAt, core::TRCoordinates& origin, gsl::not_null<const loader::Sector*> sector) const;


        enum class ClampType
        {
            Vertical,
            Horizontal,
            None
        };


        ClampType clampAlongX(core::RoomBoundPosition& origin) const;
        ClampType clampAlongZ(core::RoomBoundPosition& origin) const;
        bool clampPosition(core::RoomBoundPosition& origin) const;

        void handleCamOverride();
        int moveIntoGeometry(core::RoomBoundPosition& pos, int margin) const;
        bool isVerticallyOutsideRoom(const core::TRCoordinates& pos, const gsl::not_null<const loader::Room*>& room) const;
        void updatePosition(const ::core::RoomBoundPosition& position, int smoothFactor);
        void doUsualMovement(const gsl::not_null<const items::ItemNode*>& item);
        void handleFreeLook(const items::ItemNode& item);
        void handleEnemy(const items::ItemNode& item);

        using ClampCallback = void(long& current1, long& current2, long target1, long target2, long lowLimit1, long lowLimit2, long highLimit1, long highLimit2);

        void clampBox(core::RoomBoundPosition& camTargetPos, const std::function<ClampCallback>& callback) const;
        static void freeLookClamp(long& currentFrontBack, long& currentLeftRight, long targetFrontBack, long targetLeftRight, long back, long right, long front, long left);
        static void clampToCorners(const long lookAtDistanceSq, long& currentFrontBack, long& currentLeftRight, long targetFrontBack, long targetLeftRight, long back, long right, long front, long left);
    };
}
