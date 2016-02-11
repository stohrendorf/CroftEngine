#pragma once


#include "util/helpers.h"
#include "world/core/frustum.h"

#include <irrlicht.h>

#include <memory>
/*
  ======================
        CAM TARGET
  ======================
            ^
           (N)
  ----------------------
  |  ??  |  01  |  ??  |
  ----------------------
  |  02  | LARA |  03  |
  ----------------------
  |  ??  |  00  |  ??  |
  ----------------------

*/

namespace world
{
class Room;

namespace core
{
struct Polygon;
class Frustum;
} // namespace core

enum class CameraTarget
{
    Back,
    Front,
    Left,
    Right
};

namespace
{
const irr::f32 CameraRotationSpeed = irr::core::degToRad(1.0f);
} // anonymous namespace

constexpr float MaxShakeDistance = 8192;
constexpr float DefaultShakePower = 100;

class Camera
{
private:
    irr::core::vector3df m_position{ 0,0,0 };
    irr::core::vector3df m_viewDir;
    irr::core::vector3df m_rightDir;
    irr::core::vector3df m_upDir;

    irr::core::vector3df m_prevPos{ 0,0,0 };            // previous camera position

    irr::core::matrix4 m_view;
    irr::core::matrix4 m_projection;
    irr::core::matrix4 m_viewProjection;

    core::Frustum m_frustum;

    irr::f32 m_nearClipping = 1;
    irr::f32 m_farClipping = 65536;

    irr::f32 m_fov = 75;
    irr::f32 m_width;
    irr::f32 m_height;

    irr::f32 m_shakeValue = 0;
    util::Duration m_shakeTime{ 0 };

    CameraTarget m_targetDir = CameraTarget::Front; //Target rotation direction

    Room* m_currentRoom = nullptr;

    irr::core::vector3df m_angles{ 0,0,0 };

public:
    const irr::core::vector3df& getPosition() const noexcept
    {
        return m_position;
    }

    irr::core::vector3df getMovement() const noexcept
    {
        return m_position - m_prevPos;
    }

    void resetMovement() noexcept
    {
        m_prevPos = m_position;
    }

    void setPosition(const irr::core::vector3df& pos) noexcept
    {
        m_position = pos;
    }

    const irr::core::vector3df& getViewDir() const noexcept
    {
        return m_viewDir;
    }

    const irr::core::vector3df& getRightDir() const noexcept
    {
        return m_rightDir;
    }

    const irr::core::vector3df& getUpDir() const noexcept
    {
        return m_upDir;
    }

    const irr::core::matrix4& getProjection() const noexcept
    {
        return m_projection;
    }

    const irr::core::matrix4& getView() const noexcept
    {
        return m_view;
    }

    const irr::core::matrix4& getViewProjection() const noexcept
    {
        return m_viewProjection;
    }

    Room* getCurrentRoom() noexcept
    {
        return m_currentRoom;
    }

    const Room* getCurrentRoom() const noexcept
    {
        return m_currentRoom;
    }

    void setCurrentRoom(Room* room) noexcept
    {
        m_currentRoom = room;
    }

    const core::Frustum& getFrustum() const noexcept
    {
        return m_frustum;
    }

    CameraTarget getTargetDir() const noexcept
    {
        return m_targetDir;
    }

    void setTargetDir(CameraTarget target) noexcept
    {
        m_targetDir = target;
    }

    irr::f32 getShakeValue() const noexcept
    {
        return m_shakeValue;
    }

    const util::Duration& getShakeTime() const noexcept
    {
        return m_shakeTime;
    }

    void setShakeTime(const util::Duration& d) noexcept
    {
        m_shakeTime = d;
    }

    const irr::core::vector3df& getAngles() const noexcept
    {
        return m_angles;
    }

    void shake(irr::f32 currentAngle, irr::f32 targetAngle, const util::Duration& frameTime)
    {
        constexpr irr::f32 RotationSpeed = 2.0; //Speed of rotation

        irr::f32 d_angle = m_angles.X - targetAngle;
        if(d_angle > util::Rad90)
        {
            d_angle -= CameraRotationSpeed;
        }
        if(d_angle < -util::Rad90)
        {
            d_angle += CameraRotationSpeed;
        }
        m_angles.X = std::fmod(m_angles.X + std::atan2(std::sin(currentAngle - d_angle), std::cos(currentAngle + d_angle)) * util::toSeconds(frameTime) * RotationSpeed, util::Rad360);
    }

    Camera();
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    void apply();
    void setFovAspect(irr::f32 fov, irr::f32 aspect);
    void moveAlong(irr::f32 dist);
    void moveStrafe(irr::f32 dist);
    void moveVertical(irr::f32 dist);
    void move(const irr::core::vector3df& v)
    {
        moveAlong(v.Z);
        moveStrafe(v.X);
        moveVertical(v.Y);
    }

    void shake(irr::f32 power, util::Duration time);
    void applyRotation();
    void rotate(const irr::core::vector3df& v)
    {
        m_angles += v;
    }
};

// Static camera / sink structure.
// In original engines, static cameras and sinks shared the same structure,
// albeit with different field meanings. In compiled level, it is unfortunately
// impossible to tell camera from sink, so the only way is to share the struct
// between these two types of objects.
// Thanks to b122251 for extra info describing this structure.

struct StatCameraSink
{
    irr::core::vector3df position;
    uint16_t room_or_strength;   // Room for camera, strength for sink.
    uint16_t flag_or_zone;       // Flag for camera, zone for sink.
};

// Flyby camera structure.

struct FlybyCamera
{
    irr::core::vector3df position;
    irr::core::vector3df rotation;

    irr::f32 fov;
    irr::f32 roll;
    irr::f32 speed;

    uint32_t    sequence;   // Sequence number to which camera belongs
    uint32_t    index;      // Index in sequence
    uint32_t    timer;      // How much to sit there
    uint32_t    room_id;

    uint16_t    flags;      // See TRLE manual
};
} // namespace world
