#pragma once

#include "core/angle.h"
#include "loader/datatypes.h"
#include "audio/soundengine.h"

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
    Chase,
    Fixed,
    FreeLook,
    Combat,
    Cinematic,
    Heavy
};

enum class CameraModifier
{
    None,
    FollowCenter,
    AllowSteepSlants,
    Chase
};


class CameraController final : public audio::Listener
{
private:
    gsl::not_null<std::shared_ptr<gameplay::Camera>> m_camera;

    level::Level* m_level;

    //! @brief Global camera position.
    core::RoomBoundPosition m_eye;
    //! @brief The point the camera moves around.
    core::RoomBoundPosition m_center;
    CameraMode m_mode = CameraMode::Chase;

    //! @brief Additional height of the camera above the real position.
    core::Length m_eyeYOffset = 0_len;

    CameraModifier m_modifier = CameraModifier::None;

    bool m_fixed = false;

    /**
     * @brief If <0, bounce randomly around +/- @c m_bounce/2, increasing value by 5 each frame; if >0, do a single Y bounce downwards by @c m_bounce.
     */
    core::Length m_bounce = 0_len;

    //! @brief Goal distance between the pivot point and the camera.
    core::Length m_eyeCenterDistance = core::DefaultCameraLaraDistance;
    //! @brief Floor-projected pivot distance, squared.
    core::Area m_eyeCenterHorizontalDistanceSq{0};

    core::TRRotation m_eyeRotation;

    //! @brief Global camera rotation.
    core::TRRotation m_rotationAroundCenter;

    //! @brief An item to point the camera to.
    //! @note Also modifies Lara's head and torso rotation.
    std::shared_ptr<items::ItemNode> m_item = nullptr;
    std::shared_ptr<const items::ItemNode> m_previousItem = nullptr;
    std::shared_ptr<items::ItemNode> m_enemy = nullptr;
    //! @brief Movement smoothness for adjusting the pivot position.
    int m_smoothness = 8;
    int m_fixedCameraId = -1;
    int m_currentFixedCameraId = -1;
    core::Frame m_camOverrideTimeout{-1_frame};

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

    void setRotationAroundCenter(const core::Angle x, const core::Angle y);

    void setRotationAroundCenterX(const core::Angle x);

    void setRotationAroundCenterY(const core::Angle y);

    void setEyeRotation(const core::Angle x, const core::Angle y)
    {
        m_eyeRotation.X = x;
        m_eyeRotation.Y = y;
    }

    const core::TRRotation& getEyeRotation() const
    {
        return m_eyeRotation;
    }

    void setEyeCenterDistance(const core::Length d)
    {
        m_eyeCenterDistance = d;
    }

    void setModifier(const CameraModifier k)
    {
        m_modifier = k;
    }

    void setCamOverride(const floordata::CameraParameters& camParams,
                        uint16_t camId,
                        floordata::SequenceCondition condition,
                        bool fromHeavy,
                        core::Frame timeout,
                        bool switchIsOn);

    void setLookAtItem(const std::shared_ptr<items::ItemNode>& item)
    {
        if( item != nullptr && (m_mode == CameraMode::Fixed || m_mode == CameraMode::Heavy) )
            m_item = item;
    }

    void handleCommandSequence(const engine::floordata::FloorDataValue* cmdSequence);

    void update();

    void setMode(const CameraMode t)
    {
        m_mode = t;
    }

    CameraMode getMode() const noexcept
    {
        return m_mode;
    }

    glm::vec3 getPosition() const override
    {
        return glm::vec3{m_camera->getInverseViewMatrix()[3]};
    }

    const core::RoomBoundPosition& getCenter() const
    {
        return m_center;
    }

    glm::vec3 getFrontVector() const override
    {
        auto rs = m_camera->getInverseViewMatrix();
        rs[3].x = rs[3].y = rs[3].z = 0; // zero out translation component
        return glm::vec3{rs * glm::vec4{0, 0, -1, 1}};
    }

    glm::vec3 getUpVector() const override
    {
        auto rs = m_camera->getInverseViewMatrix();
        rs[3].x = rs[3].y = rs[3].z = 0; // zero out translation component
        return glm::vec3{rs * glm::vec4{0, 1, 0, 1}};
    }

    const gsl::not_null<const loader::Room*>& getCurrentRoom() const
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

    void setBounce(const core::Length& bounce)
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
    core::TRVec m_cinematicPos{0_len, 0_len, 0_len};
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

    void handleFixedCamera();

    core::Length moveIntoGeometry(core::RoomBoundPosition& pos, const core::Length& margin) const;

    bool isVerticallyOutsideRoom(const core::TRVec& pos, const gsl::not_null<const loader::Room*>& room) const;

    void updatePosition(const core::RoomBoundPosition& eyePositionGoal, int smoothFactor);

    void chaseItem(const gsl::not_null<std::shared_ptr<const items::ItemNode>>& item);

    void handleFreeLook(const items::ItemNode& item);

    void handleEnemy(const items::ItemNode& item);

    using ClampCallback = void(core::Length& current1,
                               core::Length& current2,
                               core::Length target1,
                               core::Length target2,
                               core::Length lowLimit1,
                               core::Length lowLimit2,
                               core::Length highLimit1,
                               core::Length highLimit2);

    void clampBox(core::RoomBoundPosition& eyePositionGoal, const std::function<ClampCallback>& callback) const;

    static void freeLookClamp(core::Length& currentFrontBack,
                              core::Length& currentLeftRight,
                              core::Length targetFrontBack,
                              core::Length targetLeftRight,
                              core::Length back,
                              core::Length right,
                              core::Length front,
                              core::Length left);

    static void clampToCorners(core::Area targetHorizontalDistanceSq,
                               core::Length& currentFrontBack,
                               core::Length& currentLeftRight,
                               core::Length targetFrontBack,
                               core::Length targetLeftRight,
                               core::Length back,
                               core::Length right,
                               core::Length front,
                               core::Length left);
};
}
