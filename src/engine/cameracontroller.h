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


    enum class CameraMode
    {
        Chase, // 0
        Fixed,
        FreeLook, // 2
        Combat,
        Cinematic,
        Heavy
    };


    class CameraController final
    {
    private:
        // Internals
        gsl::not_null<std::shared_ptr<gameplay::Camera>> m_camera;

        // For interactions
        level::Level* m_level;
        LaraNode* m_laraController;

        //! @brief Global camera position.
        core::RoomBoundPosition m_position;
        //! @brief The point the camera moves around.
        core::RoomBoundPosition m_target;
        CameraMode m_mode = CameraMode::Chase;

        //! @brief Additional height of the camera above the real position.
        int m_cameraYOffset = 0;

        CameraMode m_oldMode = CameraMode::Chase;

        bool m_tracking = false;

        // int frameCount

        int m_bounce = 0;

        // int underwater

        //! @brief Goal distance between the pivot point and the camera.
        int m_targetDistance = 1536;
        //! @brief Floor-projected pivot distance, squared.
        int m_targetDistanceSq = 0;

        core::TRRotation m_targetRotation;

        //! @brief Global camera rotation.
        core::TRRotation m_currentRotation;

        //! @brief An item to point the camera to.
        //! @note Also modifies Lara's head and torso rotation.
        items::ItemNode* m_item = nullptr;
        const items::ItemNode* m_lastItem = nullptr;
        items::ItemNode* m_enemy = nullptr;
        //! @brief Movement smothness for adjusting the pivot position.
        int m_trackingSmoothness = 8;
        int m_fixedCameraId = -1;
        int m_currentFixedCameraId = -1;
        int m_camOverrideTimeout{-1};

        std::shared_ptr<audio::SourceHandle> m_underwaterAmbience;

    public:
        explicit CameraController(gsl::not_null<level::Level*> level, gsl::not_null<LaraNode*> laraController, const gsl::not_null<std::shared_ptr<gameplay::Camera>>& camera);


        const level::Level* getLevel() const noexcept
        {
            return m_level;
        }


        void setCurrentRotation(core::Angle x, core::Angle y);
        void setCurrentRotationX(core::Angle x);
        void setCurrentRotationY(core::Angle y);


        void setTargetDistance(int d)
        {
            m_targetDistance = d;
        }


        void setOldMode(CameraMode k)
        {
            m_oldMode = k;
        }


        void setCamOverride(const floordata::CameraParameters& camParams, uint16_t camId, floordata::SequenceCondition condition, bool isDoppelganger, uint16_t activationRequest, bool switchIsOn);


        void setItem(items::ItemNode* item)
        {
            if( item == nullptr || (m_mode != CameraMode::Fixed && m_mode != CameraMode::Heavy) )
                return;

            m_item = item;
        }


        void findItem(const uint16_t* floorData);

        void update();


        void setMode(CameraMode t)
        {
            m_mode = t;
        }


        CameraMode getMode() const noexcept
        {
            return m_mode;
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


        const loader::Room* getCurrentRoom() const
        {
            return m_position.room;
        }

        /**
         * @brief Clamps a point between two endpoints if there is a floordata-defined obstacle
         * @param[in,out] origin Starting point, moved towards @a target
         * @param[in] target Destination of the movement, possibly never reached
         * @param[in] level For accessing boxes and floordata
         * @retval false if clamped
         * 
         * @warning Please be aware that the return value is reverted and not what you might expect...
         */
        static bool clampPosition(core::RoomBoundPosition& origin, const core::RoomBoundPosition& target, const level::Level& level);

    private:
        void tracePortals();
        static bool clampY(const core::TRCoordinates& lookAt, core::TRCoordinates& origin, gsl::not_null<const loader::Sector*> sector, const level::Level& level);


        enum class ClampType
        {
            Vertical,
            Horizontal,
            None
        };


        static ClampType clampAlongX(core::RoomBoundPosition& origin, const core::RoomBoundPosition& target, const level::Level& level);
        static ClampType clampAlongZ(core::RoomBoundPosition& origin, const core::RoomBoundPosition& target, const level::Level& level);

        void handleCamOverride();
        int moveIntoGeometry(core::RoomBoundPosition& pos, int margin) const;
        bool isVerticallyOutsideRoom(const core::TRCoordinates& pos, const gsl::not_null<const loader::Room*>& room) const;
        void updatePosition(const core::RoomBoundPosition& position, int smoothFactor);
        void doUsualMovement(const gsl::not_null<const items::ItemNode*>& item);
        void handleFreeLook(const items::ItemNode& item);
        void handleEnemy(const items::ItemNode& item);

        using ClampCallback = void(int& current1, int& current2, int target1, int target2, int lowLimit1, int lowLimit2, int highLimit1, int highLimit2);

        void clampBox(core::RoomBoundPosition& camTargetPos, const std::function<ClampCallback>& callback) const;
        static void freeLookClamp(int& currentFrontBack, int& currentLeftRight, int targetFrontBack, int targetLeftRight, int back, int right, int front, int left);
        static void clampToCorners(const int lookAtDistanceSq, int& currentFrontBack, int& currentLeftRight, int targetFrontBack, int targetLeftRight, int back, int right, int front, int left);
    };
}
