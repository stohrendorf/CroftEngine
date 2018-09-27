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

    //! @brief Global camera position.
    core::RoomBoundPosition m_eye;
    //! @brief The point the camera moves around.
    core::RoomBoundPosition m_center;
    CameraMode m_mode = CameraMode::Chase;

    //! @brief Additional height of the camera above the real position.
    int m_cameraYOffset = 0;

    CameraMode m_oldMode = CameraMode::Chase;

    bool m_tracking = false;

    // int frameCount

    /**
     * @brief If <0, bounce randomly around +/- @c m_bounce/2, increasing value by 5 each frame; if >0, do a single Y bounce downwards by @c m_bounce.
     */
    int m_bounce = 0;

    // int underwater

    //! @brief Goal distance between the pivot point and the camera.
    int m_eyeCenterDistance = 1536;
    //! @brief Floor-projected pivot distance, squared.
    int m_eyeCenterHorizontalDistanceSq = 0;

    core::TRRotation m_eyeRotation;

    //! @brief Global camera rotation.
    core::TRRotation m_currentRotation;

    //! @brief An item to point the camera to.
    //! @note Also modifies Lara's head and torso rotation.
    std::shared_ptr<items::ItemNode> m_item = nullptr;
    std::shared_ptr<const items::ItemNode> m_lastItem = nullptr;
    std::shared_ptr<items::ItemNode> m_enemy = nullptr;
    //! @brief Movement smoothness for adjusting the pivot position.
    int m_trackingSmoothness = 8;
    int m_fixedCameraId = -1;
    int m_currentFixedCameraId = -1;
    int m_camOverrideTimeout{-1};

    std::weak_ptr<audio::SourceHandle> m_underwaterAmbience;

public:
    explicit CameraController(const gsl::not_null<level::Level*>& level,
                              gsl::not_null<std::shared_ptr<gameplay::Camera>> camera);

    explicit CameraController(gsl::not_null<level::Level*> level,
                              gsl::not_null<std::shared_ptr<gameplay::Camera>> camera,
                              bool noLaraTag);

    const level::Level* getLevel() const noexcept
    {
        return m_level;
    }

    void setCurrentRotation(core::Angle x, core::Angle y);

    void setCurrentRotationX(core::Angle x);

    void setCurrentRotationY(core::Angle y);

    void setEyeRotation(const core::Angle x, const core::Angle y)
    {
        m_eyeRotation.X = x;
        m_eyeRotation.Y = y;
    }

    const core::TRRotation& getEyeRotation() const
    {
        return m_eyeRotation;
    }

    void setEyeCenterDistance(const int d)
    {
        m_eyeCenterDistance = d;
    }

    void setOldMode(const CameraMode k)
    {
        m_oldMode = k;
    }

    void setCamOverride(const floordata::CameraParameters& camParams,
                        uint16_t camId,
                        floordata::SequenceCondition condition,
                        bool fromHeavy,
                        uint16_t activationRequest,
                        bool switchIsOn);

    void setItem(const std::shared_ptr<items::ItemNode>& item)
    {
        if( item == nullptr || (m_mode != CameraMode::Fixed && m_mode != CameraMode::Heavy) )
            return;

        m_item = item;
    }

    void findItem(const uint16_t* cmdSequence);

    void update();

    void setMode(const CameraMode t)
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

    const core::RoomBoundPosition& getCenter() const
    {
        return m_center;
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
        return m_eye.room;
    }

    void setPosition(const core::TRVec& p)
    {
        m_eye.position = p;
    }

    const core::RoomBoundPosition& getTRPosition() const
    {
        return m_eye;
    }

    void setPosition(const core::RoomBoundPosition& p)
    {
        m_eye = p;
    }

    /**
     * @brief Clamps a point between two endpoints if there is a floordata-defined obstacle
     * @param[in] start Starting point
     * @param[in] end Destination of the movement, clamped if necessary
     * @param[in] level For accessing boxes and floordata
     * @retval false if clamped
     *
     * @warning Please be aware that the return value is reverted and not what you might expect...
     */
    static bool clampPosition(const core::RoomBoundPosition& start,
                              core::RoomBoundPosition& end,
                              const level::Level& level);

    void setBounce(const int bounce)
    {
        m_bounce = bounce;
    }

    const gsl::not_null<std::shared_ptr<gameplay::Camera>>& getCamera() const
    {
        return m_camera;
    }

    void updateCinematic(const loader::CinematicFrame& frame, bool ingame);

    YAML::Node save() const;

    void load(const YAML::Node& n);

    size_t m_cinematicFrame = 0;
    core::TRVec m_cinematicPos{0, 0, 0};
    core::TRRotation m_cinematicRot{0_deg, 0_deg, 0_deg};

private:
    void tracePortals();

    static bool clampY(const core::TRVec& start,
                       core::TRVec& end,
                       const gsl::not_null<const loader::Sector*>& sector,
                       const level::Level& level);

    enum class ClampType
    {
        Ceiling,
        Wall,
        None
    };

    static ClampType clampAlongX(const core::RoomBoundPosition& start,
                                 core::RoomBoundPosition& end,
                                 const level::Level& level);

    static ClampType clampAlongZ(const core::RoomBoundPosition& start,
                                 core::RoomBoundPosition& end,
                                 const level::Level& level);

    void handleCamOverride();

    int moveIntoGeometry(core::RoomBoundPosition& pos, int margin) const;

    bool isVerticallyOutsideRoom(const core::TRVec& pos, const gsl::not_null<const loader::Room*>& room) const;

    void updatePosition(const core::RoomBoundPosition& eyePositionGoal, int smoothFactor);

    void doUsualMovement(const gsl::not_null<std::shared_ptr<const items::ItemNode>>& item);

    void handleFreeLook(const items::ItemNode& item);

    void handleEnemy(const items::ItemNode& item);

    using ClampCallback = void(int& current1, int& current2, int target1, int target2, int lowLimit1, int lowLimit2,
                               int highLimit1, int highLimit2);

    void clampBox(core::RoomBoundPosition& eyePositionGoal, const std::function<ClampCallback>& callback) const;

    static void freeLookClamp(int& currentFrontBack, int& currentLeftRight, int targetFrontBack, int targetLeftRight,
                              int back, int right, int front, int left);

    static void clampToCorners(int targetHorizontalDistanceSq,
                               int& currentFrontBack, int& currentLeftRight, int targetFrontBack, int targetLeftRight,
                               int back, int right, int front, int left);
};
}
