#include "camera.h"

#include "util/vmath.h"
#include "world/core/frustum.h"

#include <cmath>

namespace world
{
void Camera::apply()
{
    m_projection.buildProjectionMatrixPerspectiveFovRH(irr::core::degToRad(m_fov), static_cast<irr::f32>(m_width)/m_height, m_nearClipping, m_farClipping);

    m_view.buildCameraLookAtMatrixRH(m_position, m_position + getViewDir(), getUpDir());

    m_viewProjection = m_projection * m_view;

    m_frustum.setFromMatrix(m_viewProjection);
}

void Camera::setFovAspect(irr::f32 fov, irr::f32 aspect)
{
    m_fov = fov;
    m_height = 2.0f * m_nearClipping * std::tan(irr::core::degToRad(m_fov) / 2);
    m_width = m_height * aspect;
}

void Camera::moveAlong(irr::f32 dist)
{
    m_position += getViewDir() * dist;
}

void Camera::moveStrafe(irr::f32 dist)
{
    m_position += getRightDir() * dist;
}

void Camera::moveVertical(irr::f32 dist)
{
    m_position += getUpDir() * dist;
}

void Camera::shake(irr::f32 power, util::Duration time)
{
    m_shakeValue = power;
    m_shakeTime = time;
}

void Camera::applyRotation()
{
    irr::core::quaternion q;
    q *= irr::core::quaternion().fromAngleAxis(m_angles.Z, { 0,1,0 });
    q *= irr::core::quaternion().fromAngleAxis(m_angles.X, { 0,0,1 });
    q *= irr::core::quaternion().fromAngleAxis(m_angles.Y, { 1,0,0 });
    m_axes = q.getMatrix();
}

Camera::Camera()
{
    m_width = m_height = 2.0f * m_nearClipping * std::tan(irr::core::degToRad(m_fov) / 2);

    apply();
}
} // namespace world
