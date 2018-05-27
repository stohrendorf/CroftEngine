#include "Camera.h"
#include "Node.h"

#include <glm/gtc/matrix_transform.hpp>

// Camera dirty bits
#define CAMERA_DIRTY_PROJ 2
#define CAMERA_DIRTY_VIEW_PROJ 4
#define CAMERA_DIRTY_INV_VIEW 8
#define CAMERA_DIRTY_INV_VIEW_PROJ 16
#define CAMERA_DIRTY_ALL (CAMERA_DIRTY_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW | CAMERA_DIRTY_INV_VIEW_PROJ)

namespace gameplay
{
Camera::Camera(float fieldOfView, float aspectRatio, float nearPlane, float farPlane)
        : m_fieldOfView( fieldOfView )
        , m_aspectRatio( aspectRatio )
        , m_nearPlane( nearPlane )
        , m_farPlane( farPlane )
        , m_bits( CAMERA_DIRTY_ALL )
{
}

float Camera::getFieldOfView() const
{
    return m_fieldOfView;
}

void Camera::setFieldOfView(float fieldOfView)
{
    m_fieldOfView = fieldOfView;
    m_bits |= CAMERA_DIRTY_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW_PROJ;
}

float Camera::getAspectRatio() const
{
    return m_aspectRatio;
}

float Camera::getNearPlane() const
{
    return m_nearPlane;
}

float Camera::getFarPlane() const
{
    return m_farPlane;
}

const glm::mat4& Camera::getViewMatrix() const
{
    return m_view;
}

const glm::mat4& Camera::getInverseViewMatrix() const
{
    if( m_bits & CAMERA_DIRTY_INV_VIEW )
    {
        m_inverseView = glm::inverse( m_view );

        m_bits &= ~CAMERA_DIRTY_INV_VIEW;
    }

    return m_inverseView;
}

const glm::mat4& Camera::getProjectionMatrix() const
{
    if( m_bits & CAMERA_DIRTY_PROJ )
    {
        m_projection = glm::perspective( m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane );
        m_bits &= ~CAMERA_DIRTY_PROJ;
    }

    return m_projection;
}

const glm::mat4& Camera::getViewProjectionMatrix() const
{
    if( m_bits & CAMERA_DIRTY_VIEW_PROJ )
    {
        m_viewProjection = getProjectionMatrix() * getViewMatrix();

        m_bits &= ~CAMERA_DIRTY_VIEW_PROJ;
    }

    return m_viewProjection;
}

const glm::mat4& Camera::getInverseViewProjectionMatrix() const
{
    if( m_bits & CAMERA_DIRTY_INV_VIEW_PROJ )
    {
        m_inverseViewProjection = glm::inverse( getViewProjectionMatrix() );

        m_bits &= ~CAMERA_DIRTY_INV_VIEW_PROJ;
    }

    return m_inverseViewProjection;
}

void Camera::setViewMatrix(const glm::mat4& m)
{
    m_view = m;
    m_bits |= CAMERA_DIRTY_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW;
}
}
